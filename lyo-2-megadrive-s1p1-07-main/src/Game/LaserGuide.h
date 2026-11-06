#pragma once

#include "pch.h"

class ShopManager;

class LaserGuide
{
public:
	LaserGuide(ShopManager& owner) : m_owner(owner) {}

	void Create();
	void Destroy();
	void Hide();
	void Update(float dt, bool allowCoinSpawn);

private:
	ShopManager& m_owner;

	cpu_mesh m_meshLaser;
	cpu_mesh m_meshBounceMarker;

	cpu_entity* m_segments[4] = {};
	cpu_entity* m_bounceMarkers[3] = {};

	static constexpr int MAX_BOUNCES = 3;
	static constexpr int SEGMENT_COUNT = MAX_BOUNCES + 1;
	static constexpr float LASER_Y = 0.14f;
	static constexpr float LASER_LENGTH = 8.f;
};
