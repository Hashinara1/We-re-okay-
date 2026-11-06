#include "pch.h"
#include "GameManager.h"
#include "Table.h"
#include "PlayerStriker.h"
#include "AIStriker.h"
#include "Puck.h"
#include "Eyes.h"
#include "ShopManager.h"
#include "Button3D.h"
#include "MenuManager.h"
#include "NailManager.h"

GameManager::GameManager() {}
GameManager::~GameManager() {}

void GameManager::Create()
{
	m_seed = timeGetTime();

	cpuEngine.m_groundColor = CPU_BLACK;
	cpuEngine.m_skyColor = CPU_BLACK;

	m_hud.Create();

	m_meshStriker.CreateBox(0.25f, 0.10f, 0.10f, CPU_GREEN);
	m_meshAIStriker.CreateBox(0.25f, 0.10f, 0.10f, CPU_WHITE);

	m_debris.Create();

	m_pTable = new Table;
	m_pTable->Create({ 0.f, 0.f, 0.f });
	m_pTable->SetClosingAmount(1.f);
	m_nailManager.Create(this);

	m_pPlayerStriker = new PlayerStriker;
	m_pPlayerStriker->Create({ 0.f, PUCK_Y, -5.f }, &m_meshStriker);
	m_pPlayerStriker->ResetRound({ 0.f, -0.5f, -5.f });
	m_pPlayerStriker->GetEntity()->visible = false;

	m_pAIStriker = new AIStriker;
	m_pAIStriker->Create({ 0.f, PUCK_Y, STRIKER_START_Z }, &m_meshAIStriker);
	m_aiBaseDifficulty = m_run.GetCurrentStage().aiDifficulty;
	m_pAIStriker->SetDifficulty(m_aiBaseDifficulty);
	m_pAIStriker->ResetRound({ 0.f, -0.5f, STRIKER_START_Z });
	m_pAIStriker->GetEntity()->visible = false;

	m_pPuck = new Puck;
	m_pPuck->Create({ 0.f, PUCK_DROP_HEIGHT, 0.f });
	m_pPuck->GetEntity()->visible = false;

	m_pEyes = new Eyes;
	m_pEyes->Create({ 0.f, 1.5f, 10.f });
	m_pEyes->SetScale(6.f);
	m_pEyes->SetLookTarget(m_pPuck->GetEntity());

	m_shopManager.Create(this);
	m_shopManager.SetDebris(&m_debris);

	m_camera.Create();

	m_pEyes->SetEyesColor(CPU_WHITE);
	m_pEyes->GetFSM()->ToState(5);
	BeginShop();

	m_boss.SetGameManager(this);

	MenuManager::Get()->SetCallbacks([this]() { StartShop(); }, [this]() { ContinueGame(); }, [this]() { RestartGame(); }, []() { cpuEngine.Quit(); });
	m_shopManager.SetShopItemsVisible(false);
	MenuManager::Get()->SetMenu(MenuManager::Menu::Start);

	cpuEngine.GetParticleData()->Create(2000000);
	cpuEngine.GetParticlePhysics()->gy = -0.5f;
}

void GameManager::Destroy()
{
	m_boss.End();

	m_debris.Destroy();

	if (m_pPuck)
		m_pPuck->Destroy();
	CPU_DELPTR(m_pPuck);

	if (m_pPlayerStriker)
		m_pPlayerStriker->Destroy();
	CPU_DELPTR(m_pPlayerStriker);

	if (m_pAIStriker)
		m_pAIStriker->Destroy();
	CPU_DELPTR(m_pAIStriker);

	m_nailManager.Destroy();

	if (m_pTable)
		m_pTable->Destroy();
	CPU_DELPTR(m_pTable);

	if (m_pEyes)
		m_pEyes->Destroy();
	CPU_DELPTR(m_pEyes);

	m_shopManager.Destroy();

	SetCursorVisible(true);
}

void GameManager::Update()
{
	const float dt = cpu::Clamp(cpuTime.delta, 0.f, 0.05f);

	m_camera.UpdateTransition(dt);

	MenuManager* menuManager = MenuManager::Get();

	if (menuManager->GetMenu() == MenuManager::Menu::Pause && m_pauseInputBlocked && cpuInput.vi.IsKeyReleased(VK_ESCAPE))
		m_pauseInputBlocked = false;	

	if (m_state == State::Paused && menuManager->GetMenu() == MenuManager::Menu::Pause && cpuInput.vi.IsKeyPressed(VK_ESCAPE) && m_pauseInputBlocked == false)
	{
		ContinueGame();
	}
	else if (menuManager->GetMenu() == MenuManager::Menu::None && m_state != State::Paused && cpuInput.vi.IsKeyPressed(VK_ESCAPE) && m_pauseInputBlocked == false)
	{
		PauseGame();
	}

	if (m_gameOverMenuPending)
	{
		m_gameOverMenuPending = false;
		m_state = State::GameOver;
		m_camera.MoveToMenu();
		SetCursorVisible(true);
		m_pEyes->SetEyesColor(CPU_WHITE);
		m_pEyes->GetFSM()->ToState(5);
		menuManager->SetMenu(MenuManager::Menu::GameOver);
	}

	if (m_state == State::GameOver && menuManager->GetMenu() == MenuManager::Menu::GameOver && m_camera.IsTransitioning() == false)
	{
		m_nailManager.Reset();
	}

	if (IsMenuVisible() || m_state == State::Paused)
		return;

	if (m_camera.IsTransitioning())
		return;

	m_shopManager.UpdateDiscardedItems(dt);
	m_debris.Update(dt);
	if (m_idleGameOver)
	{
		m_animation += dt;
		if (m_animation >= 2.2f)
		{
			m_suddenDeathGameOver = false;
			m_playerWonMatch = false;
			m_runWon = false;
			StartClosingTable();
			m_idleGameOver = false;
		}
	}
	else if (m_state == State::Playing)
	{
		if (UpdatePlaying(dt))
			return;
	}
	else if (m_state == State::Shop)
	{
		UpdateShop(dt);
	}
	else if (m_state == State::Countdown)
	{
		UpdateCountdown(dt);
	}	
	else
	{
		m_shopManager.UpdateGame(dt, false);
		m_pPlayerStriker->SetBackstopType(PlayerStriker::BackstopType::None);
		UpdateAnimation(dt);
	}


	if (m_state == State::Playing || m_state == State::Countdown)
	{
		if (m_state == State::Playing && m_resumeWithCountdown == false && m_goalPuckServe == false)
			m_pPlayerStriker->SetMovementEnabled(true);

		SetCursorVisible(false);
	}

	m_pAIStriker->SetSimulationActive(m_state == State::Playing && m_boss.IsRescueActive() == false && m_idleGameOver == false);

	if (IsMenuVisible() == false)
		m_camera.UpdateShake(dt);
}

bool GameManager::UpdatePlaying(float dt)
{
	if (m_run.IsBossStage() == false && m_versusClock.Update(dt))
	{
		if (m_playerScore == m_aiScore)
		{
			m_versusClock.EnterOvertime();
		}
		else
		{
			EndVersusOnTime();
			return true;
		}
	}

	m_shopManager.UpdateGame(dt, true);
	m_pPlayerStriker->Update();
	m_pPuck->ClearObstacles();

	const XMFLOAT2 puckPosition = m_pPuck->GetPosition();
	const XMFLOAT2 puckVelocity = m_pPuck->GetVelocity();
	const bool puckIdleInPlayerZone = puckPosition.y <= -1.5f && fabsf(puckVelocity.x) < 0.05f && fabsf(puckVelocity.y) < 0.05f;

	if (puckIdleInPlayerZone)
	{
		m_idleTimer += dt;
	}
	else
	{
		m_idleTimer = 0.f;
	}

	if (m_playerServePending && puckIdleInPlayerZone == false)
	{
		m_playerServePending = false;
	}

	if (m_idleTimer >= 7.f)
	{
		m_playerServePending = false;
		StartIdleGameOver();
		return true;
	}

	const bool guardianActive = m_shopManager.IsGuardianActive();
	const bool cagesActive = m_shopManager.AreCagesActive();

	const PlayerStriker::BackstopType backstopType = guardianActive ? PlayerStriker::BackstopType::Guardian : (cagesActive ? PlayerStriker::BackstopType::Cages : PlayerStriker::BackstopType::None);

	m_pPlayerStriker->SetBackstopType(backstopType);

	if (guardianActive)
	{
		m_pPuck->AddObstacle({ 0.f, -5.5f }, { 2.f, 0.1f }, 0.85f);
	}

	if (cagesActive)
	{
		m_pPuck->AddObstacle({ -1.5f, -5.5f }, { 0.7f, 0.24f }, 0.85f);
		m_pPuck->AddObstacle({ 1.5f, -5.5f }, { 0.7f, 0.24f }, 0.85f);
	}

	if (m_boss.IsActive())
	{
		m_boss.Update(dt, m_playerScore, *m_pPuck, *m_pPlayerStriker, *m_pAIStriker, *m_pEyes, m_seed);
		if (m_boss.ConsumePlayerHit())
		{
			StartPlayerStrikerGameOver(m_boss.ConsumeMeteoriteGameOver());
			return true;
		}
	}
	else
	{
		m_pAIStriker->Update(m_pPuck);
	}

	m_pPuck->SetSuddenDeathActive(m_shopManager.IsSuddenDeathActive());
	m_pPuck->SetTableFriction(m_shopManager.GetTableFriction());

	if (m_boss.IsRescueActive() == false)
		m_pPuck->Update(m_pPlayerStriker, m_pAIStriker);

	m_shopManager.UpdateLaser(dt, true);

	if (m_shopManager.ConsumeSuddenDeathHit())
	{
		m_shopManager.HideLaser();
		StartSuddenDeathGameOver();
		return true;
	}

	if (m_shopManager.IsBigStrikerActive())
	{
		m_pPlayerStriker->GetEntity()->transform.SetScaling({ 1.5f, 1.f, 1.5f });
	}

	CheckForGoal();

	return false;
}

void GameManager::UpdateShop(float dt)
{
	m_shopManager.UpdateShop(dt);

	if (cpuInput.vi.IsKeyPressed('F'))
		StartGame();
}

void GameManager::UpdateCountdown(float dt)
{
	m_shopManager.UpdateGame(dt, false);

	if (m_resumeWithCountdown == false && m_goalPuckServe == false)
		m_pPlayerStriker->Update();

	m_countdown -= dt;

	if (m_countdown <= 0.f)
	{
		if (m_resumeWithCountdown)
		{
			m_pPuck->SetVelocity(m_savedVelocity);
			m_pPlayerStriker->SetMovementEnabled(true);
			m_resumeWithCountdown = false;
		}
		else if (m_goalPuckServe == false)
		{
			if (m_run.IsBossStage() == false)
				m_versusClock.Start();

			PuckServe::LaunchRandom(*m_pPuck, cpu::Rand01(m_seed) < 0.5f ? -1.f : 1.f, m_seed);
			m_pPlayerStriker->SetMovementEnabled(true);
			m_pPlayerStriker->GetEntity()->transform.SetPosition(0.f, PUCK_Y, -5.f);
		}
		else
		{
			m_pPlayerStriker->SetMovementEnabled(true);
			m_pPlayerStriker->GetEntity()->transform.SetPosition(0.f, PUCK_Y, -5.f);
		}

		m_goalPuckServe = false;
		m_resumeWithCountdown = false;
		m_state = State::Playing;
		m_idleTimer = 0.f;
		MenuManager::Get()->SetMenu(MenuManager::Menu::None);
	}
}

void GameManager::UpdateAnimation(float dt)
{
	const float amount = dt / TRANSITION_DURATION;

	if (m_state == State::RaisingNails)
	{
		UpdateNails(dt);
	}
	else if (m_state == State::LoweringStrikers)
	{
		UpdateLoweringStrikers(amount);
	}
	else if (m_state == State::HammeringNail)
	{
		UpdateHammeringNail(dt);
	}
	else if (m_state == State::ClosingTable)
	{
		UpdateClosingTable(amount);
	}
	else if (m_state == State::OpeningShopTable)
	{
		UpdateOpeningShopTable();
	}
	else if (m_state == State::DroppingOffers)
	{
		UpdateDroppingOffers(dt);
	}
	else if (m_state == State::RaisingOffers)
	{
		UpdateRaisingOffers(dt);
	}
	else if (m_state == State::OpeningTable)
	{
		UpdateOpeningTable(amount);
	}
	else if (m_state == State::RaisingStrikers)
	{
		UpdateRaisingStrikers(dt, amount);
	}
	else if (m_state == State::DroppingPuck)
	{
		UpdateDroppingPuck(dt);
	}

}

void GameManager::UpdateLoweringStrikers(float amount)
{
	m_animation = cpu::Clamp(m_animation + amount, 0.f, 1.f);

	const float playerY = cpu::Lerp(m_goalPuckServe ? m_playerStrikerPositionBeforeGoal.y : PUCK_Y, -0.5f, m_animation);
	const float aiY = cpu::Lerp(m_goalPuckServe ? m_aiStrikerPositionBeforeGoal.y : PUCK_Y, -0.5f, m_animation);
	m_pPlayerStriker->GetEntity()->transform.pos.y = playerY;
	m_pAIStriker->GetEntity()->transform.pos.y = aiY;

	if (m_animation >= 1.f)
	{
		m_pPlayerStriker->GetEntity()->transform.pos.y = -0.5f;
		m_pAIStriker->GetEntity()->transform.pos.y = -0.5f;

		if (m_nailManager.IsHitPending())
		{
			m_state = State::HammeringNail;
			m_nailManager.StartHit();
		}
		else if (m_goalPuckServe)
		{
			m_pPlayerStriker->ResetRound({ m_playerStrikerPositionBeforeGoal.x, -0.5f, m_playerStrikerPositionBeforeGoal.z });
			m_pAIStriker->ResetRound({ m_aiStrikerPositionBeforeGoal.x, -0.5f, m_aiStrikerPositionBeforeGoal.z });
			m_pPlayerStriker->GetEntity()->visible = true;
			m_pAIStriker->GetEntity()->visible = true;
			m_state = State::RaisingStrikers;
		}
		else
		{
			m_state = State::ClosingTable;
		}

		m_animation = 0.f;
	}
}

void GameManager::UpdateHammeringNail(float dt)
{
	const bool nailDone = m_nailManager.UpdateHit(dt);

	if (nailDone)
	{
		if (m_closeAfterNail)
		{
			m_closeAfterNail = false;
			m_state = State::ClosingTable;
			m_animation = 0.f;
		}
		else if (m_goalPuckServe)
		{
			m_pPlayerStriker->ResetRound({ m_playerStrikerPositionBeforeGoal.x, -0.5f, m_playerStrikerPositionBeforeGoal.z });
			m_pAIStriker->ResetRound({ m_aiStrikerPositionBeforeGoal.x, -0.5f, m_aiStrikerPositionBeforeGoal.z });
			m_pPlayerStriker->GetEntity()->visible = true;
			m_pAIStriker->GetEntity()->visible = true;
			m_animation = 0.f;
			m_state = State::RaisingStrikers;
		}
	}
}

void GameManager::UpdateNails(float dt)
{
	if (m_state == State::RaisingNails)
	{
		if (m_nailManager.UpdateArrival(dt))
		{
			if (m_startWithCountdown)
			{
				m_state = State::OpeningTable;
				m_animation = 1.f;
			}
			else
			{
				m_animation = 0.f;
				m_state = State::RaisingStrikers;
			}
		}
	}

	m_nailManager.UpdateLayout();
}

void GameManager::UpdateClosingTable(float amount)
{
	m_animation = cpu::Clamp(m_animation + amount, 0.f, 1.f);		
	m_pTable->SetClosingAmount(m_animation * m_animation * m_animation);

	if (m_animation >= 1.f)
	{
		m_pPlayerStriker->GetEntity()->visible = false;
		m_pAIStriker->GetEntity()->visible = false;
		m_pPuck->GetEntity()->visible = false;

		m_gameStarted = false;
		OnMatchFinished();
	}
}

void GameManager::UpdateOpeningShopTable()
{
	if (m_animation >= 1.f)
	{
		m_pTable->SetClosingAmount(1.f);
		m_shopManager.SetShopItemsVisible(true);
		m_shopManager.StartOfferDrop();

		m_state = State::DroppingOffers;
	}
}

void GameManager::UpdateDroppingOffers(float dt)
{
	m_shopManager.UpdateItems(dt);

	m_pEyes->SetEyesColor(cpu::ToColor(52, 146, 235));

	if (m_pEyes->GetFSM()->state != 5)
	{
		m_pEyes->GetFSM()->ToState(5);
	}

	if (m_shopManager.AreOffersLanded())
	{
		m_shopManager.SetShopItemsVisible(true);

		m_state = State::Shop;

		SetCursorVisible(true);
	}
}

void GameManager::UpdateRaisingOffers(float dt)
{
	m_shopManager.UpdateItems(dt);

	if (m_shopManager.AreOffersGone())
	{
		m_gameStarted = true;
		m_state = State::OpeningTable;
		m_animation = 1.f;
		m_startWithCountdown = true;

		ResetGame();
		if (m_nailManager.Configure(m_run.IsBossStage()))
			m_state = State::RaisingNails;

		m_pPlayerStriker->ResetRound({ 0.f, -0.5f, -5.f });
		m_pPlayerStriker->GetEntity()->visible = false;

		m_pAIStriker->ResetRound({ 0.f, -0.5f, STRIKER_START_Z });
		m_pAIStriker->GetEntity()->visible = false;

		if (m_state == State::RaisingNails)
			m_pTable->SetClosingAmount(1.f);

		m_shopManager.SetGameItemsVisible(true);

		m_pPuck->SetVelocity({ 0.f, 0.f });
		m_pPuck->GetEntity()->visible = false;
	}
}

void GameManager::UpdateOpeningTable(float amount)
{
	m_animation = cpu::Clamp(m_animation - amount, 0.f, 1.f);

	m_pTable->SetClosingAmount(m_animation);

	if (m_animation <= 0.f)
	{
		m_pPlayerStriker->GetEntity()->visible = true;
		m_pPlayerStriker->SetMovementEnabled(false);

		m_pAIStriker->GetEntity()->visible = true;

		m_shopManager.SetGameItemsVisible(true);

		if (m_run.IsBossStage())
			m_boss.Begin();

		m_pEyes->GetFSM()->ToState(2);
		m_pEyes->SetEyesColor(m_run.GetCurrentStage().type == StageType::Versus ? cpu::ToColor(250, 168, 45) : cpu::ToColor(212, 13, 13));

		m_state = State::RaisingStrikers;
		m_animation = 0.f;
	}
}



void GameManager::UpdateRaisingStrikers(float dt, float amount)
{
	m_animation = cpu::Clamp(m_animation + amount, 0.f, 1.f);

	const XMFLOAT3 playerPosition = { 0.f, PUCK_Y, -5.f };
	const XMFLOAT3 aiPosition = { 0.f, PUCK_Y, STRIKER_START_Z };
	const float y = cpu::Lerp(-0.5f, PUCK_Y, m_animation);

	const float puckZ = m_pPuck->GetEntity()->transform.pos.z;

	if (m_goalPuckServe)
	{
		float puckY = PuckServe::GetNextDropY(m_pPuck->GetEntity()->transform.pos.y, dt);
		if (puckY <= PUCK_Y)
		{
			if (m_pPuck->GetEntity()->transform.pos.y > PUCK_Y)
			{
				m_pPuck->SetVelocity({ 0.f, 0.f });
				ShakeCamera(0.2f);
			}
			puckY = PUCK_Y;
		}
		m_pPuck->GetEntity()->transform.SetPosition(0.f, puckY, puckZ);
	}

	m_pPlayerStriker->GetEntity()->transform.pos.x = cpu::Lerp(m_goalPuckServe ? m_playerStrikerPositionBeforeGoal.x : playerPosition.x, playerPosition.x, m_animation);
	m_pPlayerStriker->GetEntity()->transform.pos.y = y;
	m_pPlayerStriker->GetEntity()->transform.pos.z = cpu::Lerp(m_goalPuckServe ? m_playerStrikerPositionBeforeGoal.z : playerPosition.z, playerPosition.z, m_animation);

	m_pAIStriker->GetEntity()->transform.pos.x = cpu::Lerp(m_goalPuckServe ? m_aiStrikerPositionBeforeGoal.x : aiPosition.x, aiPosition.x, m_animation);
	m_pAIStriker->GetEntity()->transform.pos.y = y;		
	m_pAIStriker->GetEntity()->transform.pos.z = cpu::Lerp(m_goalPuckServe ? m_aiStrikerPositionBeforeGoal.z : aiPosition.z, aiPosition.z, m_animation);

	if (m_animation >= 1.f)
	{
		m_pPlayerStriker->ResetRound(playerPosition);
		m_pAIStriker->ResetRound(aiPosition);

		if (m_startWithCountdown)
		{
			m_startWithCountdown = false;
			DropPuck(0.f);
		}
		else if (m_goalPuckServe)
		{
			m_countdown = COUNTDOWN_DURATION;
			m_playerServePending = puckZ == PLAYER_SERVE_Z;
			m_idleTimer = 0.f;
			m_state = State::Countdown;
		}
	}
}

void GameManager::UpdateDroppingPuck(float dt)
{
	m_pPlayerStriker->Update();

	const float z = m_pPuck->GetEntity()->transform.pos.z;
	float y = PuckServe::GetNextDropY(m_pPuck->GetEntity()->transform.pos.y, dt);

	if (y <= PUCK_Y)
	{
		y = PUCK_Y;
		m_pPuck->SetVelocity({ 0.f, 0.f });
		ShakeCamera(0.2f);
		m_countdown = COUNTDOWN_DURATION;
		m_goalPuckServe = false;
		m_pPuck->GetEntity()->visible = true;
		m_state = State::Countdown;
	}
	m_pPuck->GetEntity()->transform.SetPosition(0.f, y, z);
}

void GameManager::StartShop()
{
	m_camera.MoveToGame();
	MenuManager::Get()->SetMenu(MenuManager::Menu::None);
	m_pTable->SetClosingAmount(1.f);
	m_shopManager.SetShopItemsVisible(true);
	m_state = State::OpeningShopTable;
	m_animation = 1.f;
	SetCursorVisible(true);
}

void GameManager::StartGame()
{
	if (m_shopManager.AreOffersLanded() == false || m_shopManager.IsInventorySettled() == false)
		return;

	m_camera.MoveToGame();

	m_shopManager.StartOfferRise();

	m_state = State::RaisingOffers;
	MenuManager::Get()->SetMenu(MenuManager::Menu::None);
	SetCursorVisible(false);
}

void GameManager::PauseGame()
{
	if (m_state == State::Paused)
		return;

	if (m_state == State::Playing)
		m_savedVelocity = m_pPuck->GetVelocity();
	m_resumeWithCountdown = m_state == State::Playing;

	m_pause.state = m_state;
	m_pause.playerMovement = m_state == State::Playing || m_state == State::Countdown || m_state == State::DroppingPuck;
	m_pause.cursorVisible = m_cursorVisible;
	m_pause.cameraPosition = m_camera.GetTargetPosition();

	m_camera.MoveToMenu();

	m_pauseInputBlocked = true;
	m_state = State::Paused;

	m_pPlayerStriker->SetMovementEnabled(false);
	m_pAIStriker->SetSimulationActive(false);

	SetCursorVisible(true);

	m_pEyes->SetEyesColor(CPU_WHITE);

	if (m_pause.state == State::Playing || m_pause.state == State::Countdown || m_pause.state == State::DroppingPuck)
	{
		m_pEyes->GetFSM()->ToState(5);
	}

	MenuManager::Get()->SetMenu(MenuManager::Menu::Pause);
}

void GameManager::ContinueGame()
{
	if (m_state != State::Paused)
		return;

	m_state = m_pause.state;
	if (m_resumeWithCountdown)
	{
		m_pPuck->SetVelocity({ 0.f, 0.f });
		m_countdown = COUNTDOWN_DURATION;
		m_pPlayerStriker->SetMovementEnabled(false);
		m_state = State::Countdown;
	}
	else if (m_state == State::DroppingPuck)
	{
		m_pPuck->GetEntity()->visible = true;
	}

	m_camera.MoveTo(m_pause.cameraPosition);
	MenuManager::Get()->SetMenu(m_state == State::GameOver ? MenuManager::Menu::GameOver : MenuManager::Menu::None);
	m_pauseInputBlocked = false;
	m_pPlayerStriker->SetMovementEnabled(m_resumeWithCountdown ? false : m_pause.playerMovement);
	m_pAIStriker->SetSimulationActive(m_state == State::Playing && m_boss.IsRescueActive() == false && m_idleGameOver == false);
	SetCursorVisible(m_pause.cursorVisible);

	m_pEyes->SetEyesColor(m_pEyes->GetPreviousEyesColor());
}

void GameManager::RestartGame()
{
	if (m_gameStarted)
		return;

	m_camera.MoveToGame();

	MenuManager::Get()->SetMenu(MenuManager::Menu::None);

	m_run.Reset();
	m_runWon = false;
	ApplyCurrentStage();

	m_pEyes->SetEyesColor(cpu::ToColor(52, 146, 235));

	m_money = 30;

	m_shopManager.ClearInventory();
	BeginShop();
}

void GameManager::ApplyCurrentStage()
{
	m_aiBaseDifficulty = m_run.GetCurrentStage().aiDifficulty;
	m_pAIStriker->SetDifficulty(m_aiBaseDifficulty);
	m_shopManager.SetAIBaseDifficulty(m_aiBaseDifficulty);
}

void GameManager::OnMatchFinished()
{
	m_boss.End();

	if (m_suddenDeathGameOver)
	{
		m_runWon = true;
		m_gameOverMenuPending = true;
		return;
	}

	if (m_meteoriteGameOver)
	{
		m_runWon = false;
		m_gameOverMenuPending = true;
		return;
	}

	if (m_playerWonMatch == false)
	{
		m_runWon = false;
		m_gameOverMenuPending = true;
		return;
	}

	if (m_run.IsLastStage())
	{
		m_runWon = true;
		m_gameOverMenuPending = true;
		return;
	}

	m_run.Advance();
	ApplyCurrentStage();
	BeginShop();
}

bool GameManager::IsMenuVisible() const
{
	return MenuManager::Get()->GetMenu() != MenuManager::Menu::None;
}

void GameManager::CheckForGoal()
{
	const float z = m_pPuck->GetPosition().y;

	if (z >= GOAL_LINE - 0.3f)
	{
		m_shopManager.HideLaser();
		AddPoint(true);
	}
	else if (z <= -GOAL_LINE)
	{
		m_shopManager.HideLaser();
		AddPoint(false);
	}
}

void GameManager::AddPoint(bool playerScored)
{
	m_shopManager.ClearRoundCoins();
	m_playerStrikerPositionBeforeGoal = m_pPlayerStriker->GetEntity()->transform.pos;
	m_aiStrikerPositionBeforeGoal = m_pAIStriker->GetEntity()->transform.pos;
	m_goalPuckServe = true;

	if (playerScored)
	{
		++m_playerScore;
		m_boss.OnGoal();
		m_pEyes->GetFSM()->ToState(4);
		ShakeCamera(1.7f);
	}
	else
	{
		++m_aiScore;
		m_boss.OnGoal();
		m_pEyes->GetFSM()->ToState(3);
	}

	m_pPuck->SetVelocity({ 0.f, 0.f });

	const bool nailPending = m_nailManager.QueueHit(playerScored ? NailManager::AI_SIDE : NailManager::PLAYER_SIDE);

	const bool overtimeDecided = m_versusClock.IsOvertime() && m_playerScore != m_aiScore;
	const int scoreToWin = m_boss.GetScoreToWin(SCORE_TO_WIN);
	const int aiScoreToWin = SCORE_TO_WIN;

	if (overtimeDecided || m_playerScore >= scoreToWin || m_aiScore >= aiScoreToWin)
	{
		m_suddenDeathGameOver = false;
		m_playerWonMatch = overtimeDecided ? m_playerScore > m_aiScore : m_aiScore < aiScoreToWin;

		if (m_playerWonMatch == false && m_aiScore >= aiScoreToWin && m_shopManager.IsExtraLifeOwned())
		{
			m_playerWonMatch = true;
			m_shopManager.ConsumeExtraLife();
		}

		if (m_playerWonMatch)
			AwardVersusReward();

		m_closeAfterNail = nailPending;
		if (nailPending)
		{
			m_pPlayerStriker->SetMovementEnabled(false);
			m_pAIStriker->SetSimulationActive(false);
			m_pPlayerStriker->GetEntity()->visible = true;
			m_pAIStriker->GetEntity()->visible = true;
			m_pPuck->GetEntity()->visible = false;
			m_goalPuckServe = false;
			m_state = State::LoweringStrikers;
			m_animation = 0.f;
		}
		else
		{
			StartClosingTable();
		}
	}
	else
	{
		m_closeAfterNail = false;
		m_pPlayerStriker->SetMovementEnabled(false);
		m_pAIStriker->SetSimulationActive(false);
		m_pAIStriker->GetEntity()->visible = true;
		m_pPuck->GetEntity()->visible = true;
		m_pPuck->GetEntity()->transform.SetPosition(0.f, PUCK_DROP_HEIGHT, playerScored ? AI_SERVE_Z : PLAYER_SERVE_Z);
		m_playerServePending = playerScored == false;
		m_idleTimer = 0.f;
		m_state = State::LoweringStrikers;
		m_animation = 0.f;
	}
}

void GameManager::StartClosingTable()
{
	m_versusClock.Stop();
	m_shopManager.HideLaser();
	m_goalPuckServe = false;
	m_state = State::LoweringStrikers;
	m_animation = 0.f;
	m_pPlayerStriker->SetMovementEnabled(false);
	m_pPlayerStriker->GetEntity()->visible = !m_idleGameOver;
	m_pAIStriker->SetSimulationActive(false);
	m_pAIStriker->GetEntity()->visible = !m_suddenDeathGameOver;
	m_pPuck->GetEntity()->visible = false;
}

void GameManager::DropPuck(float z)
{
	m_goalPuckServe = false;
	m_pPuck->GetEntity()->visible = true;
	m_pPuck->GetEntity()->transform.SetPosition(0.f, PUCK_DROP_HEIGHT, z);
	m_state = State::DroppingPuck;
}

void GameManager::SetCursorVisible(bool visible)
{
	if (m_cursorVisible == visible)
		return;

	ShowCursor(visible ? TRUE : FALSE);
	m_cursorVisible = visible;
}

void GameManager::ResetGame()
{
	m_playerScore = 0;
	m_aiScore = 0;
	m_versusClock.Reset();
}

void GameManager::EndVersusOnTime()
{
	m_playerWonMatch = m_playerScore > m_aiScore;
	m_suddenDeathGameOver = false;
	m_closeAfterNail = false;

	if (m_playerWonMatch)
		AwardVersusReward();

	m_shopManager.HideLaser();
	m_pPuck->SetVelocity({ 0.f, 0.f });
	StartClosingTable();
}

void GameManager::AwardVersusReward()
{
	if (m_run.GetCurrentStage().type != StageType::Versus)
		return;

	m_money += VersusRules::ComputeReward(m_versusClock.GetElapsed());
}

bool GameManager::TrySpendMoney(int amount)
{
	if (amount < 0 || m_money < amount)
		return false;

	m_money -= amount;
	return true;
}

void GameManager::StartIdleGameOver()
{
	m_idleGameOver = true;
	m_playerWonMatch = false;
	m_runWon = false;

	m_idleTimer = 0.f;
	m_animation = 0.f;

	m_pPuck->SetVelocity({ 0.f, 0.f });

	m_pPlayerStriker->SetMovementEnabled(false);
	m_pAIStriker->SetSimulationActive(false);

	m_pEyes->GetFSM()->ToState(4);

	ShakeCamera(2.2f);

	SpawnStrikerDebris(m_pPlayerStriker->GetEntity()->transform.pos, CPU_GREEN);

	m_pPlayerStriker->GetEntity()->visible = false;
}

void GameManager::StartPlayerStrikerGameOver(bool meteoriteHit)
{
	m_playerWonMatch = false;
	m_suddenDeathGameOver = false;

	if (meteoriteHit)
		m_meteoriteGameOver = true;

	m_pPuck->SetVelocity({ 0.f, 0.f });

	SpawnStrikerDebris(m_pPlayerStriker->GetEntity()->transform.pos, CPU_GREEN);

	StartClosingTable();

	m_pPlayerStriker->GetEntity()->visible = false;
}

void GameManager::StartSuddenDeathGameOver()
{
	SpawnStrikerDebris(m_pAIStriker->GetEntity()->transform.pos, m_pAIStriker->GetEntity()->pMaterial->color);
	m_pAIStriker->GetEntity()->visible = false;

	m_suddenDeathGameOver = true;

	m_pPuck->SetVelocity({ 0.f, 0.f });
	m_pPuck->SetSuddenDeathActive(false);

	m_pPlayerStriker->GetEntity()->transform.pos.y = PUCK_Y;

	m_pAIStriker->GetEntity()->transform.pos.y = PUCK_Y;

	m_pEyes->SetStatusColor(false, CPU_WHITE);
	m_pEyes->SetEyesColor(CPU_WHITE);

	StartClosingTable();
}

void GameManager::BeginShop()
{
	m_idleGameOver = false;
	m_idleTimer = 0.f;
	m_playerServePending = false;
	m_boss.End();
	m_gameStarted = false;
	m_suddenDeathGameOver = false;
	m_meteoriteGameOver = false;
	m_gameOverMenuPending = false;

	m_playerScore = 0;
	m_aiScore = 0;

	MenuManager::Get()->SetMenu(MenuManager::Menu::None);

	m_shopManager.BeginShop();

	m_pTable->SetClosingAmount(1.f);
	m_state = State::OpeningShopTable;
	m_animation = 1.f;
	SetCursorVisible(true);
}

void GameManager::OnRender(int pass)
{
	if (pass != CPU_PASS_UI_END)
		return;

	GameHud::Data data;
	data.state = m_state;
	data.countdown = m_countdown;
	data.aiBaseDifficulty = m_aiBaseDifficulty;
	data.runWon = m_runWon;
	data.suddenDeathGameOver = m_suddenDeathGameOver;
	data.meteoriteGameOver = m_meteoriteGameOver;
	data.money = m_money;
	data.versusTimeLeft = m_versusClock.GetRemaining();
	data.versusOvertime = m_versusClock.IsOvertime();
	data.showVersusTimer = m_run.IsBossStage() == false;

	m_hud.Render(data, m_run, m_shopManager);
}

void GameManager::ShakeCamera(float duration)
{
	m_camera.Shake(duration);
}

void GameManager::SpawnStrikerDebris(const XMFLOAT3& position, const XMFLOAT3& color)
{
	m_debris.Spawn(position, m_seed, color, 36, 0.035f, 1.1f, 2.f, 0.8f, 2.5f, { 5.f, 4.f, 3.f });
}