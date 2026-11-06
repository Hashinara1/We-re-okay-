#pragma once

class Striker
{
public:
	Striker();
	virtual ~Striker();

	void Destroy();
	void ResetRound(const XMFLOAT3& pos);

	cpu_entity* GetEntity() const { return m_pEntity; }
	XMFLOAT2 GetPosition() const;
	XMFLOAT2 GetVelocity() const { return m_velocity; }
	XMFLOAT2 GetHalfExtents() const;

protected:
	void CreateEntity(const XMFLOAT3& p_pos, cpu_mesh* p_mesh);

	cpu_entity* m_pEntity = nullptr;
	XMFLOAT2 m_velocity = { 0.f, 0.f };
};
