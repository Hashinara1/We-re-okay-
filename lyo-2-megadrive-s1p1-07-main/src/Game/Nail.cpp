#include "pch.h"
#include "Nail.h"

void Nail::Create(cpu_mesh* mesh)
{
	m_entity = cpuEngine.CreateEntity();
	m_entity->pMesh = mesh;
	m_entity->visible = false;
	m_arrivalPending = false;
	m_driven = false;
}

void Nail::Destroy()
{
	m_entity = cpuEngine.Release(m_entity);
	m_arrivalPending = false;
}

void Nail::SetPosition(const XMFLOAT3& position)
{
	m_entity->transform.SetPosition(position.x, m_driven ? m_entity->transform.pos.y : position.y, position.z);
}

void Nail::SetY(float y)
{
	m_entity->transform.pos.y = y;
}

void Nail::SetDriven(float y)
{
	SetY(y);
	m_entity->visible = false;
	m_arrivalPending = false;
	m_driven = true;
}

void Nail::SetVisible(bool visible)
{
	m_entity->visible = visible;
}

bool Nail::IsVisible() const
{
	return m_entity->visible;
}

void Nail::BeginArrival(const XMFLOAT3& position, float dropY)
{
	m_entity->transform.SetPosition(position.x, dropY, position.z);
	m_entity->visible = false;
	m_arrivalStartY = dropY;
	m_arrivalPending = true;
	m_driven = false;
}

bool Nail::UpdateArrival(float timer, float delay, float duration, float raisedY)
{
	if (m_arrivalPending == false)
		return true;

	const float progress = cpu::Clamp((timer - delay) / duration, 0.f, 1.f);
	if (timer >= delay)
	{
		m_entity->visible = true;
		m_entity->transform.pos.y = cpu::Lerp(m_arrivalStartY, raisedY, progress);
	}
	if (progress >= 1.f)
		m_arrivalPending = false;

	return m_arrivalPending == false;
}

XMFLOAT3 Nail::GetPosition() const
{
	return m_entity->transform.pos;
}

void Nail::Reset()
{
	m_entity->visible = false;
	m_arrivalPending = false;
	m_arrivalStartY = 0.f;
	m_driven = false;
}
