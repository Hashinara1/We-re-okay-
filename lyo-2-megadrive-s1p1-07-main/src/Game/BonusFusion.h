#pragma once

#include "pch.h"
#include "BonusCatalog.h"

class ShopManager;
struct ShopItem;

class BonusFusion
{
public:
	BonusFusion(ShopManager& owner);

	bool IsActive() const { return m_active; }

	void Begin(ShopItem& dragged, ShopItem& target, int resultSlot, BonusId resultId);
	void Update(float dt);
	void Release();
	void Cancel();

	void Destroy();

private:
	void Reset();

	ShopManager& m_owner;

	cpu_entity* m_entities[2] = {};

	XMFLOAT3 m_starts[2] = {};

	float m_elapsed = 0.f;

	int m_resultSlot = -1;

	BonusId m_resultId = BonusId::Count;

	bool m_returningToSlot = false;
	bool m_active = false;

	cpu_particle_emitter* m_explosionEmitter = nullptr;
	float m_explosionTimer = 0.f;
};