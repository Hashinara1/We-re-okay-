#include "pch.h"
#include "ShopManager.h"
#include "PlayerStriker.h"
#include "AIStriker.h"
#include "Puck.h"
#include "Eyes.h"
#include "GameManager.h"

ShopManager::ShopManager() : m_shop(*this) , m_inventory(*this), m_interaction(*this), m_presentation(*this), m_effects(*this), m_coins(*this), m_laser(*this), m_bonusFusion(*this) {}

void ShopManager::Create(GameManager* gameManager)
{
	m_pGameManager = gameManager;

	m_pPlayer = gameManager->GetPlayerStriker();
	m_pAI = gameManager->GetAiStriker();
	m_pPuck = gameManager->GetPuck();
	m_pEyes = gameManager->GetEyes();
	m_aiBaseDifficulty = m_pAI->GetBaseDifficulty();
	m_seed = timeGetTime();
	m_tooltipFont.Create(cpuDevice.GetHeight() <= 512 ? 10 : 15, CPU_WHITE, "Dracutaz");

	m_meshSlot.CreateBox(SLOT_HALF_WIDTH, 0.015f, SLOT_HALF_WIDTH, cpu::ToColor(38, 18, 5));

	for (int id = 0; id < BONUS_COUNT; ++id)
		m_bonusMeshes[id].CreateBox(ITEM_HALF_SIZE, ITEM_HALF_HEIGHT, ITEM_HALF_SIZE, BonusCatalog::Get((BonusId)id).color);

	m_meshGuardianWall.CreateBox(2.f, 0.11f, 0.1f, cpu::ToColor(65, 170, 230));

	m_meshCageWall.CreateBox(0.5f, 0.12f, 0.24f, cpu::ToColor(110, 190, 85));

	m_meshCoin.CreateCylinder(COIN_HALF_HEIGHT, COIN_RADIUS, 12, true, true, BonusCatalog::Get(BonusId::GoldShower).color);
	m_meshCoinShadow.CreateCylinder(0.004f, COIN_RADIUS, 16, true, true, cpu::ToColor(120, 100, 15));

	m_aiStrikerMeshes[0].CreateBox(0.25f, 0.10f, 0.10f, CPU_GREEN);
	m_aiStrikerMeshes[1].CreateBox(0.25f, 0.10f, 0.10f, BonusCatalog::Get(BonusId::Freeze).color);
	m_aiStrikerMeshes[2].CreateBox(0.25f, 0.10f, 0.10f, BonusCatalog::Get(BonusId::Confusion).color);

	for (int slot = 0; slot < ACTIVE_SLOT_COUNT; ++slot)
	{
		m_activeSlotEntities[slot] = cpuEngine.CreateEntity();
		m_activeSlotEntities[slot]->pMesh = &m_meshSlot;
		m_activeSlotEntities[slot]->transform.SetPosition(INVENTORY_X, SLOT_Y, ACTIVE_SLOT_Z[slot]);
		m_activeSlotEntities[slot]->visible = true;
	}

	for (int slot = 0; slot < PASSIVE_SLOT_COUNT; ++slot)
	{
		m_passiveSlotEntities[slot] = cpuEngine.CreateEntity();
		m_passiveSlotEntities[slot]->pMesh = &m_meshSlot;
		m_passiveSlotEntities[slot]->transform.SetPosition(INVENTORY_X, SLOT_Y, PASSIVE_SLOT_Z[slot]);
		m_passiveSlotEntities[slot]->visible = true;
	}

	m_effectEntities[0] = cpuEngine.CreateEntity();
	m_effectEntities[0]->pMesh = &m_meshGuardianWall;
	m_effectEntities[0]->transform.SetPosition(0.f, 0.12f, -5.5f);

	m_effectEntities[1] = cpuEngine.CreateEntity();
	m_effectEntities[1]->pMesh = &m_meshCageWall;
	m_effectEntities[1]->transform.SetPosition(-1.5f, 0.12f, -5.5f);

	m_effectEntities[2] = cpuEngine.CreateEntity();
	m_effectEntities[2]->pMesh = &m_meshCageWall;
	m_effectEntities[2]->transform.SetPosition(1.5f, 0.12f, -5.5f);

	for (cpu_entity* effect : m_effectEntities)
	{
		effect->visible = false;
	}

	m_laser.Create();

	m_suddenDeathAura = cpuEngine.CreateParticleEmitter();
	m_suddenDeathAura->active = false;
	m_suddenDeathAura->colorMin = cpu::ToColor(255, 0, 0);
	m_suddenDeathAura->colorMax = cpu::ToColor(255, 0, 0);
	m_suddenDeathAura->durationMin = 0.1f;
	m_suddenDeathAura->durationMax = 0.15f;

	m_suddenDeathAura2 = cpuEngine.CreateParticleEmitter();
	m_suddenDeathAura2->active = false;
	m_suddenDeathAura2->colorMin = cpu::ToColor(255, 0, 0);
	m_suddenDeathAura2->colorMax = cpu::ToColor(255, 0, 0);
	m_suddenDeathAura2->durationMin = 0.1f;
	m_suddenDeathAura2->durationMax = 0.15f;

	m_shop.GenerateOffers();
}

void ShopManager::Destroy()
{
	for (ShopItem& item : m_offers)
	{
		m_presentation.DestroyItem(item);
	}

	for (ShopItem& item : m_activeBonuses)
	{
		m_presentation.DestroyItem(item);
	}

	for (ShopItem& item : m_passiveBonuses)
	{
		m_presentation.DestroyItem(item);
	}

	for (ShopItem& item : m_discardedBonuses)
	{
		m_presentation.DestroyItem(item);
	}

	for (GoldCoin& coin : m_goldCoins)
	{
		coin.entity = cpuEngine.Release(coin.entity);
		coin.shadow = cpuEngine.Release(coin.shadow);
	}

	if (m_suddenDeathAura)
	{
		m_suddenDeathAura = cpuEngine.Release(m_suddenDeathAura);
	}
	if (m_suddenDeathAura2)
	{
		m_suddenDeathAura2 = cpuEngine.Release(m_suddenDeathAura2);
	}

	m_bonusFusion.Destroy();

	for (CoinPickupEmitter& emitter : m_coinPickupEmitters)
	{
		if (emitter.emitter)
		{
			emitter.emitter = cpuEngine.Release(emitter.emitter);
		}
	}

	m_coinPickupEmitters.clear();
	m_laser.Destroy();

	for (cpu_entity*& entity : m_activeSlotEntities)
	{
		entity = cpuEngine.Release(entity);
	}

	for (cpu_entity*& entity : m_passiveSlotEntities)
	{
		entity = cpuEngine.Release(entity);
	}

	for (cpu_entity*& entity : m_effectEntities)
	{
		entity = cpuEngine.Release(entity);
	}
}

void ShopManager::BeginShop()
{
	m_shop.BeginShop();
}

void ShopManager::UpdateShop(float dt)
{
	m_shop.UpdateShop(dt);
	UpdateFusion(dt);
	UpdateSuddenDeathAura(dt);
}

void ShopManager::UpdateItems(float dt)
{
	m_presentation.UpdateItems(dt);
}

void ShopManager::UpdateDiscardedItems(float dt)
{
	m_presentation.UpdateDiscardedItems(dt);
}

void ShopManager::StartOfferDrop()
{
	m_shop.StartOfferDrop();
}

void ShopManager::StartOfferRise()
{
	m_shop.StartOfferRise();
}

bool ShopManager::AreOffersLanded() const
{
	return m_shop.AreOffersLanded();
}

bool ShopManager::AreOffersGone() const
{
	return m_shop.AreOffersGone();
}

bool ShopManager::IsInventorySettled() const
{
	return m_inventory.IsSettled();
}

void ShopManager::SetShopItemsVisible(bool visible)
{
	m_presentation.SetShopItemsVisible(visible);
}

void ShopManager::SetGameItemsVisible(bool visible)
{
	m_presentation.SetGameItemsVisible(visible);
}

void ShopManager::ClearRoundCoins()
{
	for (GoldCoin& coin : m_goldCoins)
	{
		coin.entity = cpuEngine.Release(coin.entity);
		coin.shadow = cpuEngine.Release(coin.shadow);
	}

	m_goldCoins.clear();

	for (CoinPickupEmitter& emitter : m_coinPickupEmitters)
	{
		if (emitter.emitter)
		{
			emitter.emitter->rate = 0.f;
			emitter.emitter->active = false;
			emitter.emitter = cpuEngine.Release(emitter.emitter);
		}
	}

	m_coinPickupEmitters.clear();
	m_goldSpawnTimer = 0.f;
}

void ShopManager::ClearInventory()
{
	m_bonusFusion.Cancel();

	for (ShopItem& item : m_activeBonuses)
		m_presentation.DestroyItem(item);

	for (ShopItem& item : m_passiveBonuses)
		m_presentation.DestroyItem(item);

	for (ShopItem& item : m_discardedBonuses)
		m_presentation.DestroyItem(item);

	m_discardedBonuses.clear();
	m_ownedExtraLife = false;
	m_ownedCages = false;
	m_goldSpawnTimer = 0.f;

	for (float& timer : m_bonusTimers)
		timer = 0.f;

	m_effects.SetAIStatusVisual(false, false);
	m_pPlayer->GetEntity()->transform.SetScaling(1.f);
	m_pAI->SetFrozen(false);
	m_pAI->SetConfused(false);
	m_pAI->SetDifficulty(m_aiBaseDifficulty);
	m_pPuck->SetStrikerVelocityMultiplier(1.f);
	ClearRoundCoins();
}

void ShopManager::UpdateGame(float dt, bool acceptInput)
{
	m_effects.UpdateGame(dt, acceptInput);
}

void ShopManager::UpdateLaser(float dt, bool allowCoinSpawn)
{
	m_laser.Update(dt, allowCoinSpawn);
}

void ShopManager::UpdateFusion(float dt)
{
	m_bonusFusion.Update(dt);
}

void ShopManager::UpdateSuddenDeathAura(float dt)
{
	m_suddenDeathAuraPhase += dt * SUDDEN_DEATH_AURA_ORBIT_SPEED;

	const bool effectActive = m_bonusTimers[(int)BonusId::SuddenDeath] > 0.f;
	const XMFLOAT3* center = nullptr;

	if (effectActive && m_pPuck)
	{
		center = &m_pPuck->GetEntity()->transform.pos;
	}
	else
	{
		for (const ShopItem& item : m_activeBonuses)
		{
			if (item.id == (int)BonusId::SuddenDeath && item.entity != nullptr)
			{
				center = &item.entity->transform.pos;
				break;
			}
		}
	}

	if (center)
	{
		m_suddenDeathAura->active = true;
		m_suddenDeathAura->rate = 1.f;

		m_suddenDeathAura2->active = true;
		m_suddenDeathAura2->rate = 1.f;

		if (effectActive)
		{
			m_suddenDeathAura2->active = false;
			m_suddenDeathAura->pos = { center->x, center->y, center->z };
		}
		else
		{
			const float angle = m_suddenDeathAuraPhase - XM_PIDIV2;

			m_suddenDeathAura->pos =
			{
				center->x + cosf(angle) * SUDDEN_DEATH_AURA_ORBIT_RADIUS,
				center->y + sinf(m_suddenDeathAuraPhase) * SUDDEN_DEATH_AURA_ORBIT_Y_AMPLITUDE,
				center->z + sinf(angle) * SUDDEN_DEATH_AURA_ORBIT_RADIUS
			};

			m_suddenDeathAura2->pos =
			{
				center->x + cosf(angle) * SUDDEN_DEATH_AURA_ORBIT_RADIUS,
				center->y + sinf(m_suddenDeathAuraPhase) * -SUDDEN_DEATH_AURA_ORBIT_Y_AMPLITUDE,
				center->z + sinf(angle) * SUDDEN_DEATH_AURA_ORBIT_RADIUS
			};
		}
	}
	else
	{
		m_suddenDeathAura->active = false;
		m_suddenDeathAura2->active = false;
	}

	if (effectActive == false)
		m_pPuck->SetSuddenDeathActive(false);
}

void ShopManager::HideLaser()
{
	m_laser.Hide();
}

bool ShopManager::ConsumeSuddenDeathHit()
{
	const bool hit = m_pPuck->ConsumeSuddenDeathHit();

	if (hit)
	{
		m_bonusTimers[(int)BonusId::SuddenDeath] = 0.f;
		m_suddenDeathAura->active = false;
		m_suddenDeathAura2->active = false;
	}

	return hit;
}

bool ShopManager::IsGuardianActive() const
{
	return m_bonusTimers[(int)BonusId::Guardian] > 0.f;
}

bool ShopManager::AreCagesActive() const
{
	return m_ownedCages;
}

bool ShopManager::IsBigStrikerActive() const
{
	return m_bonusTimers[(int)BonusId::BigStriker] > 0.f;
}

bool ShopManager::IsSuddenDeathActive() const
{
	return m_bonusTimers[(int)BonusId::SuddenDeath] > 0.f;
}

bool ShopManager::IsPuckSlowdownActive() const
{
	return m_inventory.HasPassive(BonusId::PuckSlowdown);
}

bool ShopManager::IsExtraLifeOwned() const
{
	return m_inventory.HasPassive(BonusId::ExtraLife);
}

void ShopManager::ConsumeExtraLife()
{
	if(m_inventory.HasPassive(BonusId::ExtraLife))
		m_inventory.DestroyInventoryItem(BonusId::ExtraLife);
}

float ShopManager::GetTableFriction() const
{
	return m_inventory.GetTableFriction();
}

void ShopManager::SetAIStatusVisual(bool frozen, bool confused)
{
	m_effects.SetAIStatusVisual(frozen, confused);
}

void ShopManager::Render(bool showShopTooltip)
{
	m_presentation.Render(showShopTooltip);
}

GameCamera& ShopManager::GetGameCamera()
{
	return m_pGameManager->GetGameCamera();
}

void ShopManager::AddMoney(int p_value)
{
	m_pGameManager->AddMoney(p_value); 
}
