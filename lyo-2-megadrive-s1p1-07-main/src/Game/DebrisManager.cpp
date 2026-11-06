#include "pch.h"
#include "DebrisManager.h"

void DebrisManager::Create()
{
	m_mesh.CreateBox(1.f, 1.f, 1.f, CPU_WHITE);
}

void DebrisManager::Destroy()
{
	for (Debris& debris : m_debris)
		debris.entity = cpuEngine.Release(debris.entity);

	m_debris.clear();
}

void DebrisManager::Spawn(const XMFLOAT3& origin, ui32& seed, const XMFLOAT3& color, int count, float halfSize, float lifetime, float horizontalSpeed, float verticalSpeedMin, float verticalSpeedRange, const XMFLOAT3& spin, float gravity)
{
	auto material = std::make_shared<cpu_material>();
	material->color = color;

	for (int i = 0; i < count; ++i)
	{
		Debris debris;
		debris.material = material;
		debris.entity = cpuEngine.CreateEntity();
		debris.entity->pMesh = &m_mesh;
		debris.entity->pMaterial = material.get();
		debris.entity->transform.SetPosition(origin.x, origin.y, origin.z);
		debris.entity->transform.SetScaling({ halfSize, halfSize, halfSize });
		debris.velocity =
		{
			cpu::RandSigned(seed) * horizontalSpeed,
			cpu::Rand01(seed) * verticalSpeedRange + verticalSpeedMin,
			cpu::RandSigned(seed) * horizontalSpeed
		};
		debris.spin = spin;
		debris.gravity = gravity;
		debris.maxLifetime = lifetime;
		m_debris.push_back(debris);
	}
}

void DebrisManager::Update(float dt)
{
	for (size_t i = 0; i < m_debris.size(); )
	{
		Debris& debris = m_debris[i];
		debris.lifetime += dt;
		debris.velocity.y -= debris.gravity * dt;
		debris.entity->transform.pos.x += debris.velocity.x * dt;
		debris.entity->transform.pos.y += debris.velocity.y * dt;
		debris.entity->transform.pos.z += debris.velocity.z * dt;

		if (debris.spin.x != 0.f || debris.spin.y != 0.f || debris.spin.z != 0.f)
			debris.entity->transform.AddYPR(dt * debris.spin.x, dt * debris.spin.y, dt * debris.spin.z);

		if (debris.lifetime >= debris.maxLifetime)
		{
			debris.entity = cpuEngine.Release(debris.entity);
			m_debris[i] = std::move(m_debris.back());
			m_debris.pop_back();
			continue;
		}

		++i;
	}
}