#pragma once

#include "pch.h"

class DebrisManager
{
public:
	void Create();
	void Destroy();

	void Spawn(const XMFLOAT3& origin, ui32& seed, const XMFLOAT3& color, int count = 8, float halfSize = 0.035f, float lifetime = 1.f, float horizontalSpeed = 2.f, float verticalSpeedMin = 0.8f, float verticalSpeedRange = 2.5f, const XMFLOAT3& spin = { 0.f, 0.f, 0.f }, float gravity = 5.f);
	void Update(float dt);

private:
	struct Debris
	{
		cpu_entity* entity = nullptr;
		std::shared_ptr<cpu_material> material;
		XMFLOAT3 velocity = { 0.f, 0.f, 0.f };
		XMFLOAT3 spin = { 0.f, 0.f, 0.f };
		float gravity = 5.f;
		float lifetime = 0.f;
		float maxLifetime = 1.f;
	};

	cpu_mesh m_mesh;
	std::vector<Debris> m_debris;
};