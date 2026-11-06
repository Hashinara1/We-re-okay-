#include "pch.h"
#include "BonusEffects.h"
#include "ShopManager.h"
#include "AIStriker.h"
#include "Puck.h"
#include "PlayerStriker.h"
#include "Eyes.h"

void BonusEffects::SetAIStatusVisual(bool frozen, bool confused)
{
	const bool statusActive = frozen || confused;
	XMFLOAT3 statusColor = CPU_GREEN;

	if (frozen)
	{
		statusColor = BonusCatalog::Get(BonusId::Freeze).color;
	}
	else if (confused)
	{
		statusColor = BonusCatalog::Get(BonusId::Confusion).color;
	}

	m_owner.m_pAI->SetColor(statusColor);
	m_owner.m_pEyes->SetStatusColor(statusActive, statusColor);
}

void BonusEffects::UpdateGame(float dt, bool acceptInput)
{
	if (acceptInput)
		m_owner.m_presentation.UpdateItems(dt);

	m_owner.UpdateFusion(dt);
	m_owner.m_coins.UpdateGoldCoins(dt, acceptInput);

	if (acceptInput)
	{
		if (cpuInput.vi.IsKeyPressed('1'))
			ActivateBonus(0);

		if (cpuInput.vi.IsKeyPressed('2'))
			ActivateBonus(1);
	}

	for (float& timer : m_owner.m_bonusTimers)
	{
		if (timer > 0.f)
		{
			timer -= dt;

			if (timer < 0.f)
			{
				timer = 0.f;
			}
		}
	}

	m_owner.m_pAI->SetFrozen(m_owner.m_bonusTimers[(int)BonusId::Freeze] > 0.f);
	m_owner.m_pAI->SetConfused(m_owner.m_bonusTimers[(int)BonusId::Confusion] > 0.f);

	SetAIStatusVisual(m_owner.m_bonusTimers[(int)BonusId::Freeze] > 0.f, m_owner.m_bonusTimers[(int)BonusId::Confusion] > 0.f);

	m_owner.m_pPuck->SetStrikerVelocityMultiplier(m_owner.m_bonusTimers[(int)BonusId::Force] > 0.f ? 2.5f : 1.f);
	m_owner.UpdateSuddenDeathAura(dt);

	m_owner.m_effectEntities[0]->visible = m_owner.m_bonusTimers[(int)BonusId::Guardian] > 0.f;
	m_owner.m_effectEntities[1]->visible = m_owner.m_effectEntities[2]->visible = m_owner.m_ownedCages;

	if (m_owner.m_bonusTimers[(int)(BonusId::BigStriker)] <= 0.f)
	{
		m_owner.m_pPlayer->GetEntity()->transform.SetScaling(1.f);
	}
}

void BonusEffects::ActivateBonus(int slot)
{
	if (slot < 0 || slot >= ShopManager::ACTIVE_SLOT_COUNT)
		return;

	ShopItem& item = m_owner.m_activeBonuses[slot];

	if (item.id < 0)
		return;

	const int id = item.id;

	if (BonusCatalog::Get((BonusId)id).active == false)
		return;

	if (id == (int)BonusId::SuddenDeath)
	{
		m_owner.m_bonusTimers[id] = ShopManager::SUDDEN_DEATH_DURATION;
		m_owner.m_suddenDeathHit = false;
	}
	else if (id == (int)BonusId::Freeze || id == (int)BonusId::Confusion)
	{
		m_owner.m_bonusTimers[id] = 3.f;
	}
	else
	{
		m_owner.m_bonusTimers[id] = 8.f;
	}

	if (id == (int)BonusId::BigStriker)
	{
		m_owner.m_pPlayer->GetEntity()->transform.SetScaling({ 1.5f, 1.f, 1.5f });
	}

	item.entity = cpuEngine.Release(item.entity);
	item.id = -1;
}
