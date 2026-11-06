#pragma once

#include "pch.h"

class Nail
{
public:
	void Create(cpu_mesh* mesh);
	void Destroy();
	void SetPosition(const XMFLOAT3& position);
	void SetY(float y);
	void SetDriven(float y);
	void SetVisible(bool visible);
	bool IsVisible() const;
	bool IsArrivalPending() const { return m_arrivalPending; }
	void BeginArrival(const XMFLOAT3& position, float dropY);
	bool UpdateArrival(float timer, float delay, float duration, float raisedY);
	XMFLOAT3 GetPosition() const;
	void Reset();

private:
	cpu_entity* m_entity = nullptr;
	bool m_arrivalPending = false;
	float m_arrivalStartY = 0.f;
	bool m_driven = false;
};
