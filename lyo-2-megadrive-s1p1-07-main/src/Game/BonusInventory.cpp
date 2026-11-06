#include "pch.h"
#include "BonusInventory.h"
#include "ShopManager.h"
#include "Puck.h"
#include "DebrisManager.h"

bool BonusInventory::IsSettled() const
{
	if (m_owner.m_bonusFusion.IsActive())
		return false;

	for (const ShopItem& item : m_owner.m_activeBonuses)
	{
		if (item.moving)
		{
			return false;
		}
	}

	for (const ShopItem& item : m_owner.m_passiveBonuses)
	{
		if (item.moving)
		{
			return false;
		}
	}

	return true;
}

int BonusInventory::FindFreeActiveSlot() const
{
	for (int i = 0; i < ShopManager::ACTIVE_SLOT_COUNT; ++i)
	{
		if (m_owner.m_activeBonuses[i].id < 0)
			return i;
	}

	return -1;
}

void BonusInventory::DiscardInventoryItem(int slot, bool passive, const XMFLOAT2& releasePosition)
{
	ShopItem& item = passive ? m_owner.m_passiveBonuses[slot] : m_owner.m_activeBonuses[slot];

	if (item.id < 0 || item.entity == nullptr)
	{
		return;
	}

	if (item.id == (int)BonusId::ExtraLife)
	{
		m_owner.m_ownedExtraLife = false;
	}

	if (item.id == (int)BonusId::Cages)
	{
		m_owner.m_ownedCages = false;
	}

	if (item.id == (int)BonusId::GoldShower)
	{
		m_owner.m_goldSpawnTimer = 0.f;
	}

	item.entity->transform.SetPosition(releasePosition.x, ShopManager::ITEM_Y, releasePosition.y);
	item.entity->visible = true;
	item.moving = false;

	m_owner.m_discardedBonuses.push_back(item);

	item = ShopItem{};
}

void BonusInventory::DestroyInventoryItem(BonusId id)
{
	BonusInfo info = BonusCatalog::Get(id);
		 
	if (info.active)
	{
		for (ShopItem& item : m_owner.m_activeBonuses)
		{
			if (item.id == (int)id)
			{
				m_owner.m_debris->Spawn(item.entity->transform.pos, m_owner.m_seed, info.color, 36);
				m_owner.m_presentation.DestroyItem(item);
			}
		}
	}
	else
	{
		for (ShopItem& item : m_owner.m_passiveBonuses)
		{
			if (item.id == (int)id)
			{
				m_owner.m_debris->Spawn(item.entity->transform.pos, m_owner.m_seed, info.color, 36);
				m_owner.m_presentation.DestroyItem(item);
			}
		}
	}
}

bool BonusInventory::HasPassive(BonusId id) const
{
	for (const ShopItem& item : m_owner.m_passiveBonuses)
	{
		if (item.id == (int)id)
		{
			return true;
		}
	}

	return false;
}

float BonusInventory::GetTableFriction() const
{
	const XMFLOAT2 position = m_owner.m_pPuck->GetPosition();
	const XMFLOAT2 velocity = m_owner.m_pPuck->GetVelocity();

	const bool puckInPlayerHalf = position.y < 0.f;
	const bool puckMovingTowardPlayer = velocity.y < 0.f;

	if (m_owner.IsPuckSlowdownActive() && puckInPlayerHalf && puckMovingTowardPlayer)
	{
		return ShopManager::PASSIVE_FRICTION;
	}

	return ShopManager::BASE_FRICTION;
}
