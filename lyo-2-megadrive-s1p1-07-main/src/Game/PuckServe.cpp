#include "pch.h"
#include "PuckServe.h"
#include "Puck.h"

void PuckServe::LaunchRandom(Puck& puck, float direction, ui32& seed)
{
	float ratio = HORIZONTAL_RATIO_MIN + cpu::Rand01(seed) * (HORIZONTAL_RATIO_MAX - HORIZONTAL_RATIO_MIN);

	if (cpu::Rand01(seed) < 0.5f)
		ratio = -ratio;

	const float vertical = sqrtf(1.f - ratio * ratio) * direction;
	const float speed = PUCK_SPEED_MIN + cpu::Rand01(seed) * (PUCK_SPEED_MAX - PUCK_SPEED_MIN);

	puck.GetEntity()->visible = true;
	puck.SetVelocity({ ratio * speed, vertical * speed });
}

float PuckServe::GetNextDropY(float currentY, float dt)
{
	const float progress = cpu::Clamp((PUCK_DROP_HEIGHT - currentY) / (PUCK_DROP_HEIGHT - PUCK_Y), 0.f, 1.f);
	return currentY - cpu::Lerp(PUCK_DROP_SPEED_MIN, PUCK_DROP_SPEED_MAX, progress * progress) * dt;
}
