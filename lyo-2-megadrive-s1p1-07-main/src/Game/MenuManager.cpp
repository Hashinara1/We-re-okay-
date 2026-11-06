#include "pch.h"
#include "MenuManager.h"

MenuManager::MenuManager()
{
	s_pMenuManager = this;
}

void MenuManager::Create()
{
	const float menuZ = 8.f;
	const float buttonX = -3.f;
	const float buttonWidth = 2.f;
	const float buttonHeight = 0.6f;
	const float textSize = 0.22f;
	const XMFLOAT3 buttonColor = cpu::ToColor(179, 163, 34);
	const XMFLOAT3 buttonEdgeColor = cpu::ToColor(92, 85, 24);
	const XMFLOAT3 quitColor = cpu::ToColor(120, 20, 7);
	const XMFLOAT3 quitEdgeColor = cpu::ToColor(75, 15, 7);

	m_startButton.Create("START", { buttonX, 2.f, menuZ }, buttonWidth, buttonHeight, textSize, buttonColor, CPU_WHITE, buttonEdgeColor);
	m_continueButton.Create("CONTINUE", { buttonX, 2.f, menuZ }, buttonWidth, buttonHeight, textSize, buttonColor, CPU_WHITE, buttonEdgeColor);
	m_restartButton.Create("RESTART", { buttonX, 2.f, menuZ }, buttonWidth, buttonHeight, textSize, buttonColor, CPU_WHITE, buttonEdgeColor);
	m_quitButton.Create("QUIT", { buttonX, 1.f, menuZ }, buttonWidth, buttonHeight, textSize, quitColor, CPU_WHITE, quitEdgeColor);

	m_startButton.SetOnLeftClick([this]() { if (m_onStart) m_onStart(); });
	m_continueButton.SetOnLeftClick([this]() { if (m_onContinue) m_onContinue(); });
	m_restartButton.SetOnLeftClick([this]() { if (m_onRestart) m_onRestart(); });
	m_quitButton.SetOnLeftClick([this]() { if (m_onQuit) m_onQuit(); });

	SetMenu(Menu::None);
}

void MenuManager::SetCallbacks(std::function<void()> onStart, std::function<void()> onContinue, std::function<void()> onRestart, std::function<void()> onQuit)
{
	m_onStart = std::move(onStart);
	m_onContinue = std::move(onContinue);
	m_onRestart = std::move(onRestart);
	m_onQuit = std::move(onQuit);
}

void MenuManager::SetMenu(Menu menu)
{
	m_menu = menu;
	m_startButton.SetVisible(menu == Menu::Start);
	m_continueButton.SetVisible(menu == Menu::Pause);
	m_restartButton.SetVisible(menu == Menu::GameOver);
	m_quitButton.SetVisible(menu != Menu::None);
}

void MenuManager::Update(float dt)
{
	m_startButton.Update(dt);
	m_continueButton.Update(dt);
	m_restartButton.Update(dt);
	m_quitButton.Update(dt);
}

void MenuManager::Destroy()
{
	m_startButton.Destroy();
	m_continueButton.Destroy();
	m_restartButton.Destroy();
	m_quitButton.Destroy();
	m_menu = Menu::None;
}
