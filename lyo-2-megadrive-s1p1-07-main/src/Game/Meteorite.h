#pragma once

class PlayerStriker;

class Meteorite
{
public:
	void Create(const XMFLOAT3& position);
	void Destroy();
	void Update(float dt);
	bool Hits(PlayerStriker& striker);
	bool IsFinished() const { return m_finished; }
	XMFLOAT3 GetPosition() const { return m_entity->transform.pos; }

private:
	cpu_mesh m_mesh;
	cpu_entity* m_entity = nullptr;
	cpu_mesh m_shadowMesh;
	cpu_entity* m_shadow = nullptr;
	float m_startY = 0.f;
	cpu_particle_emitter* m_emitter = nullptr;
	float m_lifetime = 0.f;
	bool m_finished = false;

	static constexpr float RADIUS = 0.13f;
	static constexpr float FALL_SPEED = 1.5f;
	static constexpr float LIFETIME = 4.f;
};
