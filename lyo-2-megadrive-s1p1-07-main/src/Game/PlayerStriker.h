#pragma once

#include "Striker.h"

class PlayerStriker : public Striker
{
public:
	enum class BackstopType { None, Guardian, Cages };

	PlayerStriker();
	~PlayerStriker();

	void Create(const XMFLOAT3& p_pos, cpu_mesh* p_mesh);

	void Update();
	void SetMovementEnabled(bool enabled);
	void SetBackstopType(BackstopType type) { m_backstopType = type; }

private:
	bool m_movementEnabled = true;
	bool m_mousePositionReset = false;
	BackstopType m_backstopType = BackstopType::None;

	XMFLOAT2 m_center = { 0.f, 0.f };
};

