#include "pch.h"
#include "CoinSystem.h"
#include "ShopManager.h"
#include "PlayerStriker.h"
#include "Puck.h"
#include "DebrisManager.h"

void CoinSystem::SpawnGoldCoin()
{
	const float x = cpu::RandSigned(m_owner.m_seed) * ShopManager::COIN_SPAWN_HALF_WIDTH;
	const float z = (-cpu::Rand01(m_owner.m_seed) * ShopManager::COIN_SPAWN_HALF_LENGTH) - 1.5f;

	GoldCoin coin;

	coin.entity = cpuEngine.CreateEntity();
	coin.entity->pMesh = &m_owner.m_meshCoin;
	coin.entity->transform.SetPosition(x, ShopManager::DROP_Y, z);
	coin.entity->transform.SetYPR(0.f, 0.f, XM_PIDIV2);
	coin.entity->visible = true;

	coin.shadow = cpuEngine.CreateEntity();
	coin.shadow->pMesh = &m_owner.m_meshCoinShadow;
	coin.shadow->transform.SetPosition(x, 0.06f, z);
	coin.shadow->visible = true;
	m_owner.m_goldCoins.push_back(coin);
}

void CoinSystem::BurstGoldCoin(size_t index, bool playBurst)
{
	GoldCoin fallingCoin = m_owner.m_goldCoins[index];

	cpu_entity* coin = fallingCoin.entity;

	const XMFLOAT3 origin = coin->transform.pos;
	coin->visible = false;

	if (playBurst)
	{
		m_owner.m_debris->Spawn(origin, m_owner.m_seed, BonusCatalog::Get(BonusId::GoldShower).color, 8, 0.035f, 0.35f, 1.5f, 0.5f, 2.f);
	}
	else
	{
		SpawnCoinPickupParticles(origin);
	}

	cpuEngine.Release(coin);
	cpuEngine.Release(m_owner.m_goldCoins[index].shadow);
	m_owner.m_goldCoins.erase(m_owner.m_goldCoins.begin() + index);
}

void CoinSystem::SpawnCoinPickupParticles(const XMFLOAT3& position)
{
	CoinPickupEmitter particleEmitter;
	particleEmitter.emitter = cpuEngine.CreateParticleEmitter();
	particleEmitter.emitter->pos = position;
	particleEmitter.emitter->dir = { 0.f, 1.f, 0.f };
	particleEmitter.emitter->colorMin = cpu::ToColor(238, 190, 55);
	particleEmitter.emitter->colorMax = cpu::ToColor(255, 235, 125);
	particleEmitter.emitter->durationMin = 0.09f;
	particleEmitter.emitter->durationMax = 0.15f;
	particleEmitter.emitter->spread = 1.25f;
	particleEmitter.lifetime = 0.2f;
	m_owner.m_coinPickupEmitters.push_back(particleEmitter);
}

void CoinSystem::UpdateGoldCoins(float dt, bool allowCoinSpawn)
{
	const XMFLOAT2 puckVelocity = m_owner.m_pPuck->GetVelocity();
	const bool puckIsMoving = puckVelocity.x * puckVelocity.x + puckVelocity.y * puckVelocity.y > 0.25f;

	if (allowCoinSpawn && puckIsMoving && m_owner.m_inventory.HasPassive(BonusId::GoldShower))
	{
		m_owner.m_goldSpawnTimer -= dt;

		if (m_owner.m_goldSpawnTimer <= 0.f)
		{
			SpawnGoldCoin();
			m_owner.m_goldSpawnTimer = ShopManager::COIN_SPAWN_MIN + cpu::Rand01(m_owner.m_seed) * (ShopManager::COIN_SPAWN_MAX - ShopManager::COIN_SPAWN_MIN);
		}
	}

	cpu_entity* striker = m_owner.m_pPlayer->GetEntity();

	for (size_t i = 0; i < m_owner.m_goldCoins.size(); )
	{
		GoldCoin& fallingCoin = m_owner.m_goldCoins[i];
		cpu_entity* coin = fallingCoin.entity;

		if (puckIsMoving)
		{
			const float nextHeight = coin->transform.pos.y - ShopManager::COIN_FALL_SPEED * dt;
			coin->transform.pos.y = nextHeight > ShopManager::COIN_HALF_HEIGHT ? nextHeight : ShopManager::COIN_HALF_HEIGHT;
			fallingCoin.rotation += 7.f * dt;
			coin->transform.SetYPR(fallingCoin.rotation, 0.f, XM_PIDIV2);
		}

		fallingCoin.shadow->transform.SetPosition(coin->transform.pos.x, 0.06f, coin->transform.pos.z);

		const float shadowScale = cpu::Clamp(coin->transform.pos.y / ShopManager::DROP_Y, 0.12f, 1.f);
		fallingCoin.shadow->transform.SetScaling({ shadowScale, 1.f, shadowScale });

		const bool hasReachedTable = puckIsMoving && coin->transform.pos.y <= ShopManager::COIN_HALF_HEIGHT + CPU_EPSILON;

		XMFLOAT3 coinCenter = coin->sphere.center;
		XMFLOAT3 strikerCenter = striker->sphere.center;

		const float overlapX = coin->obb.half.x + striker->obb.half.x;
		const float overlapZ = coin->obb.half.z + striker->obb.half.z;
		const float dx = coinCenter.x - strikerCenter.x;
		const float dz = coinCenter.z - strikerCenter.z;

		const bool overlapsOnTable = fabsf(dx) <= overlapX && fabsf(dz) <= overlapZ;

		if (puckIsMoving && overlapsOnTable && cpu::SphereSphere(coinCenter, coin->sphere.radius, strikerCenter, striker->sphere.radius))
		{
			if(cpu::ObbObb(coin->obb, striker->obb))
			{
				BurstGoldCoin(i, false);
				m_owner.AddMoney(1);
				continue;
			}
		}

		if (hasReachedTable)
		{
			BurstGoldCoin(i, true);
			continue;
		}

		++i;
	}
}

void CoinSystem::UpdateCoinPickupEmitters(float dt)
{
	for (size_t i = 0; i < m_owner.m_coinPickupEmitters.size(); )
	{
		CoinPickupEmitter& particleEmitter = m_owner.m_coinPickupEmitters[i];
		particleEmitter.lifetime -= dt;

		if (particleEmitter.lifetime <= 0.f)
		{
			particleEmitter.emitter->rate = 0.f;
			particleEmitter.emitter = cpuEngine.Release(particleEmitter.emitter);
			m_owner.m_coinPickupEmitters.erase(m_owner.m_coinPickupEmitters.begin() + i);
			continue;
		}

		++i;
	}
}
