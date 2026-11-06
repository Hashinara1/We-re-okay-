#pragma once

#include "pch.h"

class ShopManager;

class ShopInteraction
{
public:
	ShopInteraction(ShopManager& owner) : m_owner(owner) {}

	void UpdateShop();
	bool GetMouseTablePosition(XMFLOAT2& position) const;
	int FindHoveredItem(bool& isOffer, int& slot) const;
	bool IsOutsideTable(const XMFLOAT2 & position) const;

private:
	ShopManager& m_owner;
};
