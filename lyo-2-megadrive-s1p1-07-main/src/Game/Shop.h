#pragma once

#include "pch.h"

class ShopManager;

class Shop
{
public:
	Shop(ShopManager& owner) : m_owner(owner) {}

	void GenerateOffers();
	void BeginShop();
	void StartOfferDrop();
	void StartOfferRise();
	bool AreOffersLanded() const;
	bool AreOffersGone() const;
	void UpdateShop(float dt);
	void PurchaseOffer(int index);

private:
	ShopManager& m_owner;
};
