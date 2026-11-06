#include "pch.h"
#include "BonusFusion.h"
#include "ShopManager.h"

BonusFusion::BonusFusion(ShopManager& owner) : m_owner(owner)
{
	m_explosionEmitter = cpuEngine.CreateParticleEmitter();
	m_explosionEmitter->active = false;
}

void BonusFusion::Begin(ShopItem& dragged, ShopItem& target, int resultSlot, BonusId resultId)
{
	m_entities[0] = dragged.entity;
	m_entities[1] = target.entity;
	m_starts[0] = dragged.entity->transform.pos;
	m_starts[1] = target.entity->transform.pos;
	m_elapsed = 0.f;
	m_resultSlot = resultSlot;
	m_resultId = resultId;
	m_returningToSlot = false;
	m_active = true;

	m_explosionEmitter->active = false;
	m_explosionEmitter->colorMin = cpu::ToColor(255, 0, 0);
	m_explosionEmitter->colorMax = cpu::ToColor(255, 0, 0);
	m_explosionEmitter->durationMax = 1.8f;
	m_explosionEmitter->spread = 3.f;
	m_explosionEmitter->rate = 1.f;
	m_explosionEmitter->pos = { 0.f, 1.5f, m_starts[0].z + 0.5f};
}

void BonusFusion::Release()
{
	for (cpu_entity*& entity : m_entities)
		entity = cpuEngine.Release(entity);
}

void BonusFusion::Reset()
{
	m_elapsed = 0.f;
	m_resultSlot = -1;
	m_resultId = BonusId::Count;
	m_returningToSlot = false;
	m_active = false;
}

void BonusFusion::Cancel()
{
	Release();
	Reset();
}

void BonusFusion::Update(float dt)
{
	if(m_explosionTimer > 0.f)
	{
		m_explosionTimer -= dt;
		if (m_explosionTimer <= 0.4f)
		{
			m_explosionEmitter->active = false;
		}

		return;
	}

	if (m_active == false)
		return;

	m_elapsed += dt;

	if (m_returningToSlot == false)
	{
		const float progress = cpu::Clamp(m_elapsed / ShopManager::FUSION_DURATION, 0.f, 1.f);
		const float radiusProgress = progress * progress * (3.f - 2.f * progress);
		const float orbitAngle = progress * XM_2PI * 2.f;
		const float radius = cpu::Lerp(0.55f, 0.04f, radiusProgress);
		const float x = cosf(orbitAngle) * radius;
		const float y = sinf(orbitAngle) * radius;

		if (m_entities[0] != nullptr)
		{
			m_entities[0]->transform.SetPosition(x, 1.5f + y, m_starts[0].z);
			m_entities[0]->transform.SetScaling(1.f);
		}

		if (m_entities[1] != nullptr)
		{
			m_entities[1]->transform.SetPosition(-x, 1.5f - y, m_starts[1].z);
			m_entities[1]->transform.SetScaling(1.f);
		}

		if (progress >= 1.f)
		{
			Release();

			int slot = -1;

			for (int i = 0; i < ShopManager::ACTIVE_SLOT_COUNT; ++i)
			{
				if (m_owner.m_activeBonuses[i].id < 0)
				{
					slot = i;
					break;
				}
			}

			if (slot < 0)
				slot = m_resultSlot;

			if (slot < 0 || slot >= ShopManager::ACTIVE_SLOT_COUNT)
				slot = 0;

			m_resultSlot = slot;

			m_explosionEmitter->active = true;
			m_explosionEmitter->rate = 1.f;
			m_explosionTimer = 0.5f;

			ShopItem& result = m_owner.m_activeBonuses[slot];
			result.id = (int)m_resultId;
			result.entity = cpuEngine.CreateEntity();
			result.entity->pMesh = &m_owner.m_bonusMeshes[result.id];
			result.entity->transform.SetPosition(0.f, 1.5f, m_starts[0].z);
			result.entity->transform.SetScaling(1.f);
			result.entity->visible = true;
			result.moving = false;

			m_returningToSlot = true;

			m_elapsed = 0.f;
		}
		return;
	}

	const float progress = cpu::Clamp(m_elapsed / ShopManager::FUSION_RETURN_DURATION, 0.f, 1.f);

	ShopItem& result = m_owner.m_activeBonuses[m_resultSlot];

	if (result.entity != nullptr)
	{
		result.entity->transform.SetPosition(cpu::Lerp(0.f, ShopManager::INVENTORY_X, progress), cpu::Lerp(1.5f, ShopManager::ITEM_Y, progress), cpu::Lerp(m_starts[0].z, ShopManager::ACTIVE_SLOT_Z[m_resultSlot], progress));
	}

	if (progress >= 1.f)
		Reset();
}

void BonusFusion::Destroy()
{
	Release();

	if (m_explosionEmitter)
	{
		m_explosionEmitter = cpuEngine.Release(m_explosionEmitter);
	}
}