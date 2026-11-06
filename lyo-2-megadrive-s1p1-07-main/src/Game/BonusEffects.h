#pragma once

#include "pch.h"

class ShopManager;

class BonusEffects
{
public:
	BonusEffects(ShopManager& owner) : m_owner(owner) {}

	void SetAIStatusVisual(bool frozen, bool confused);
	void UpdateGame(float dt, bool acceptInput);
	void ActivateBonus(int slot);

private:
	ShopManager& m_owner;
};
