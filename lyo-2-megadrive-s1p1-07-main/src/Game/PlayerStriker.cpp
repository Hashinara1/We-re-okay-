#include "pch.h"
#include "PlayerStriker.h"
#include "Table.h"

PlayerStriker::PlayerStriker()
{
}

PlayerStriker::~PlayerStriker()
{
}

void PlayerStriker::Create(const XMFLOAT3& p_pos, cpu_mesh* p_mesh)
{
	CreateEntity(p_pos, p_mesh);

	m_center = { cpuDevice.GetWidth() / 2.f, cpuDevice.GetHeight() / 2.f };
}

void PlayerStriker::SetMovementEnabled(bool enabled)
{
	if (enabled && m_movementEnabled == false)
		m_mousePositionReset = true;

	m_movementEnabled = enabled;
}

void PlayerStriker::Update()
{
	if (m_movementEnabled == false)
	{
		m_velocity = { 0.f, 0.f };
		return;
	}

	float dt = cpu::Clamp(cpuTime.delta, 0.f, 0.05f);

	XMFLOAT2 mousePos = { 0, 0 };
	cpuEngine.GetCursor(mousePos);
	cpuEngine.SetCursor(m_center);

	if (m_mousePositionReset)
	{
		m_mousePositionReset = false;
		m_velocity = { 0.f, 0.f };
		return;
	}

	XMFLOAT2 mouseDelta = { 0, 0 };

	mouseDelta.x = m_center.x - mousePos.x;
	mouseDelta.y = m_center.y - mousePos.y;

	mouseDelta.x = cpu::Clamp(mouseDelta.x, -15.f, 15.f);
	mouseDelta.y = cpu::Clamp(mouseDelta.y, -15.f, 15.f);	

	const float halfLength = m_pEntity->obb.half.x;
	const float halfWidth = m_pEntity->obb.half.z;
	const float scaleX = fabsf(m_pEntity->transform.sca.x);
	const float scaleZ = fabsf(m_pEntity->transform.sca.z);
	const float strikerHalfX = halfLength * scaleX;
	const float strikerHalfZ = halfWidth * scaleZ;

	const XMFLOAT2 currentPosition = { m_pEntity->transform.pos.x, m_pEntity->transform.pos.z };

	float targetX = currentPosition.x - mouseDelta.x * dt;
	float targetZ = currentPosition.y + mouseDelta.y * dt;
	targetX = cpu::Clamp(targetX, -2.f + strikerHalfX, 2.f - strikerHalfX);

	const float tableMinZ = -5.5f + strikerHalfZ;
	float minAllowedZ = tableMinZ;

	if (m_backstopType == BackstopType::Guardian)
	{
		const float wallFront = -5.5f + 0.1f;
		minAllowedZ = fmaxf(minAllowedZ, wallFront + strikerHalfZ);
	}
	else if (m_backstopType == BackstopType::Cages)
	{
		const float wallHalfX = 0.5f;
		const float wallHalfZ = 0.24f;
		const float wallCenterX[2] = { -1.5f, 1.5f };

		for (float wallX : wallCenterX)
		{
			if (fabsf(targetX - wallX) <= wallHalfX + strikerHalfX)
			{
				const float wallFront = -5.5f + wallHalfZ;
				minAllowedZ = fmaxf(minAllowedZ, wallFront + strikerHalfZ);
			}
		}
	}

	targetZ = cpu::Clamp(targetZ, minAllowedZ, -1.5f - strikerHalfZ);

	m_pEntity->transform.pos.x = targetX;
	m_pEntity->transform.pos.z = targetZ;
	if (dt > CPU_EPSILON)
	{
		const float invDt = 1.f / dt;
		m_velocity.x = (targetX - currentPosition.x) * invDt;
		m_velocity.y = (targetZ - currentPosition.y) * invDt;
	}
}
