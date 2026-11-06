#pragma once

class Puck;
class PlayerStriker;
class AIStriker;
class Eyes;
class BlackHole;
class Meteorite;
class GameManager;

class BossEncounter
{
public:
	void SetGameManager(GameManager* p_gameManager) { m_pGameManager = p_gameManager; };
	void Begin();
	void End();
	void OnGoal();
	bool Update(float dt, int playerScore, Puck& puck, PlayerStriker& player, AIStriker& ai, Eyes& eyes, ui32& seed);
	bool IsActive() const { return m_active; }
	int GetScoreToWin(int normalScoreToWin) const { return m_active ? 4 : normalScoreToWin; }
	bool IsRescueActive() const { return m_rescueActive; }
	bool ConsumePlayerHit() { const bool hit = m_playerHit; m_playerHit = false; return hit; }
	bool ConsumeMeteoriteGameOver() { const bool gameOver = m_meteoriteGameOver; m_meteoriteGameOver = false; return gameOver; }

private:
	void ClearHazards();
	bool TryStartRescue(int playerScore, Puck& puck, AIStriker& ai, ui32& seed);
	void UpdateHazards(float dt, int playerScore, Puck& puck, PlayerStriker& player, ui32& seed);

	bool m_active = false;
	bool m_rescueActive = false;
	bool m_rescueAttempted = false;
	bool m_playerHit = false;
	bool m_meteoriteGameOver = false;

	XMFLOAT2 m_savedVelocity = { 0.f, 0.f };

	float m_blackHoleSpawnTimer = 0.f;
	float m_meteoriteSpawnTimer = 0.f;

	std::vector<BlackHole*> m_blackHoles;
	std::vector<Meteorite*> m_meteorites;

	GameManager* m_pGameManager = nullptr;
};