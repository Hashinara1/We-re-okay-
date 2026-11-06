#include "pch.h"
#include "Meteorite.h"
#include "PlayerStriker.h"

void Meteorite::Create(const XMFLOAT3& position)
{
	m_mesh.CreateSphere(RADIUS, 10, 10, cpu::ToColor(110, 62, 28), cpu::ToColor(110, 62, 28));
	m_shadowMesh.CreateCylinder(0.004f, RADIUS, 16, true, true, cpu::ToColor(90, 15, 12));
	m_entity = cpuEngine.CreateEntity();
	m_entity->pMesh = &m_mesh;
	m_entity->transform.SetPosition(position.x, position.y, position.z);
	m_entity->transform.SetScaling(1.f);
	m_entity->visible = true;

	m_shadow = cpuEngine.CreateEntity();
	m_shadow->pMesh = &m_shadowMesh;
	m_shadow->transform.SetPosition(position.x, 0.06f, position.z);
	m_shadow->visible = true;
	m_startY = position.y;

	m_emitter = cpuEngine.CreateParticleEmitter();
	m_emitter->active = true;
	m_emitter->rate = 5.f;
	m_emitter->spawnRadius = 0.015f;
	m_emitter->pos = position;
	m_emitter->dir = { 0.f, 1.f, 0.f };
	m_emitter->colorMin = cpu::ToColor(255, 210, 35);
	m_emitter->colorMax = cpu::ToColor(245, 65, 12);
	m_emitter->durationMin = 0.2f;
	m_emitter->durationMax = 0.55f;
	m_emitter->spread = 0.75f;
	m_lifetime = LIFETIME;
	m_finished = false;
}

void Meteorite::Destroy()
{
	m_entity = cpuEngine.Release(m_entity);
	m_shadow = cpuEngine.Release(m_shadow);
	if (m_emitter)
	{
		m_emitter->rate = 0.f;
		m_emitter->active = false;
		m_emitter = cpuEngine.Release(m_emitter);
	}
}

void Meteorite::Update(float dt)
{
	m_lifetime -= dt;
	m_entity->transform.pos.y -= FALL_SPEED * dt;
	m_emitter->pos = m_entity->transform.pos;

	const float shadowScale = cpu::Clamp(m_entity->transform.pos.y / m_startY, 0.12f, 1.f);
	m_shadow->transform.SetPosition(m_entity->transform.pos.x, 0.06f, m_entity->transform.pos.z);
	m_shadow->transform.SetScaling({ shadowScale, 1.f, shadowScale });

	if (m_lifetime <= 0.f || m_entity->transform.pos.y < 0.2f)
		m_finished = true;
}

bool Meteorite::Hits(PlayerStriker& striker)
{
	if (m_finished)
		return false;

	if (cpu::SphereSphere(m_entity->sphere.center, m_entity->sphere.radius, striker.GetEntity()->sphere.center, striker.GetEntity()->sphere.radius))
	{
		if (cpu::ObbObb(m_entity->obb, striker.GetEntity()->obb))
			return true;
	}

	return false;
}
