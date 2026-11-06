#include "pch.h"
#include "Shop.h"
#include "ShopManager.h"
#include "GameManager.h"
#include "PlayerStriker.h"
#include "AIStriker.h"
#include "Puck.h"

void Shop::GenerateOffers()
{
	for (ShopItem& item : m_owner.m_offers)
	{
		m_owner.m_presentation.DestroyItem(item);
	}

	int pool[ShopManager::BONUS_COUNT];
	int count = 0;

	for (int id = 0; id < ShopManager::BONUS_COUNT; ++id)
	{
		if (id != (int)BonusId::SuddenDeath &&
			(id != (int)BonusId::ExtraLife || m_owner.m_ownedExtraLife == false) &&
			(id != (int)BonusId::Cages || m_owner.m_ownedCages == false) &&
			(id != (int)BonusId::GoldShower || m_owner.m_inventory.HasPassive(BonusId::GoldShower) == false) &&
			(id != (int)BonusId::Laser || m_owner.m_inventory.HasPassive(BonusId::Laser) == false) &&
			(id != (int)BonusId::PuckSlowdown || m_owner.m_inventory.HasPassive(BonusId::PuckSlowdown) == false))
		{
			pool[count++] = id;
		}
	}

	for (int i = 0; i < ShopManager::OFFER_COUNT && count > 0; ++i)
	{
		const int randomIndex = (int)(cpu::Rand01(m_owner.m_seed) * count);
		const int index = cpu::Clamp(randomIndex, 0, count - 1);
		const int id = pool[index];

		pool[index] = pool[--count];

		ShopItem& offer = m_owner.m_offers[i];
		offer.id = id;
		offer.target = { ShopManager::OFFER_X[i], ShopManager::ITEM_Y, ShopManager::OFFER_Z };
		offer.entity = cpuEngine.CreateEntity();
		offer.entity->pMesh = &m_owner.m_bonusMeshes[id];
		offer.entity->transform.SetPosition(offer.target.x, ShopManager::DROP_Y, offer.target.z);
		offer.entity->visible = false;
	}

	m_owner.m_shopItemsReady = false;
}

void Shop::BeginShop()
{
	GenerateOffers();

	for (ShopItem& item : m_owner.m_offers)
	{
		if (item.entity)
		{
			item.entity->transform.pos.y = ShopManager::DROP_Y;
			item.entity->visible = false;
		}
	}

	for (float& timer : m_owner.m_bonusTimers)
	{
		timer = 0.f;
	}

	m_owner.m_goldSpawnTimer = 0.f;

	for (GoldCoin& coin : m_owner.m_goldCoins)
	{
		coin.entity = cpuEngine.Release(coin.entity);
		coin.shadow = cpuEngine.Release(coin.shadow);
	}

	m_owner.m_goldCoins.clear();

	for (CoinPickupEmitter& emitter : m_owner.m_coinPickupEmitters)
	{
		if (emitter.emitter != nullptr)
		{
			emitter.emitter->active = false;
			emitter.emitter = cpuEngine.Release(emitter.emitter);
		}
	}

	m_owner.m_coinPickupEmitters.clear();

	m_owner.m_laser.Hide();
	if (m_owner.m_suddenDeathAura != nullptr)
	{
		m_owner.m_suddenDeathAura->rate = 0.f;
		m_owner.m_suddenDeathAura->active = false;
	}

	m_owner.m_pPlayer->GetEntity()->transform.SetScaling(1.f);

	m_owner.m_pAI->SetFrozen(false);
	m_owner.m_pAI->SetConfused(false);	
	m_owner.m_pAI->SetDifficulty(m_owner.m_aiBaseDifficulty);

	m_owner.m_effects.SetAIStatusVisual(false, false);

	m_owner.m_pPuck->SetStrikerVelocityMultiplier(1.f);

	for (cpu_entity* effect : m_owner.m_effectEntities)
	{
		effect->visible = false;
	}

	m_owner.m_presentation.SetShopItemsVisible(true);
}

void Shop::StartOfferDrop()
{
	for (int i = 0; i < ShopManager::OFFER_COUNT; ++i)
	{
		ShopItem& item = m_owner.m_offers[i];

		if (item.entity == nullptr)
			continue;

		item.entity->visible = true;
		item.entity->transform.SetPosition(item.target.x, ShopManager::DROP_Y, item.target.z);
		item.falling = true;
		item.rising = false;
		item.delay = i * 0.14f;
	}
}

void Shop::StartOfferRise()
{
	m_owner.m_shopItemsReady = false;

	for (int i = 0; i < ShopManager::OFFER_COUNT; ++i)
	{
		ShopItem& item = m_owner.m_offers[i];
		item.rising = item.entity != nullptr;
		item.falling = false;
		item.delay = i * 0.14f;
	}
}

bool Shop::AreOffersLanded() const
{
	for (const ShopItem& item : m_owner.m_offers)
	{
		if (item.falling)
			return false;
	}

	return true;
}

bool Shop::AreOffersGone() const
{
	for (const ShopItem& item : m_owner.m_offers)
	{
		if (item.rising)
			return false;
	}

	return true;
}

void Shop::UpdateShop(float dt)
{
	m_owner.m_presentation.UpdateItems(dt);

	if (AreOffersLanded())
	{
		m_owner.m_shopItemsReady = true;
	}

	m_owner.m_interaction.UpdateShop();
}

void Shop::PurchaseOffer(int index)
{
	if (index < 0 || index >= ShopManager::OFFER_COUNT)
		return;

	ShopItem& offer = m_owner.m_offers[index];

	if (offer.id < 0 || offer.entity == nullptr)
		return;

	const int id = offer.id;
	const bool active = BonusCatalog::Get((BonusId)id).active;
	int slot = -1;

	if (active)
	{
		slot = m_owner.m_inventory.FindFreeActiveSlot();
	}
	else
	{
		for (int i = 0; i < ShopManager::PASSIVE_SLOT_COUNT; ++i)
		{
			if (m_owner.m_passiveBonuses[i].id < 0)
			{
				slot = i;
				break;
			}
		}
	}

	const int price = BonusCatalog::Get((BonusId)id).price;

	if (slot < 0 || m_owner.m_pGameManager->TrySpendMoney(price) == false)
	{
		offer.shakeTimer = ShopManager::SHAKE_DURATION;
		return;
	}

	ShopItem& item = active ? m_owner.m_activeBonuses[slot] : m_owner.m_passiveBonuses[slot];
	item.id = id;
	item.entity = offer.entity;
	item.target = { ShopManager::INVENTORY_X, ShopManager::ITEM_Y, active ? ShopManager::ACTIVE_SLOT_Z[slot] : ShopManager::PASSIVE_SLOT_Z[slot] };
	item.moving = true;

	if (id == (int)BonusId::ExtraLife)
	{
		m_owner.m_ownedExtraLife = true;
	}

	if (id == (int)BonusId::Cages)
	{
		m_owner.m_ownedCages = true;
	}

	if (id == (int)BonusId::GoldShower)
	{
		m_owner.m_goldSpawnTimer = ShopManager::COIN_SPAWN_MIN + cpu::Rand01(m_owner.m_seed) * (ShopManager::COIN_SPAWN_MAX - ShopManager::COIN_SPAWN_MIN);
	}

	offer.id = -1;
	offer.entity = nullptr;
}
