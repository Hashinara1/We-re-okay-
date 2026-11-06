#pragma once

#include "pch.h"
#include "BonusCatalog.h"

class ShopManager;

class BonusInventory
{
public:
	BonusInventory(ShopManager& owner) : m_owner(owner) {}

	bool IsSettled() const;
	int FindFreeActiveSlot() const;
	void DiscardInventoryItem(int slot, bool passive, const XMFLOAT2& releasePosition);
	void DestroyInventoryItem(BonusId id);
	bool HasPassive(BonusId id) const;
	float GetTableFriction() const;

private:
	ShopManager& m_owner;
};
