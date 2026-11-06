#include "pch.h"
#include "AIStriker.h"
#include "AIState.h"
#include "Puck.h"

float Length2D(const XMFLOAT2& v)
{
	return sqrtf(v.x * v.x + v.y * v.y);
}

XMFLOAT2 Normalize2D(const XMFLOAT2& v)
{
	const float length = Length2D(v);
	return length < CPU_EPSILON ? XMFLOAT2{ 0.f, 0.f } : XMFLOAT2{ v.x / length, v.y / length };
}

float MirrorIntoRange(float value, float limit)
{
	const float period = limit * 4.f;
	float x = fmodf(value + limit, period);

	if (x < 0.f)
		x += period;

	if (x > limit * 2.f)
		x = period - x;

	return x - limit;
}

AIStriker::AIStriker()
{
	m_seed = timeGetTime();
	SetDifficulty(m_difficulty);
}

AIStriker::~AIStriker()
{
}

void AIStriker::Create(const XMFLOAT3& p_pos, cpu_mesh* p_mesh)
{
	CreateEntity(p_pos, p_mesh);
	m_pEntity->pMaterial = &m_material;
	m_material.color = CPU_GREEN;

	m_target = { p_pos.x, p_pos.z };

	m_fsm = cpuEngine.CreateFSM(this);
	m_fsm->Add<StateAIDefend>("Defend");
	m_fsm->Add<StateAIAttack>("Attack");
	m_fsm->Add<StateAIConfused>("Confused");
	m_fsm->Add<StateAIFrozen>("Frozen");
	m_fsm->ToState((int)BehaviorState::Defend);
}

void AIStriker::Destroy()
{
	m_fsm = cpuEngine.Release(m_fsm);
	Striker::Destroy();
}

void AIStriker::ResetRound(const XMFLOAT3& pos)
{
	Striker::ResetRound(pos);
	m_target = { pos.x, pos.z };
	m_behaviorTime = 0.f;
	m_confusedTargetTimer = 0.f;

	m_frozen = false;
	m_confused = false;
}

void AIStriker::SetDifficulty(float level)
{
	m_difficulty = cpu::Clamp(level, 0.f, 1.f);
	m_baseDifficulty = m_difficulty;
	m_skill = 0.2f + 0.7f * sqrtf(m_difficulty);
	m_maxSpeed = cpu::Lerp(6.f, 13.f, m_skill);
	m_acceleration = cpu::Lerp(18.f, 75.f, m_skill);
	m_reactionTime = cpu::Lerp(0.3f, 0.055f, m_skill);
	m_aimError = cpu::Lerp(0.4f, 0.02f, m_skill);
	m_predictionBlend = cpu::Lerp(0.6f, 0.97f, m_skill);
	m_attackSpeedLimit = cpu::Lerp(1.4f, 8.f, m_skill);
}

void AIStriker::SetColor(const XMFLOAT3& color)
{
	m_material.color = color;
}

void AIStriker::Update(const Puck* puck)
{
	m_currentPuck = puck;

	if (puck && m_simulationActive)
		SelectBehavior(*puck);
}

void AIStriker::SelectBehavior(const Puck& puck)
{
	const XMFLOAT2 pos = puck.GetPosition();
	const XMFLOAT2 velocity = puck.GetVelocity();
	BehaviorState next = BehaviorState::Defend;

	if (m_frozen)
	{
		next = BehaviorState::Frozen;
	}
	else if (m_confused)
	{
		next = BehaviorState::Confused;
	}
	else
	{
		const bool inZone = pos.y > 1.5f + puck.GetRadius() && pos.y < 5.f + puck.GetRadius();
		const bool enteringZone = pos.y > 0.8f && velocity.y > 0.12f;
		const bool reachable = Length2D({ pos.x - GetPosition().x, pos.y - GetPosition().y }) < 6.3f;
		const bool slow = Length2D(velocity) < m_attackSpeedLimit;
		const bool threat = pos.y > 2.2f && velocity.y > 0.12f;

		if (reachable && ((inZone && slow) || (enteringZone && slow) || threat))
			next = BehaviorState::Attack;
	}

	const int desiredState = (int)next;
	if (m_fsm->state != desiredState)
		m_fsm->ToState(desiredState);
}

void AIStriker::UpdateDefend(const Puck& puck)
{
	const float dt = cpuTime.delta;

	m_behaviorTime += dt;

	const XMFLOAT2 puckPos = puck.GetPosition();
	const XMFLOAT2 puckVelocity = puck.GetVelocity();

	m_target = ComputeDefendTarget(puckPos, puckVelocity, puck.GetRadius());

	const float phase = m_behaviorTime * (1.35f + m_difficulty * 0.5f);

	m_target.x += sinf(phase) * DEFENSE_SWAY_X;
	m_target.y += sinf(phase * 0.73f + 1.2f) * DEFENSE_SWAY_Z;
	m_target.y += cpu::Clamp((puckPos.y - 3.f) * 0.08f, -0.18f, 0.18f);

	const XMFLOAT2 half = GetHalfExtents();

	m_target.x = cpu::Clamp(m_target.x, -2.f + half.x, 2.f - half.x);
	m_target.y = cpu::Clamp(m_target.y, 1.5f + half.y, 5.f - half.y);

	MoveToTarget(dt);
}

void AIStriker::UpdateAttack(const Puck& puck)
{
	const float dt = cpuTime.delta;

	m_behaviorTime += dt;

	XMFLOAT2 target;

	if (ComputeAttackTarget(puck.GetPosition(), puck.GetVelocity(), puck.GetRadius(), target))
	{
		m_target = target;
	}
	else
	{
		m_target = ComputeDefendTarget(puck.GetPosition(), puck.GetVelocity(), puck.GetRadius());
	}

	m_aimTimer -= dt;

	if (m_aimTimer <= 0.f)
	{
		m_aimOffset = cpu::RandSigned(m_seed) * m_aimError;
		m_aimTimer = m_reactionTime;
	}

	m_target.x += m_aimOffset;
	MoveToTarget(dt);
}

void AIStriker::UpdateConfused(float dt)
{
	m_behaviorTime += dt;
	m_confusedTargetTimer -= dt;

	if (m_confusedTargetTimer <= 0.f)
		ChooseConfusedTarget();

	m_target = m_confusedTarget;
	MoveToTarget(dt, 0.8f);
}

void AIStriker::UpdateFrozen(const Puck& puck, float dt)
{
	m_behaviorTime += dt;

	const XMFLOAT2 predicted = ComputeDefendTarget(puck.GetPosition(), puck.GetVelocity(), puck.GetRadius());
	m_target = predicted;

	const float speed = Length2D(m_velocity);

	if (speed > FROZEN_MAX_SPEED)
	{
		const float scale = FROZEN_MAX_SPEED / speed;
		m_velocity.x *= scale;
		m_velocity.y *= scale;
	}

	const float speedScale = cpu::Clamp(FROZEN_MAX_SPEED / m_maxSpeed, 0.f, 1.f);
	MoveToTarget(dt, speedScale);
}

void AIStriker::ChooseConfusedTarget()
{
	const XMFLOAT2 half = GetHalfExtents();

	const float minX = -2.f + half.x;
	const float maxX = 2.f - half.x;
	const float minZ = 1.5f + half.y;
	const float maxZ = 5.f - half.y;

	m_confusedTarget = { cpu::Lerp(minX, maxX, cpu::Rand01(m_seed)), cpu::Lerp(minZ, maxZ, cpu::Rand01(m_seed)) };
	m_confusedTargetTimer = cpu::Lerp(0.18f, 0.55f, cpu::Rand01(m_seed));
}

XMFLOAT2 AIStriker::ComputeDefendTarget(const XMFLOAT2& puckPos, const XMFLOAT2& puckVelocity, float puckRadius) const
{
	const XMFLOAT2 half = GetHalfExtents();

	const float homeZ = 5.f - half.y - HOME_MARGIN;
	float targetX = puckPos.x * 0.78f;

	if (puckVelocity.y > 0.35f)
	{
		const float time = cpu::Clamp((homeZ - puckPos.y) / puckVelocity.y, 0.f, 1.2f);
		const float predictedX = MirrorIntoRange(puckPos.x + puckVelocity.x * time, 2.f - puckRadius);
		targetX = cpu::Lerp(puckPos.x, predictedX, m_predictionBlend);
	}

	const float clearance = half.x + puckRadius + KEEP_AWAY;

	if (puckPos.y > homeZ - (half.y + puckRadius) && fabsf(targetX - puckPos.x) < clearance)
		targetX = puckPos.x + (puckPos.x >= 0.f ? -clearance : clearance);

	return { targetX, homeZ };
}

bool AIStriker::ComputeAttackTarget(const XMFLOAT2& puckPos, const XMFLOAT2& puckVelocity, float puckRadius, XMFLOAT2& outTarget) const
{
	const XMFLOAT2 myPos = GetPosition();
	const XMFLOAT2 half = GetHalfExtents();

	const float maxX = 2.f - half.x;
	const float minZ = 1.5f + half.y;
	const float maxZ = 5.f - half.y;
	const float puckMinX = -2.f + puckRadius;
	const float puckMaxX = 2.f - puckRadius;
	const float puckMinZ = 1.5f + puckRadius;
	const float puckMaxZ = 5.f - puckRadius;
	const float contactRange = sqrtf((half.x + puckRadius) * (half.x + puckRadius) + (half.y + puckRadius) * (half.y + puckRadius));
	const float timeToEdge = puckVelocity.y < -0.01f ? fmaxf(0.f, (puckPos.y - (puckMinZ + 0.08f)) / -puckVelocity.y) : 0.8f;
	const float horizon = cpu::Clamp(timeToEdge, 0.04f, 1.2f);
	const float accelerationTime = m_maxSpeed / fmaxf(m_acceleration, CPU_EPSILON);

	float bestGap = FLT_MAX;
	XMFLOAT2 meet = { cpu::Clamp(puckPos.x, puckMinX, puckMaxX), cpu::Clamp(puckPos.y, puckMinZ, puckMaxZ) };

	for (float t = 0.f; t <= horizon + 0.001f; t += 0.035f)
	{
		const XMFLOAT2 candidate = { MirrorIntoRange(puckPos.x + puckVelocity.x * t, 2.f - puckRadius), cpu::Clamp(puckPos.y + puckVelocity.y * t, puckMinZ, puckMaxZ) };
		const float distance = Length2D({ candidate.x - myPos.x, candidate.y - myPos.y });

		float reachableDistance = 0.f;

		if (t < accelerationTime)
		{
			reachableDistance = 0.5f * m_acceleration * t * t;
		}
		else
		{
			reachableDistance = 0.5f * m_acceleration * accelerationTime * accelerationTime + m_maxSpeed * (t - accelerationTime);
		}

		const float gap = distance - reachableDistance - contactRange;

		if (gap < bestGap)
		{
			bestGap = gap;
			meet = candidate;
		}

		if (gap <= 0.f)
		{
			meet = candidate;
			break;
		}
	}

	const XMFLOAT2 hitDir = Normalize2D({ -meet.x, -5.f - meet.y });
	XMFLOAT2 behind = { meet.x - hitDir.x * APPROACH_DISTANCE, meet.y - hitDir.y * APPROACH_DISTANCE };

	float pushZOffset = 0.f;

	const float urgentTime = cpu::Lerp(0.35f, 0.8f, m_skill);
	const bool urgent = puckVelocity.y < -0.12f && timeToEdge < urgentTime;

	if (urgent)
	{
		outTarget = meet;
		return true;
	}

	if (behind.y > maxZ || fabsf(behind.x) > maxX)
	{
		const float side = meet.x >= 0.f ? 1.f : -1.f;
		const float sideX = cpu::Clamp(meet.x - side * (half.x + puckRadius + 0.08f), -maxX, maxX);
		const float sideZ = cpu::Clamp(meet.y + half.y + puckRadius * 0.5f, minZ, maxZ);

		behind = { sideX, sideZ };
		pushZOffset = -SIDE_PUSH_OFFSET;
	}

	const XMFLOAT2 fromPuck = Normalize2D({ myPos.x - meet.x, myPos.y - meet.y });
	const float alignment = -(fromPuck.x * hitDir.x + fromPuck.y * hitDir.y);
	const float distance = Length2D({ myPos.x - meet.x, myPos.y - meet.y });

	if (alignment > ALIGNED_DOT && distance < 1.15f)
	{
		outTarget = { cpu::Clamp(meet.x + hitDir.x * PUSH_THROUGH, -maxX, maxX), cpu::Clamp(meet.y + pushZOffset + hitDir.y * PUSH_THROUGH, minZ, maxZ) };
	}
	else
	{
		outTarget = behind;
	}

	return true;
}

void AIStriker::MoveToTarget(float dt, float speedScale)
{
	if (dt <= CPU_EPSILON)
		return;

	const XMFLOAT2 pos = GetPosition();
	const XMFLOAT2 toTarget = { m_target.x - pos.x, m_target.y - pos.y };

	const float distance = Length2D(toTarget);

	XMFLOAT2 desired = { 0.f, 0.f };

	if (distance > CPU_EPSILON)
	{
		const float speed = fminf(m_maxSpeed * speedScale, distance * ARRIVE_GAIN);
		const float scale = speed / distance;
		desired = { toTarget.x * scale, toTarget.y * scale };
	}

	XMFLOAT2 change = { desired.x - m_velocity.x, desired.y - m_velocity.y };

	const float changeLength = Length2D(change);
	const float maxChange = m_acceleration * speedScale * dt;

	if (changeLength > maxChange && changeLength > CPU_EPSILON)
	{
		const float scale = maxChange / changeLength;
		change.x *= scale;
		change.y *= scale;
	}

	m_velocity.x += change.x;
	m_velocity.y += change.y;

	const XMFLOAT2 half = GetHalfExtents();

	m_pEntity->transform.pos.x = cpu::Clamp(pos.x + m_velocity.x * dt, -2.f + half.x, 2.f - half.x);
	m_pEntity->transform.pos.z = cpu::Clamp(pos.y + m_velocity.y * dt, 1.5f + half.y, 5.f - half.y);

	const float invDt = 1.f / dt;
	m_velocity.x = (m_pEntity->transform.pos.x - pos.x) * invDt;
	m_velocity.y = (m_pEntity->transform.pos.z - pos.y) * invDt;

	if (speedScale * m_maxSpeed <= FROZEN_MAX_SPEED + CPU_EPSILON)
	{
		const float finalSpeed = Length2D(m_velocity);
		if (finalSpeed > FROZEN_MAX_SPEED)
		{
			const float scale = FROZEN_MAX_SPEED / finalSpeed;
			m_velocity.x *= scale;
			m_velocity.y *= scale;
		}
	}
}

