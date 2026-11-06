#include "pch.h"
#include "BossEncounter.h"
#include "Puck.h"
#include "PlayerStriker.h"
#include "AIStriker.h"
#include "Eyes.h"
#include "BlackHole.h"
#include "Meteorite.h"
#include "GameManager.h"

void BossEncounter::Begin()
{
	End();

	m_active = true;
	m_blackHoleSpawnTimer = 2.f;
	m_meteoriteSpawnTimer = 2.f;
}

void BossEncounter::End()
{
	ClearHazards();
	m_active = false;
	m_rescueActive = false;
	m_rescueAttempted = false;
	m_playerHit = false;
	m_meteoriteGameOver = false;
	m_savedVelocity = { 0.f, 0.f };
}

void BossEncounter::OnGoal()
{
	ClearHazards();
	m_rescueActive = false;
	m_rescueAttempted = false;
}

bool BossEncounter::Update(float dt, int playerScore, Puck& puck, PlayerStriker& player, AIStriker& ai, Eyes& eyes, ui32& seed)
{
	if (m_active == false)
		return false;

	eyes.SetEyesColor(cpu::ToColor(212, 13, 13));

	if (playerScore >= 1 && eyes.GetFSM()->state == 2)
		eyes.GetFSM()->ToState(6);

	if (m_rescueActive)
	{
		puck.SetVelocity({ 0.f, 0.f });

		if (eyes.GetFSM()->state == 2 || eyes.GetFSM()->state == 6)
		{
			m_rescueActive = false;
			m_rescueAttempted = true;
			ai.SetSimulationActive(true);
			puck.SetVelocity({ m_savedVelocity.x, -fabsf(m_savedVelocity.y) });
		}
		return false;
	}

	ai.Update(&puck);

	if (TryStartRescue(playerScore, puck, ai, seed))
	{
		m_rescueActive = true;
		m_savedVelocity = puck.GetVelocity();
		puck.SetVelocity({ 0.f, 0.f });
		ai.SetSimulationActive(false);
		eyes.GetFSM()->ToState(3);
		return false;
	}

	UpdateHazards(dt, playerScore, puck, player, seed);
	return false;
}

void BossEncounter::ClearHazards()
{
	for (BlackHole* blackHole : m_blackHoles)
	{
		blackHole->Destroy();
		delete blackHole;
	}
	m_blackHoles.clear();

	for (Meteorite* meteorite : m_meteorites)
	{
		meteorite->Destroy();
		delete meteorite;
	}
	m_meteorites.clear();
}

bool BossEncounter::TryStartRescue(int playerScore, Puck& puck, AIStriker& ai, ui32& seed)
{
	const XMFLOAT2 position = puck.GetPosition();
	const XMFLOAT2 velocity = puck.GetVelocity();

	if (m_rescueAttempted || velocity.y <= 0.1f || position.y < 2.2f)
	{
		m_rescueAttempted = false;
		return false;
	}

	const float timeToGoal = (5.1f - position.y) / velocity.y;

	if (timeToGoal <= 0.f || timeToGoal > 1.f)
		return false;

	const float predictedX = position.x + velocity.x * timeToGoal;
	const float interceptDistance = sqrtf((predictedX - ai.GetPosition().x) * (predictedX - ai.GetPosition().x) + (5.3f - ai.GetPosition().y) * (5.3f - ai.GetPosition().y));
	const float interceptTime = fmaxf(0.f, interceptDistance - 0.4f) / 13.f;

	if (interceptTime <= timeToGoal)
		return false;

	m_rescueAttempted = true;
	return cpu::Rand01(seed) < 0.5f;
}

void BossEncounter::UpdateHazards(float dt, int playerScore, Puck& puck, PlayerStriker& player, ui32& seed)
{
	m_playerHit = false;
	const bool blackHolesEnabled = playerScore == 1 || playerScore >= 3;
	const bool meteoritesEnabled = playerScore >= 2;

	if (blackHolesEnabled)
	{
		m_blackHoleSpawnTimer -= dt;
		if (m_blackHoleSpawnTimer <= 0.f && m_blackHoles.size() < 2)
		{
			BlackHole* blackHole = new BlackHole;
			blackHole->Create({ cpu::RandSigned(seed) * 1.55f, 0.32f, cpu::RandSigned(seed) * 4.8f });
			m_blackHoles.push_back(blackHole);
			m_blackHoleSpawnTimer = 5.f + cpu::Rand01(seed) * 4.f;
		}
	}

	if (meteoritesEnabled)
	{
		m_meteoriteSpawnTimer -= dt;
		if (m_meteoriteSpawnTimer <= 0.f && m_meteorites.size() < 3)
		{
			Meteorite* meteorite = new Meteorite;
			meteorite->Create({ cpu::RandSigned(seed) * 1.55f, 2.4f, -1.5f - cpu::Rand01(seed) * 3.5f });
			m_meteorites.push_back(meteorite);
			m_meteoriteSpawnTimer = 1.5f + cpu::Rand01(seed) * 2.5f;
		}
	}

	for (size_t i = 0; i < m_blackHoles.size(); )
	{
		BlackHole* blackHole = m_blackHoles[i];
		blackHole->Update(dt, puck);
		if (blackHole->IsFinished())
		{
			blackHole->Destroy();
			delete blackHole;
			m_blackHoles.erase(m_blackHoles.begin() + i);
			continue;
		}
		++i;
	}

	for (size_t i = 0; i < m_meteorites.size(); )
	{
		Meteorite* meteorite = m_meteorites[i];
		meteorite->Update(dt);
		if (meteorite->Hits(player))
		{
			m_pGameManager->ShakeCamera(0.3f);
			m_pGameManager->GetDebris().Spawn(meteorite->GetPosition(), m_pGameManager->GetSeed(), cpu::ToColor(110, 62, 28), 15);

			meteorite->Destroy();
			delete meteorite;
			m_meteorites.erase(m_meteorites.begin() + i);
			m_playerHit = true;
			m_meteoriteGameOver = true;
			continue;
		}
		if (meteorite->IsFinished())
		{
			m_pGameManager->ShakeCamera(0.3f);
			m_pGameManager->GetDebris().Spawn(meteorite->GetPosition(), m_pGameManager->GetSeed(), cpu::ToColor(110, 62, 28), 15);

			meteorite->Destroy();
			delete meteorite;
			m_meteorites.erase(m_meteorites.begin() + i);
			continue;
		}
		++i;
	}
}
