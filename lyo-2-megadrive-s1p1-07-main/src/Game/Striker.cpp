#include "pch.h"
#include "Striker.h"

Striker::Striker()
{
}

Striker::~Striker()
{
}

void Striker::CreateEntity(const XMFLOAT3& p_pos, cpu_mesh* p_mesh)
{
	m_pEntity = cpuEngine.CreateEntity();
	m_pEntity->pMesh = p_mesh;
	m_pEntity->transform.SetPosition(p_pos.x, p_pos.y, p_pos.z);
	m_velocity = { 0.f, 0.f };
}

void Striker::Destroy()
{
	m_pEntity = cpuEngine.Release(m_pEntity);
}

void Striker::ResetRound(const XMFLOAT3& p_pos)
{
	m_pEntity->transform.SetPosition(p_pos.x, p_pos.y, p_pos.z);
	m_velocity = { 0.f, 0.f };
}

XMFLOAT2 Striker::GetPosition() const
{
	return { m_pEntity->transform.pos.x, m_pEntity->transform.pos.z };
}

XMFLOAT2 Striker::GetHalfExtents() const
{
	return { m_pEntity->obb.half.x, m_pEntity->obb.half.z };
}
