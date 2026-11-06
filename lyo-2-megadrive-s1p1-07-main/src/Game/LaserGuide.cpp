#include "pch.h"
#include "LaserGuide.h"
#include "ShopManager.h"
#include "Puck.h"

void LaserGuide::Create()
{
	m_meshLaser.CreateBox(0.02f, 0.02f, LASER_LENGTH * 0.5f, BonusCatalog::Get(BonusId::Laser).color);
	m_meshBounceMarker.CreateSphere(0.08f, 8, 8, BonusCatalog::Get(BonusId::Laser).color, BonusCatalog::Get(BonusId::Laser).color);

	for (int i = 0; i < SEGMENT_COUNT; ++i)
	{
		m_segments[i] = cpuEngine.CreateEntity();
		m_segments[i]->pMesh = &m_meshLaser;
		m_segments[i]->visible = false;
	}

	for (int i = 0; i < MAX_BOUNCES; ++i)
	{
		m_bounceMarkers[i] = cpuEngine.CreateEntity();
		m_bounceMarkers[i]->pMesh = &m_meshBounceMarker;
		m_bounceMarkers[i]->visible = false;
	}
}

void LaserGuide::Destroy()
{
	for (cpu_entity*& segment : m_segments)
	{
		segment = cpuEngine.Release(segment);
	}

	for (cpu_entity*& marker : m_bounceMarkers)
	{
		marker = cpuEngine.Release(marker);
	}
}

void LaserGuide::Hide()
{
	for (cpu_entity* segment : m_segments)
	{
		if (segment != nullptr)
			segment->visible = false;
	}

	for (cpu_entity* marker : m_bounceMarkers)
	{
		if (marker != nullptr)
			marker->visible = false;
	}
}

void LaserGuide::Update(float dt, bool allowCoinSpawn)
{
	const bool shouldShow = m_owner.m_inventory.HasPassive(BonusId::Laser) && m_owner.m_pPuck->GetEntity()->visible;

	if (shouldShow == false)
	{
		Hide();
		return;
	}

	const int bounceLimit = m_owner.m_ownedCages ? 2 : MAX_BOUNCES;
	XMFLOAT2 predicted[MAX_BOUNCES + 1];
	bool bouncePoints[MAX_BOUNCES + 1] = {};
	const int pointCount = m_owner.m_pPuck->PredictTrajectory(predicted, bouncePoints, bounceLimit, LASER_LENGTH);

	if (pointCount == 0)
	{
		Hide();
		return;
	}

	const XMFLOAT2 start = m_owner.m_pPuck->GetPosition();
	XMFLOAT2 previous = start;

	int segmentIndex = 0;
	int bounceIndex = 0;

	for (int i = 0; i < pointCount; ++i)
	{
		if (segmentIndex >= SEGMENT_COUNT)
			break;

		const float dx = predicted[i].x - previous.x;
		const float dz = predicted[i].y - previous.y;
		const float length = sqrtf(dx * dx + dz * dz);

		cpu_entity* segment = m_segments[segmentIndex++];
		segment->visible = length > CPU_EPSILON;

		if (segment->visible)
		{
			segment->transform.SetPosition(previous.x + dx * 0.5f, LASER_Y, previous.y + dz * 0.5f);
			segment->transform.SetScaling({ 1.f, 1.f, length / LASER_LENGTH });
			segment->transform.SetYPR(atan2f(dx, dz), 0.f, 0.f);
		}

		if (bouncePoints[i] && bounceIndex < MAX_BOUNCES)
		{
			cpu_entity* marker = m_bounceMarkers[bounceIndex++];
			marker->visible = true;
			marker->transform.SetPosition(predicted[i].x, LASER_Y, predicted[i].y);
		}
		previous = predicted[i];
	}

	for (int i = segmentIndex; i < SEGMENT_COUNT; ++i)
		m_segments[i]->visible = false;

	for (int i = bounceIndex; i < MAX_BOUNCES; ++i)
		m_bounceMarkers[i]->visible = false;
}
