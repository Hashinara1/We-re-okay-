#pragma once

#include "pch.h"

class ShopManager;

class CoinSystem
{
public:
	CoinSystem(ShopManager& owner) : m_owner(owner) {}

	void SpawnGoldCoin();
	void BurstGoldCoin(size_t index, bool playBurst);
	void SpawnCoinPickupParticles(const XMFLOAT3& position);
	void UpdateGoldCoins(float dt, bool allowCoinSpawn);
	void UpdateCoinPickupEmitters(float dt);

private:
	ShopManager& m_owner;
};
