#include "pch.h"
#include "GameHud.h"
#include "MenuManager.h"

void GameHud::Create()
{
	m_font.Create(cpuDevice.GetHeight() <= 512 ? 14 : 28, CPU_WHITE, "Dracutaz");

	m_textureMoney.Load("../../res/Coin/coin.png");
	m_pSpriteMoney = cpuEngine.CreateSprite();
	m_pSpriteMoney->pTexture = &m_textureMoney;
	m_pSpriteMoney->CenterAnchor();
	m_pSpriteMoney->x = cpuDevice.GetWidth() - 30;
	m_pSpriteMoney->y = 30;
	m_pSpriteMoney->z = 0;

	m_pSpriteMoney->visible = false;
}

void GameHud::Render(const Data& data, RunProgress& run, ShopManager& shopManager)
{
	MenuManager* menuManager = MenuManager::Get();

	//const std::string fps = CPU_STR(cpuTime.fps) + " fps" + "\nai diff : " + CPU_STR(data.aiBaseDifficulty);
	//cpuDevice.DrawText(&m_font, fps.c_str(), 0, 2, CPU_TEXT_LEFT);

	if (menuManager->GetMenu() == MenuManager::Menu::Start)
		return;

	const std::string money = CPU_STR(data.money) + " PIECES";
	cpuDevice.DrawText(&m_font, money.c_str(), cpuDevice.GetWidth() - 10, 2, CPU_TEXT_RIGHT);

	if (data.showVersusTimer && (data.state == GameState::Playing || data.state == GameState::Paused || data.state == GameState::Countdown))
	{
		const std::string timer = data.versusOvertime ? "PROLONGATION" : CPU_STR((int)ceilf(data.versusTimeLeft)) + " s";
		cpuDevice.DrawText(&m_font, timer.c_str(), cpuDevice.GetWidth() - 10, 2 + m_font.height, CPU_TEXT_RIGHT);
	}

	if (data.state == GameState::Shop || data.state == GameState::OpeningShopTable || data.state == GameState::DroppingOffers)
	{
		shopManager.Render(data.state == GameState::Shop);
		const std::string shopTitle = "BOUTIQUE (prochain : " + run.GetStageLabel() + ")\nF : commencer la partie";
		cpuDevice.DrawText(&m_font, shopTitle.c_str(), cpuDevice.GetWidth() / 2, 10, CPU_TEXT_CENTER);
		cpuDevice.DrawText(&m_font, "1 / 2 : utiliser un bonus actif - Glisser hors table : jeter", cpuDevice.GetWidth() / 2, cpuDevice.GetHeight() - 28, CPU_TEXT_CENTER);
	}
	else if (data.state == GameState::GameOver)
	{
		const std::string endTitle = data.runWon ? "VICTOIRE" : "GAME OVER";
		const std::string endDetail = data.runWon ? (data.suddenDeathGameOver ? "Striker adverse detruit" : "Boss vaincu") : (data.meteoriteGameOver ? "Striker detruit par une meteorite" : "Defaite - " + run.GetStageLabel());
		cpuDevice.DrawText(&m_font, endTitle.c_str(), cpuDevice.GetWidth() / 2, 10, CPU_TEXT_CENTER);
		cpuDevice.DrawText(&m_font, endDetail.c_str(), cpuDevice.GetWidth() / 2, 35, CPU_TEXT_CENTER);
	}
	else if (data.state == GameState::Paused && menuManager->GetMenu() == MenuManager::Menu::Pause)
	{
		cpuDevice.DrawText(&m_font, "PAUSE", cpuDevice.GetWidth() / 2, 10, CPU_TEXT_CENTER);
	}

	if (data.state == GameState::Countdown)
	{
		const std::string countdown = CPU_STR((int)ceilf(data.countdown));
		cpuDevice.DrawText(&m_font, countdown.c_str(), cpuDevice.GetWidth() / 2, 20, CPU_TEXT_CENTER);
	}
}
