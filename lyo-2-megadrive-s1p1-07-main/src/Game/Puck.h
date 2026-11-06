#pragma once

class PlayerStriker;
class AIStriker;

class Puck
{
public:
	Puck();
	~Puck();

	void Create(const XMFLOAT3& p_pos);
	void Destroy();
	void Update(const PlayerStriker* p_pStriker, const AIStriker* p_pAIStriker);

	void AddObstacle(const XMFLOAT2& p_center, const XMFLOAT2& p_halfExtents, float p_restitution = 0.65f);
	void ClearObstacles();

	void SetVelocity(const XMFLOAT2& p_velocity) { m_velocity = p_velocity; }
	void SetTableFriction(float friction) { m_tableFriction = friction; }
	void SetStrikerVelocityMultiplier(float p_multiplier) { m_strikerVelocityMultiplier = p_multiplier; }
	void SetSuddenDeathActive(bool active) { m_suddenDeathActive = active; }
	bool ConsumeSuddenDeathHit() { const bool hit = m_suddenDeathHit; m_suddenDeathHit = false; return hit; }

	cpu_entity* GetEntity() { return m_pEntity; }
	XMFLOAT2 GetPosition() const;
	XMFLOAT2 GetVelocity() const { return m_velocity; }
	float GetRadius() const { return m_radius; }
	int PredictTrajectory(XMFLOAT2* points, bool* bouncePoints, int maxBounces, float maxDistance) const;

private:
	struct Obstacle
	{
		XMFLOAT2 center;      
		XMFLOAT2 halfExtents; 
		float restitution;   
	};

	void Move(float p_dt, const PlayerStriker* p_pStriker, const AIStriker* p_pAIStriker);
	void ApplyTableFriction(float p_dt);
	void BounceOnSideWalls();
	void BounceOnObstacle(const Obstacle& p_obstacle);
	void BounceOnStriker(const XMFLOAT2& p_strikerPos, const XMFLOAT2& p_halfExtents, const XMFLOAT2& p_strikerVelocity, bool isAIStriker, float strikerSkill);
	void ClampSpeed();

	cpu_mesh m_mesh;
	cpu_entity* m_pEntity = nullptr;

	XMFLOAT2 m_velocity = { 0.f, 0.f };
	std::vector<Obstacle> m_obstacles;

	float m_radius = 0.18f;
	float m_strikerVelocityMultiplier = 1.f;
	float m_tableFriction = 0.25f;

	bool m_suddenDeathActive = false;
	bool m_suddenDeathHit = false;
	bool m_ignoreAIStrikerThisUpdate = false;

	static constexpr float TABLE_HALF_WIDTH = 2.f; 
	static constexpr float WALL_BOUNCE = 0.85f;    
	static constexpr float MIN_SPEED = 0.7f;     
	static constexpr float MAX_SPEED = 25.f;       
};
