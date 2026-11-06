#pragma once

class Puck;

class BlackHole
{
public:
	void Create(const XMFLOAT3& position);
	void Destroy();
	void Update(float dt, Puck& puck);
	bool IsFinished() const { return m_lifetime <= 0.f; }

private:
	cpu_mesh m_mesh;
	cpu_entity* m_entity = nullptr;
	cpu_particle_emitter* m_emitter = nullptr;
	float m_lifetime = 7.f;
	float m_phase = 0.f;

	static constexpr float RADIUS = 0.28f;
};
