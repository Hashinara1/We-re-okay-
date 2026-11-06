#pragma once

#include "pch.h"

class Hammer
{
public:
	static constexpr float HEAD_OFFSET = 0.25f;
	static constexpr float IMPACT_PROGRESS = 0.4f;
	static constexpr float DRIVEN_PROGRESS = 0.65f;

	static void BuildMesh(cpu_mesh& mesh);

	void Create(cpu_mesh* mesh);
	void Destroy();
	void StartStrike(const XMFLOAT3& nailPosition, float nailTipOffset, float headOffset);
	void Update(float progress, const XMFLOAT3& nailPosition, float nailTipOffset);
	void SetVisible(bool visible);

private:
	cpu_entity* m_entity = nullptr;
	XMFLOAT3 m_pivot = { 0.f, 0.f, 0.f };
	float m_nailTipY = 0.f;
	float m_impactY = 0.f;

	static constexpr float HEAD_HALF_HEIGHT = 0.1f;
	static constexpr float WINDUP_ANGLE = 0.9f;
	static constexpr float IMPACT_ANGLE = 0.f;
	static constexpr float LIFT_ANGLE = 0.5f;
};
