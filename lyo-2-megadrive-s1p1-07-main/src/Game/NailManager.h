#pragma once

#include "pch.h"
#include "NailDisplay.h"
#include "Nail.h"
#include "Hammer.h"

class GameManager;

class NailManager
{
public:
	static constexpr int PLAYER_SIDE = 0;
	static constexpr int AI_SIDE = 1;

	void Create(GameManager* p_gameManager);
	void Destroy();

	bool Configure(bool bossStage);
	bool UpdateArrival(float dt);
	void UpdateLayout();

	bool QueueHit(int side);
	bool IsHitPending() const { return m_nailPending; }
	void StartHit();
	bool UpdateHit(float dt);

	void Reset();

private:
	static float GetTableSign(int side);

	cpu_mesh m_meshNail;
	cpu_mesh m_meshHammer;
	Nail m_nails[2][4];
	Hammer m_hammer;
	NailDisplay m_nailDisplay[2];
	float m_nailRaiseTimer = 0.f;
	float m_hammerTimer = 0.f;
	int m_hammerSide = 0;
	int m_hammerNail = 0;
	bool m_nailPending = false;

	GameManager* m_pGameManager = nullptr;

	static constexpr float NAIL_HAMMER_DURATION = 0.55f;
	static constexpr float NAIL_RAISED_Y = 0.42f;
	static constexpr float NAIL_DRIVEN_Y = 0.14f;
	static constexpr float NAIL_SIDE_X = 2.25f;
	static constexpr float NAIL_DROP_Y = 3.5f;
	static constexpr float NAIL_ARRIVAL_DURATION = 0.45f;
	static constexpr float NAIL_ARRIVAL_DELAY = 0.09f;
	static constexpr float NAIL_TIP_OFFSET = 0.025f;
};
