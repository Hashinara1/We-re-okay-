#pragma once

#include "pch.h"

class ShopManager;
struct ShopItem;

class ShopItemPresentation
{
public:
	ShopItemPresentation(ShopManager& owner) : m_owner(owner) {}

	void SetShopItemsVisible(bool visible);
	void SetGameItemsVisible(bool visible);
	void UpdateItems(float dt);
	void UpdateDiscardedItems(float dt);
	void DestroyItem(ShopItem& item);
	void Render(bool showShopTooltip);

private:
	ShopManager& m_owner;
};
