#pragma once

#include "pch.h"

namespace VersusRules
{
	constexpr float DURATION = 60.f;
	constexpr float REWARD_FAST_TIME = 20.f;
	constexpr float REWARD_SLOW_TIME = 60.f;
	constexpr int REWARD_FAST = 20;
	constexpr int REWARD_SLOW = 7;

	inline int ComputeReward(float elapsed)
	{
		const float clamped = cpu::Clamp(elapsed, REWARD_FAST_TIME, REWARD_SLOW_TIME);
		const float ratio = (clamped - REWARD_FAST_TIME) / (REWARD_SLOW_TIME - REWARD_FAST_TIME);

		return REWARD_FAST - static_cast<int>(ratio * (REWARD_FAST - REWARD_SLOW) + 0.5f);
	}
}

class VersusClock
{
public:
	void Reset()
	{
		m_elapsed = 0.f;
		m_remaining = VersusRules::DURATION;
		m_running = false;
		m_overtime = false;
	}

	void Start() { m_running = true; }
	void Stop() { m_running = false; }

	bool Update(float dt)
	{
		if (m_running == false)
			return false;

		m_elapsed += dt;

		if (m_overtime)
			return false;

		m_remaining -= dt;

		if (m_remaining > 0.f)
			return false;

		m_remaining = 0.f;
		m_running = false;

		return true;
	}

	void EnterOvertime()
	{
		m_overtime = true;
		m_remaining = 0.f;
		m_running = true;
	}

	float GetRemaining() const { return m_remaining; }
	float GetElapsed() const { return m_elapsed; }
	bool IsOvertime() const { return m_overtime; }

private:
	float m_elapsed = 0.f;
	float m_remaining = VersusRules::DURATION;
	bool m_running = false;
	bool m_overtime = false;
};
