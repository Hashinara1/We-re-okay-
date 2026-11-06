#include "pch.h"
#include "Puck.h"
#include "PlayerStriker.h"
#include "AIStriker.h"

static float Dot2D(const XMFLOAT2& p_a, const XMFLOAT2& p_b)
{
	return p_a.x * p_b.x + p_a.y * p_b.y;
}

static float Length2D(const XMFLOAT2& p_v)
{
	return sqrtf(p_v.x * p_v.x + p_v.y * p_v.y);
}

static XMFLOAT2 Reflect2D(const XMFLOAT2& p_velocity, const XMFLOAT2& p_normal)
{
	float pushingIn = Dot2D(p_velocity, p_normal);
	return { p_velocity.x - 2.f * pushingIn * p_normal.x, p_velocity.y - 2.f * pushingIn * p_normal.y };
}

Puck::Puck()
{
}

Puck::~Puck()
{
}

void Puck::Create(const XMFLOAT3& p_pos)
{
	m_mesh.CreateCylinder(0.08f, m_radius, 8, true, true, CPU_ORANGE);
	m_pEntity = cpuEngine.CreateEntity();
	m_pEntity->pMesh = &m_mesh;
	m_pEntity->transform.SetPosition(p_pos.x, p_pos.y, p_pos.z);
}

void Puck::Destroy()
{
	m_pEntity = cpuEngine.Release(m_pEntity);
}

void Puck::Update(const PlayerStriker* p_pStriker, const AIStriker* p_pAIStriker)
{
	float dt = cpuTime.delta;
	Move(dt, p_pStriker, p_pAIStriker);
}

void Puck::AddObstacle(const XMFLOAT2& p_center, const XMFLOAT2& p_halfExtents, float p_restitution)
{
	m_obstacles.push_back({ p_center, p_halfExtents, cpu::Clamp(p_restitution, 0.f, 1.f) });
}

void Puck::ClearObstacles()
{
	m_obstacles.clear();
}

XMFLOAT2 Puck::GetPosition() const
{
	return { m_pEntity->transform.pos.x, m_pEntity->transform.pos.z };
}

int Puck::PredictTrajectory(XMFLOAT2* points, bool* bouncePoints, int maxBounces, float maxDistance) const
{
	if (points == nullptr || bouncePoints == nullptr || maxBounces < 0 || maxDistance <= 0.f)
		return 0;

	XMFLOAT2 position = GetPosition();
	XMFLOAT2 velocity = m_velocity;

	const float speed = Length2D(velocity);

	if (speed <= CPU_EPSILON)
		return 0;

	const float invSpeed = 1.f / speed;
	velocity = { velocity.x * invSpeed, velocity.y * invSpeed };

	int pointCount = 0;

	float remainingDistance = maxDistance;
	const float xLimit = TABLE_HALF_WIDTH - m_radius;

	while (pointCount <= maxBounces && remainingDistance > CPU_EPSILON)
	{
		float distanceToWall = FLT_MAX;
		int bounceAxis = -1;
		float boundary = 0.f;

		if (velocity.x > CPU_EPSILON)
		{
			const float distance = (xLimit - position.x) / velocity.x;

			if (distance >= 0.f)
			{
				distanceToWall = distance;
				bounceAxis = 0;
				boundary = xLimit;
			}
		}
		else if (velocity.x < -CPU_EPSILON)
		{
			const float distance = (-xLimit - position.x) / velocity.x;

			if (distance >= 0.f)
			{
				distanceToWall = distance;
				bounceAxis = 0;
				boundary = -xLimit;
			}
		}

		for (const Obstacle& obstacle : m_obstacles)
		{
			const float minX = obstacle.center.x - obstacle.halfExtents.x - m_radius;
			const float maxX = obstacle.center.x + obstacle.halfExtents.x + m_radius;
			const float minZ = obstacle.center.y - obstacle.halfExtents.y - m_radius;
			const float maxZ = obstacle.center.y + obstacle.halfExtents.y + m_radius;

			float distance = FLT_MAX;
			int axis = -1;
			float obstacleBoundary = 0.f;

			if (velocity.x > CPU_EPSILON && position.x <= minX && position.y >= minZ && position.y <= maxZ)
			{
				distance = (minX - position.x) / velocity.x;
				axis = 0;
				obstacleBoundary = minX;
			}
			else if (velocity.x < -CPU_EPSILON && position.x >= maxX && position.y >= minZ && position.y <= maxZ)
			{
				distance = (maxX - position.x) / velocity.x;
				axis = 0;
				obstacleBoundary = maxX;
			}
			else if (velocity.y > CPU_EPSILON && position.y <= minZ && position.x >= minX && position.x <= maxX)
			{
				distance = (minZ - position.y) / velocity.y;
				axis = 1;
				obstacleBoundary = minZ;
			}
			else if (velocity.y < -CPU_EPSILON && position.y >= maxZ && position.x >= minX && position.x <= maxX)
			{
				distance = (maxZ - position.y) / velocity.y;
				axis = 1;
				obstacleBoundary = maxZ;
			}

			if (axis >= 0 && distance >= 0.f && distance < distanceToWall)
			{
				distanceToWall = distance;
				bounceAxis = axis;
				boundary = obstacleBoundary;
			}
		}

		if (bounceAxis >= 0 && distanceToWall < remainingDistance && pointCount == maxBounces)
		{
			position.x += velocity.x * distanceToWall;
			position.y += velocity.y * distanceToWall;

			if (bounceAxis == 0)
			{
				position.x = boundary;
			}
			else
			{
				position.y = boundary;
			}

			bouncePoints[pointCount] = false;
			points[pointCount++] = position;

			break;
		}

		if (distanceToWall >= remainingDistance || bounceAxis < 0)
		{
			position.x += velocity.x * remainingDistance;
			position.y += velocity.y * remainingDistance;

			bouncePoints[pointCount] = false;
			points[pointCount++] = position;

			break;
		}

		position.x += velocity.x * distanceToWall;
		position.y += velocity.y * distanceToWall;

		if (bounceAxis == 0)
		{
			position.x = boundary;
			velocity.x = -velocity.x;
		}
		else
		{
			position.y = boundary;
			velocity.y = -velocity.y;
		}

		bouncePoints[pointCount] = true;
		points[pointCount++] = position;
		remainingDistance -= distanceToWall;
	}

	return pointCount;
}

void Puck::Move(float p_dt, const PlayerStriker* p_pStriker, const AIStriker* p_pAIStriker)
{
	m_ignoreAIStrikerThisUpdate = false;
	float maxStrikerSpeed = 0.f;

	if (p_pStriker != nullptr)
		maxStrikerSpeed = Length2D(p_pStriker->GetVelocity());

	if (p_pAIStriker != nullptr)
	{
		float aiSpeed = Length2D(p_pAIStriker->GetVelocity());

		if (aiSpeed > maxStrikerSpeed)
			maxStrikerSpeed = aiSpeed;
	}

	float relativeTravel = (Length2D(m_velocity) + maxStrikerSpeed) * p_dt;
	int steps = (int)(ceilf(relativeTravel / (m_radius * 0.5f)));

	if (steps < 1)
		steps = 1;
	else if (steps > 32)
		steps = 32;

	float step = p_dt / (float)steps;

	for (int i = 0; i < steps; ++i)
	{
		ApplyTableFriction(step);

		m_pEntity->transform.pos.x += m_velocity.x * step;
		m_pEntity->transform.pos.z += m_velocity.y * step;
		BounceOnSideWalls();

		for (const Obstacle& obstacle : m_obstacles)
			BounceOnObstacle(obstacle);

		float remainingTime = p_dt - (i + 1) * step;

		if (p_pStriker != nullptr)
		{
			XMFLOAT2 strikerPos = p_pStriker->GetPosition();
			XMFLOAT2 strikerVelocity = p_pStriker->GetVelocity();
			strikerPos.x -= strikerVelocity.x * remainingTime;
			strikerPos.y -= strikerVelocity.y * remainingTime;
			BounceOnStriker(strikerPos, p_pStriker->GetHalfExtents(), strikerVelocity, false, 0.f);
		}
		if (p_pAIStriker != nullptr)
		{
			XMFLOAT2 strikerPos = p_pAIStriker->GetPosition();
			XMFLOAT2 strikerVelocity = p_pAIStriker->GetVelocity();
			strikerPos.x -= strikerVelocity.x * remainingTime;
			strikerPos.y -= strikerVelocity.y * remainingTime;
			BounceOnStriker(strikerPos, p_pAIStriker->GetHalfExtents(), strikerVelocity, true, p_pAIStriker->GetDifficulty());
		}

		ClampSpeed();
	}
	m_ignoreAIStrikerThisUpdate = false;
}

void Puck::ApplyTableFriction(float p_dt)
{
	if (Length2D(m_velocity) < MIN_SPEED)
		return;

	float keep = 1.f - m_tableFriction * p_dt;

	if (keep < 0.f)
		keep = 0.f;

	m_velocity.x *= keep;
	m_velocity.y *= keep;
}

void Puck::BounceOnSideWalls()
{
	float limit = TABLE_HALF_WIDTH - m_radius;

	if (m_pEntity->transform.pos.x < -limit)
	{
		m_pEntity->transform.pos.x = -limit;

		if (m_velocity.x < 0.f)
			m_velocity.x = -m_velocity.x * WALL_BOUNCE;
	}
	else if (m_pEntity->transform.pos.x > limit)
	{
		m_pEntity->transform.pos.x = limit;

		if (m_velocity.x > 0.f)
			m_velocity.x = -m_velocity.x * WALL_BOUNCE;
	}
}

void Puck::BounceOnObstacle(const Obstacle& p_obstacle)
{
	float closestX = cpu::Clamp(m_pEntity->transform.pos.x, p_obstacle.center.x - p_obstacle.halfExtents.x, p_obstacle.center.x + p_obstacle.halfExtents.x);
	float closestZ = cpu::Clamp(m_pEntity->transform.pos.z, p_obstacle.center.y - p_obstacle.halfExtents.y, p_obstacle.center.y + p_obstacle.halfExtents.y);

	float dx = m_pEntity->transform.pos.x - closestX;
	float dz = m_pEntity->transform.pos.z - closestZ;
	float distance = sqrtf(dx * dx + dz * dz);

	if (distance >= m_radius)
		return; 

	XMFLOAT2 normal;

	if (distance > CPU_EPSILON)
		normal = { dx / distance, dz / distance };
	else
		normal = { 0.f, 1.f };

	m_pEntity->transform.pos.x += normal.x * (m_radius - distance);
	m_pEntity->transform.pos.z += normal.y * (m_radius - distance);

	if (Dot2D(m_velocity, normal) < 0.f)
	{
		XMFLOAT2 reflected = Reflect2D(m_velocity, normal);
		m_velocity = { reflected.x * p_obstacle.restitution, reflected.y * p_obstacle.restitution };
	}
}

void Puck::BounceOnStriker(const XMFLOAT2& p_strikerPos, const XMFLOAT2& p_halfExtents, const XMFLOAT2& p_strikerVelocity, bool isAIStriker, float strikerSkill)
{
	if (m_ignoreAIStrikerThisUpdate && isAIStriker)
		return;
	if (m_suddenDeathActive && isAIStriker)
	{
		const XMFLOAT2 puckVelocity = m_velocity;
		const XMFLOAT2 strikerVelocity = p_strikerVelocity;

		const float velocityDot = (puckVelocity.x - strikerVelocity.x) * (m_pEntity->transform.pos.x - p_strikerPos.x) + (puckVelocity.y - strikerVelocity.y) * (m_pEntity->transform.pos.z - p_strikerPos.y);

		if (velocityDot >= 0.f)
			return;

		float closestX = cpu::Clamp(m_pEntity->transform.pos.x, p_strikerPos.x - p_halfExtents.x, p_strikerPos.x + p_halfExtents.x);
		float closestZ = cpu::Clamp(m_pEntity->transform.pos.z, p_strikerPos.y - p_halfExtents.y, p_strikerPos.y + p_halfExtents.y);
		float dx = m_pEntity->transform.pos.x - closestX;
		float dz = m_pEntity->transform.pos.z - closestZ;

		if (dx * dx + dz * dz >= m_radius * m_radius)
			return;

		XMFLOAT2 normal;

		if (dx * dx + dz * dz > CPU_EPSILON * CPU_EPSILON)
		{
			const float inverseDistance = 1.f / sqrtf(dx * dx + dz * dz);
			normal = { dx * inverseDistance, dz * inverseDistance };
		}
		else
		{
			normal = { 0.f, puckVelocity.y >= 0.f ? -1.f : 1.f };
		}

		m_pEntity->transform.pos.x += normal.x * m_radius;
		m_pEntity->transform.pos.z += normal.y * m_radius;

		m_velocity = puckVelocity;

		m_suddenDeathHit = true;
		m_suddenDeathActive = false;

		m_ignoreAIStrikerThisUpdate = true;

		return;
	}

	float closestX = cpu::Clamp(m_pEntity->transform.pos.x, p_strikerPos.x - p_halfExtents.x, p_strikerPos.x + p_halfExtents.x);
	float closestZ = cpu::Clamp(m_pEntity->transform.pos.z, p_strikerPos.y - p_halfExtents.y, p_strikerPos.y + p_halfExtents.y);

	float dx = m_pEntity->transform.pos.x - closestX;
	float dz = m_pEntity->transform.pos.z - closestZ;
	float distance = sqrtf(dx * dx + dz * dz);

	if (distance >= m_radius)
		return;

	XMFLOAT2 relative = { m_velocity.x - p_strikerVelocity.x, m_velocity.y - p_strikerVelocity.y };
	XMFLOAT2 normal;

	float penetration = m_radius - distance;

	if (distance > CPU_EPSILON)
	{
		normal = { dx / distance, dz / distance };
	}
	else
	{
		float left = m_pEntity->transform.pos.x - (p_strikerPos.x - p_halfExtents.x);
		float right = (p_strikerPos.x + p_halfExtents.x) - m_pEntity->transform.pos.x;
		float nearZ = m_pEntity->transform.pos.z - (p_strikerPos.y - p_halfExtents.y);
		float farZ = (p_strikerPos.y + p_halfExtents.y) - m_pEntity->transform.pos.z;
		float horizontalFace = (left < right) ? left : right;
		float depthFace = (nearZ < farZ) ? nearZ : farZ;

		if (horizontalFace <= depthFace)
		{
			float direction = left < right ? -1.f : 1.f;

			if (fabsf(left - right) <= CPU_EPSILON && relative.x != 0.f)
				direction = (relative.x < 0.f) ? 1.f : -1.f;

			normal = { direction, 0.f };
			penetration += horizontalFace;
		}
		else
		{
			float direction = nearZ < farZ ? -1.f : 1.f;

			if (fabsf(nearZ - farZ) <= CPU_EPSILON && relative.y != 0.f)
				direction = (relative.y < 0.f) ? 1.f : -1.f;

			normal = { 0.f, direction };
			penetration += depthFace;
		}
	}

	m_pEntity->transform.pos.x += normal.x * penetration;
	m_pEntity->transform.pos.z += normal.y * penetration;

	if (Dot2D(relative, normal) >= 0.f)
		return;

	XMFLOAT2 reflected = Reflect2D(relative, normal);

	float impactResponse = 0.3f;
	float strikerDrive = 1.f;

	if(isAIStriker)
	{
		impactResponse = cpu::Lerp(0.35f, 0.60f, strikerSkill);
		strikerDrive = cpu::Lerp(0.95f, 1.50f, strikerSkill);
		m_velocity = { reflected.x * impactResponse + p_strikerVelocity.x * strikerDrive, reflected.y * impactResponse + p_strikerVelocity.y * strikerDrive };
	}
	else
	{
		m_velocity = { reflected.x * impactResponse + p_strikerVelocity.x * strikerDrive, reflected.y * impactResponse + p_strikerVelocity.y * strikerDrive };
		m_velocity.x *= m_strikerVelocityMultiplier;
		m_velocity.y *= m_strikerVelocityMultiplier;
	}
}

void Puck::ClampSpeed()
{
	float speed = Length2D(m_velocity);
	if (speed > MAX_SPEED)
	{
		const float scale = MAX_SPEED / speed;
		m_velocity.x *= scale;
		m_velocity.y *= scale;
	}
}
