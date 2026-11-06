#pragma once

#include "Striker.h"

class Puck;

class AIStriker : public Striker
{
public:
	AIStriker();
	~AIStriker();

	void Create(const XMFLOAT3& p_pos, cpu_mesh* p_mesh);
	void Destroy();
	void ResetRound(const XMFLOAT3& pos);
	void Update(const Puck* p_pPuck);
	void SetSimulationActive(bool active) { m_simulationActive = active; }

	// 0 = tres facile, 1 = tres difficile
	void SetDifficulty(float p_level);
	void SetColor(const XMFLOAT3& color);
	float GetDifficulty() const { return m_difficulty; }
	float GetBaseDifficulty() const { return m_baseDifficulty; }
	void SetConfused(bool confused) { m_confused = confused; }
	void SetFrozen(bool frozen) { m_frozen = frozen; }
	bool IsFrozen() const { return m_frozen; }
	bool IsConfused() const { return m_confused; }

	friend struct StateAIDefend;
	friend struct StateAIAttack;
	friend struct StateAIConfused;
	friend struct StateAIFrozen;

private:

	enum class BehaviorState 
	{ 
		Defend,
		Attack, 
		Confused, 
		Frozen 
	};

	void SelectBehavior(const Puck& p_puck);
	void UpdateDefend(const Puck& p_puck);
	void UpdateAttack(const Puck& p_puck);
	void UpdateConfused(float dt);
	void UpdateFrozen(const Puck& p_puck, float dt);
	void ChooseConfusedTarget();
	XMFLOAT2 ComputeDefendTarget(const XMFLOAT2& p_puckPos, const XMFLOAT2& p_puckVelocity, float p_puckRadius) const;
	bool ComputeAttackTarget(const XMFLOAT2& p_puckPos, const XMFLOAT2& p_puckVelocity, float p_puckRadius, XMFLOAT2& p_outTarget) const;
	void MoveToTarget(float p_dt, float p_speedScale = 1.f);

	cpu_material m_material;
	cpu_fsm<AIStriker>* m_fsm = nullptr;
	const Puck* m_currentPuck = nullptr;

	XMFLOAT2 m_target = { 0.f, 0.f };
	XMFLOAT2 m_confusedTarget = { 0.f, 0.f };

	float m_behaviorTime = 0.f;
	float m_confusedTargetTimer = 0.f;
	float m_aimTimer = 0.f;
	float m_aimOffset = 0.f;

	float m_difficulty = 0.5f;
	float m_skill = 1.f;
	float m_baseDifficulty = 0.5f;
	bool m_confused = false;
	bool m_frozen = false;
	bool m_simulationActive = false;
	float m_maxSpeed = 8.f;
	float m_acceleration = 50.f;
	float m_reactionTime = 0.2f;
	float m_aimError = 0.3f;
	float m_predictionBlend = 0.6f;
	float m_attackSpeedLimit = 5.f;

	ui32 m_seed = 0;

	static constexpr float HOME_MARGIN = 0.28f;
	static constexpr float APPROACH_DISTANCE = 0.42f;
	static constexpr float PUSH_THROUGH = 0.55f;
	static constexpr float ALIGNED_DOT = 0.78f;
	static constexpr float SIDE_PUSH_OFFSET = 0.36f;
	static constexpr float KEEP_AWAY = 0.12f;
	static constexpr float ARRIVE_GAIN = 14.f;
	static constexpr float DEFENSE_SWAY_X = 0.34f;
	static constexpr float DEFENSE_SWAY_Z = 0.42f;
	static constexpr float FROZEN_MAX_SPEED = 0.65f;
};
