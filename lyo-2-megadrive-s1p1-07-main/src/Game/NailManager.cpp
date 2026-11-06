#include "pch.h"
#include "NailManager.h"
#include "GameManager.h"

void NailManager::Create(GameManager* p_GameManager)
{
	m_pGameManager = p_GameManager;

	m_meshNail.Clear();
	m_meshNail.AddBox({ 0.f, 0.f, 0.f }, { 0.06f, 0.025f, 0.06f }, cpu::ToColor(130, 130, 130));
	m_meshNail.AddBox({ 0.f, -0.1f, 0.f }, { 0.018f, 0.1f, 0.018f }, cpu::ToColor(130, 130, 130));
	m_meshNail.Optimize();

	Hammer::BuildMesh(m_meshHammer);

	for (int side = 0; side < 2; ++side)
	{
		m_nailDisplay[side].Reset(2);
		for (int nail = 0; nail < 4; ++nail)
			m_nails[side][nail].Create(&m_meshNail);
	}
	m_hammer.Create(&m_meshHammer);
}

void NailManager::Destroy()
{
	for (int side = 0; side < 2; ++side)
		for (int nail = 0; nail < 4; ++nail)
			m_nails[side][nail].Destroy();
	m_hammer.Destroy();
}

bool NailManager::Configure(bool bossStage)
{
	bool needsArrival = false;

	for (int side = 0; side < 2; ++side)
	{
		const int count = side == AI_SIDE && bossStage ? 4 : 2;
		const int previousCount = m_nailDisplay[side].GetCount();
		const int hammeredBeforeShop = m_nailDisplay[side].GetHammered();
		m_nailDisplay[side].Reset(count);

		for (int nailIndex = 0; nailIndex < 4; ++nailIndex)
		{
			Nail& nail = m_nails[side][nailIndex];
			const bool exists = nailIndex < count;

			if (exists == false)
			{
				nail.SetVisible(false);
				continue;
			}

			const XMFLOAT3 position = { NAIL_SIDE_X, NAIL_RAISED_Y, GetTableSign(side) * m_nailDisplay[side].GetZ(nailIndex) };
			const bool replaced = nailIndex < hammeredBeforeShop || count != previousCount || nail.IsVisible() == false;

			if (replaced)
			{
				nail.BeginArrival(position, NAIL_DROP_Y);
				needsArrival = true;
			}
			else
			{
				nail.SetPosition(position);
				nail.SetVisible(true);
			}
		}
	}

	m_nailRaiseTimer = 0.f;
	m_hammer.SetVisible(false);
	return needsArrival;
}

bool NailManager::UpdateArrival(float dt)
{
	m_nailRaiseTimer += dt;
	bool allArrived = true;
	bool hasPendingArrival = false;
	for (int side = 0; side < 2; ++side)
	{
		for (int nailIndex = 0; nailIndex < 4; ++nailIndex)
		{
			Nail& nail = m_nails[side][nailIndex];

			if (nail.IsArrivalPending() == false)
				continue;

			hasPendingArrival = true;

			const int sideCount = m_nailDisplay[side].GetCount();
			const float delay = (sideCount - 1 - nailIndex) * NAIL_ARRIVAL_DELAY;

			if (nail.UpdateArrival(m_nailRaiseTimer, delay, NAIL_ARRIVAL_DURATION, NAIL_RAISED_Y) == false)
				allArrived = false;
		}
	}
	return hasPendingArrival && allArrived;
}

void NailManager::UpdateLayout()
{
	for (int side = 0; side < 2; ++side)
	{
		for (int nailIndex = 0; nailIndex < 4; ++nailIndex)
		{
			Nail& nail = m_nails[side][nailIndex];

			if (nail.IsVisible() == false)
				continue;

			const float zOffset = m_nailDisplay[side].GetZ(nailIndex);
			const float tableSide = GetTableSign(side);

			XMFLOAT3 position = nail.GetPosition();
			position.x = NAIL_SIDE_X;
			position.z = tableSide * zOffset;

			nail.SetPosition(position);
		}
	}
}

bool NailManager::QueueHit(int side)
{
	m_hammerSide = side;
	m_hammerNail = m_nailDisplay[m_hammerSide].GetHammered();
	m_nailPending = m_hammerNail < m_nailDisplay[m_hammerSide].GetCount();

	return m_nailPending;
}

void NailManager::StartHit()
{
	m_hammerTimer = 0.f;
	m_hammer.StartStrike(m_nails[m_hammerSide][m_hammerNail].GetPosition(), NAIL_TIP_OFFSET, Hammer::HEAD_OFFSET);
}

bool NailManager::UpdateHit(float dt)
{
	m_hammerTimer += dt;

	const float progress = cpu::Clamp(m_hammerTimer / NAIL_HAMMER_DURATION, 0.f, 1.f);
	const float nailY = cpu::Lerp(NAIL_RAISED_Y, NAIL_DRIVEN_Y, cpu::Clamp((progress - Hammer::IMPACT_PROGRESS) / (Hammer::DRIVEN_PROGRESS - Hammer::IMPACT_PROGRESS), 0.f, 1.f));

	Nail& nail = m_nails[m_hammerSide][m_hammerNail];
	nail.SetY(nailY);
	m_hammer.Update(progress, nail.GetPosition(), NAIL_TIP_OFFSET);

	m_pGameManager->ShakeCamera(0.3f);

	if (progress >= 1.f)
	{
		m_nailDisplay[m_hammerSide].HammerNext();
		nail.SetDriven(NAIL_DRIVEN_Y);
		m_hammer.SetVisible(false);
		m_nailPending = false;

		return true;
	}

	return false;
}

float NailManager::GetTableSign(int side)
{
	return side == PLAYER_SIDE ? -1.f : 1.f;
}

void NailManager::Reset()
{
	for (int side = 0; side < 2; ++side)
	{
		m_nailDisplay[side].Reset(2);
		for (int nail = 0; nail < 4; ++nail)
			m_nails[side][nail].Reset();
	}

	m_hammer.SetVisible(false);
	m_nailRaiseTimer = 0.f;
	m_hammerTimer = 0.f;
	m_hammerSide = 0;
	m_hammerNail = 0;
	m_nailPending = false;
}