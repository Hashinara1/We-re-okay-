#pragma once

class Puck;

namespace PuckServe
{
	constexpr float PUCK_Y = 0.2f;
	constexpr float PUCK_DROP_HEIGHT = 4.f;

	constexpr float PUCK_SPEED_MIN = 5.f;
	constexpr float PUCK_SPEED_MAX = 8.f;
	constexpr float HORIZONTAL_RATIO_MIN = 0.22f;
	constexpr float HORIZONTAL_RATIO_MAX = 0.72f;
	constexpr float PUCK_DROP_SPEED_MIN = 5.f;
	constexpr float PUCK_DROP_SPEED_MAX = 16.f;

	void LaunchRandom(Puck& puck, float direction, ui32& seed);
	float GetNextDropY(float currentY, float dt);
}
