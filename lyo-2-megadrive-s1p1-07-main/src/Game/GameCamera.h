#pragma once

class GameCamera
{
public:
	void Create();
	void Shake(float duration);
	void UpdateShake(float dt);

	void MoveToGame();
	void MoveToMenu();
	void MoveTo(const XMFLOAT3& position);

	void UpdateTransition(float dt);
	bool IsTransitioning() const;

	XMFLOAT3 GetPosition() const;
	XMFLOAT3 GetTargetPosition() const;

private:
	XMFLOAT3 m_gamePosition = { 0.f, 2.f, -8.3f };
	XMFLOAT3 m_menuPosition = { -1.3f, 2.5f, 3.f };

	float m_shakeTimer = 0.f;
	float m_shakePhase = 0.f;

	bool m_transitioning = false;
	float m_transitionProgress = 0.f;
	XMFLOAT3 m_transitionFrom = { 0.f, 0.f, 0.f };
	XMFLOAT3 m_transitionTo = { 0.f, 0.f, 0.f };

	static constexpr float TRANSITION_DURATION = 1.f;
	static constexpr float ARRIVAL_EPSILON = 0.1f;
};
