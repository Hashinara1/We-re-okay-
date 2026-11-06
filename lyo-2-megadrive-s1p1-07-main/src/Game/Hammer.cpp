#include "pch.h"
#include "Hammer.h"

void Hammer::BuildMesh(cpu_mesh& mesh)
{
	mesh.Clear();
	mesh.AddBox({ HEAD_OFFSET, 0.f, 0.f }, { 0.07f, HEAD_HALF_HEIGHT, 0.07f }, cpu::ToColor(150, 150, 150));
	mesh.AddBox({ HEAD_OFFSET * 0.5f, 0.f, 0.f }, { HEAD_OFFSET * 0.5f, 0.025f, 0.025f }, cpu::ToColor(110, 62, 28));
	mesh.Optimize();
}

void Hammer::Create(cpu_mesh* mesh)
{
	m_entity = cpuEngine.CreateEntity();
	m_entity->pMesh = mesh;
	m_entity->visible = false;
}

void Hammer::Destroy()
{
	m_entity = cpuEngine.Release(m_entity);
}

void Hammer::StartStrike(const XMFLOAT3& nailPosition, float nailTipOffset, float headOffset)
{
	m_nailTipY = nailPosition.y + nailTipOffset;
	m_pivot = { nailPosition.x - headOffset, m_nailTipY + HEAD_HALF_HEIGHT, nailPosition.z };
	m_impactY = 0.f;
	Update(0.f, nailPosition, nailTipOffset);
	m_entity->visible = true;
}

void Hammer::Update(float progress, const XMFLOAT3& nailPosition, float nailTipOffset)
{
	float angle = IMPACT_ANGLE;

	if (progress < IMPACT_PROGRESS)
	{
		const float swing = progress / IMPACT_PROGRESS;
		angle = cpu::Lerp(WINDUP_ANGLE, IMPACT_ANGLE, swing * swing);
	}
	else if (progress > DRIVEN_PROGRESS)
	{
		const float lift = (progress - DRIVEN_PROGRESS) / (1.f - DRIVEN_PROGRESS);
		angle = cpu::Lerp(IMPACT_ANGLE, LIFT_ANGLE, lift);
	}

	m_impactY = nailPosition.y + nailTipOffset - m_nailTipY;
	m_entity->transform.SetPosition(m_pivot.x, m_pivot.y + m_impactY, m_pivot.z);
	m_entity->transform.SetYPR(0.f, 0.f, angle);
}

void Hammer::SetVisible(bool visible)
{
	m_entity->visible = visible;
}
