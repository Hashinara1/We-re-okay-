#pragma once

#include "ShopManager.h"
#include "Button3D.h"
#include "MenuManager.h"
#include "RunProgress.h"
#include "BossEncounter.h"
#include "GameState.h"
#include "GameCamera.h"
#include "DebrisManager.h"
#include "PuckServe.h"
#include "GameHud.h"
#include "NailManager.h"
#include "VersusRules.h"

class Table;
class PlayerStriker;
class AIStriker;
class Puck;
class Eyes;

class GameManager
{
public:
	GameManager();
	~GameManager();

	void Create();
	void Destroy();
	void Update();

	void OnRender(int pass);
	void ShakeCamera(float duration);

	PlayerStriker* GetPlayerStriker() { return m_pPlayerStriker; }
	AIStriker* GetAiStriker() { return m_pAIStriker; }
	Puck* GetPuck() { return m_pPuck; }
	Eyes* GetEyes() { return m_pEyes; }

	GameCamera& GetGameCamera() { return m_camera; }
	DebrisManager& GetDebris() { return m_debris; }
	ui32& GetSeed() { return m_seed; }

	int GetMoney() const { return m_money; }
	void AddMoney(int p_value) { m_money += p_value; }
	bool TrySpendMoney(int amount);

private:
	using State = GameState;

	bool UpdatePlaying(float dt);
	void UpdateShop(float dt);
	void UpdateCountdown(float dt);
	void UpdateAnimation(float dt);
	void UpdateLoweringStrikers(float amount);
	void UpdateHammeringNail(float dt);
	void UpdateNails(float dt);
	void UpdateClosingTable(float amount);
	void UpdateOpeningShopTable();
	void UpdateDroppingOffers(float dt);
	void UpdateRaisingOffers(float dt);
	void UpdateOpeningTable(float amount);
	void UpdateRaisingStrikers(float dt, float amount);
	void UpdateDroppingPuck(float dt);
	void CheckForGoal();
	void AddPoint(bool playerScored);
	void StartClosingTable();
	void DropPuck(float z);
	void SetCursorVisible(bool visible);
	void ResetGame();
	void BeginShop();
	void StartSuddenDeathGameOver();
	void StartIdleGameOver();
	void StartPlayerStrikerGameOver(bool meteoriteHit);
	void StartShop();
	void StartGame();
	void PauseGame();
	void ContinueGame();
	void RestartGame();
	bool IsMenuVisible() const;
	void ApplyCurrentStage();
	void OnMatchFinished();
	void EndVersusOnTime();
	void AwardVersusReward();
	void SpawnStrikerDebris(const XMFLOAT3& position, const XMFLOAT3& color);

	GameHud m_hud;
	GameCamera m_camera;
	DebrisManager m_debris;
	NailManager m_nailManager;

	cpu_mesh m_meshStriker;
	cpu_mesh m_meshAIStriker;

	ShopManager m_shopManager;

	float m_aiBaseDifficulty = 0.5f;

	RunProgress m_run;
	BossEncounter m_boss;
	VersusClock m_versusClock;

	Table* m_pTable = nullptr;
	PlayerStriker* m_pPlayerStriker = nullptr;
	AIStriker* m_pAIStriker = nullptr;
	Puck* m_pPuck = nullptr;
	Eyes* m_pEyes = nullptr;

	State m_state = State::Shop;

	int m_playerScore = 0;
	int m_aiScore = 0;
	int m_money = 30;

	float m_animation = 0.f;
	float m_countdown = 0.f;
	XMFLOAT3 m_playerStrikerPositionBeforeGoal = { 0.f, 0.2f, -5.f };
	XMFLOAT3 m_aiStrikerPositionBeforeGoal = { 0.f, 0.2f, 5.f };
	bool m_resumeWithCountdown = false;
	bool m_goalPuckServe = false;
	XMFLOAT2 m_savedVelocity = { 0.f, 0.f };

	bool m_startWithCountdown = false;
	bool m_closeAfterNail = false;
	bool m_suddenDeathGameOver = false;
	bool m_meteoriteGameOver = false;
	bool m_playerWonMatch = false;
	bool m_runWon = false;
	bool m_cursorVisible = true;

	ui32 m_seed = 0;

	PauseSnapshot m_pause;
	bool m_gameStarted = false;
	bool m_pauseInputBlocked = false;
	bool m_gameOverMenuPending = false;
	bool m_idleGameOver = false;
	bool m_playerServePending = false;
	float m_idleTimer = 0.f;

	static constexpr float GOAL_LINE = 5.3f;
	static constexpr int SCORE_TO_WIN = 2;
	static constexpr float TRANSITION_DURATION = 1.f;
	static constexpr float COUNTDOWN_DURATION = 3.f;
	static constexpr float PUCK_Y = PuckServe::PUCK_Y;
	static constexpr float STRIKER_START_Z = 5.f;
	static constexpr float PLAYER_SERVE_Z = -3.5f;
	static constexpr float AI_SERVE_Z = 3.5f;
	static constexpr float PUCK_DROP_HEIGHT = PuckServe::PUCK_DROP_HEIGHT;
};
