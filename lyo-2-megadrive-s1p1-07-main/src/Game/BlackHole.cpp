#include "pch.h"
#include "BlackHole.h"
#include "Puck.h"

void BlackHole::Create(const XMFLOAT3& position)
{
	m_mesh.CreateSphere(RADIUS, 12, 12, CPU_BLACK, CPU_BLACK);
	m_entity = cpuEngine.CreateEntity();
	m_entity->pMesh = &m_mesh;
	m_entity->transform.SetPosition(position.x, position.y, position.z);

	m_emitter = cpuEngine.CreateParticleEmitter();
	m_emitter->active = true;
	m_emitter->pos = position;
	m_emitter->colorMin = cpu::ToColor(80, 45, 125);
	m_emitter->colorMax = cpu::ToColor(190, 110, 255);
	m_emitter->durationMin = 0.15f;
	m_emitter->durationMax = 0.35f;
	m_emitter->spread = 1.f;
}

void BlackHole::Destroy()
{
	m_entity = cpuEngine.Release(m_entity);
	if (m_emitter)
	{
		m_emitter->rate = 0.f;
		m_emitter->active = false;
		m_emitter = cpuEngine.Release(m_emitter);
	}
}

void BlackHole::Update(float dt, Puck& puck)
{
	m_lifetime -= dt;
	m_phase += dt * 2.4f;
	const float pulse = 0.82f + 0.18f * sinf(m_phase);
	m_entity->transform.SetScaling({ pulse, pulse, pulse });
	m_emitter->pos = m_entity->transform.pos;

	const XMFLOAT2 position = puck.GetPosition();
	const XMFLOAT2 velocity = puck.GetVelocity();
	const XMFLOAT2 delta = { m_entity->transform.pos.x - position.x, m_entity->transform.pos.z - position.y };
	const float distanceSquared = delta.x * delta.x + delta.y * delta.y;
	const float influenceRadius = 4.f;
	if (distanceSquared < influenceRadius * influenceRadius && distanceSquared > CPU_EPSILON * CPU_EPSILON)
	{
		const float distance = sqrtf(distanceSquared);
		const float invDistance = 1.f / distance;
		const float force = 20.f * (1.f - distance / influenceRadius);
		const float forceDt = force * dt * invDistance;
		puck.SetVelocity({ velocity.x + delta.x * forceDt, velocity.y + delta.y * forceDt });
	}
}
