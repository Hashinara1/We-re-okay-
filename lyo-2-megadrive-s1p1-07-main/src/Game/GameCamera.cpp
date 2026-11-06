#include "pch.h"
#include "GameCamera.h"

void GameCamera::Create()
{
	cpu_camera* camera = cpuEngine.GetCamera();
	camera->transform.SetPosition(m_menuPosition.x, m_menuPosition.y, m_menuPosition.z);
	camera->transform.AddYPR(0.f, 0.25f);
	camera->fov = XM_PIDIV4 / 1.2f;
	camera->UpdateProjection();
}

void GameCamera::Shake(float duration)
{
	if (m_shakeTimer > 0.f)
		return;

	m_shakeTimer = duration;
	m_shakePhase = 0.f;
}

void GameCamera::UpdateShake(float dt)
{
	cpu_camera* camera = cpuEngine.GetCamera();

	if (m_shakeTimer > 0.f)
	{
		m_shakeTimer -= dt;
		m_shakePhase += dt * 40.f;
		const float amount = m_shakeTimer * 0.05f;
		camera->transform.SetPosition(m_gamePosition.x + sinf(m_shakePhase) * amount, m_gamePosition.y + cosf(m_shakePhase * 1.4f) * amount, m_gamePosition.z);
	}
	else
	{
		camera->transform.SetPosition(m_gamePosition.x, m_gamePosition.y, m_gamePosition.z);
	}
}

void GameCamera::MoveToGame()
{
	MoveTo(m_gamePosition);
}

void GameCamera::MoveToMenu()
{
	MoveTo(m_menuPosition);
}

void GameCamera::MoveTo(const XMFLOAT3& position)
{
	const XMFLOAT3 current = GetPosition();
	const float dx = position.x - current.x;
	const float dy = position.y - current.y;
	const float dz = position.z - current.z;

	if (dx * dx + dy * dy + dz * dz < ARRIVAL_EPSILON * ARRIVAL_EPSILON)
	{
		m_transitioning = false;
		cpuEngine.GetCamera()->transform.SetPosition(position.x, position.y, position.z);
		return;
	}

	m_transitionFrom = current;
	m_transitionTo = position;
	m_transitionProgress = 0.f;
	m_transitioning = true;
}

void GameCamera::UpdateTransition(float dt)
{
	if (m_transitioning == false)
		return;

	m_transitionProgress = cpu::Clamp(m_transitionProgress + dt / TRANSITION_DURATION, 0.f, 1.f);

	const float t = m_transitionProgress * m_transitionProgress * (3.f - 2.f * m_transitionProgress);

	cpuEngine.GetCamera()->transform.SetPosition(
		cpu::Lerp(m_transitionFrom.x, m_transitionTo.x, t),
		cpu::Lerp(m_transitionFrom.y, m_transitionTo.y, t),
		cpu::Lerp(m_transitionFrom.z, m_transitionTo.z, t));

	if (m_transitionProgress >= 1.f)
		m_transitioning = false;
}

bool GameCamera::IsTransitioning() const
{
	return m_transitioning;
}

XMFLOAT3 GameCamera::GetPosition() const
{
	return cpuEngine.GetCamera()->transform.pos;
}

XMFLOAT3 GameCamera::GetTargetPosition() const
{
	return m_transitioning ? m_transitionTo : GetPosition();
}
