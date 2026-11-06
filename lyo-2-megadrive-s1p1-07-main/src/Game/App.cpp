#include "pch.h"
#include "App.h"

#include "GameManager.h"
#include "MenuManager.h"

App::App()
{
	s_pApp = this;
	CPU_CALLBACK_START(OnStart);
	CPU_CALLBACK_UPDATE(OnUpdate);
	CPU_CALLBACK_EXIT(OnExit);
	CPU_CALLBACK_RENDER(OnRender);

	m_pMenuManager = new MenuManager;

	AddFontResourceExW(L"../../res/Game/font.otf", FR_PRIVATE, 0);
}

App::~App()
{
	RemoveFontResourceExW(L"../../res/Game/font.otf", FR_PRIVATE, 0);
}

void App::OnStart()
{
	//cpuEngine.EnableBoxRender();

	m_pMenuManager->Create();

	m_pGameManager = new GameManager;
	m_pGameManager->Create();
}

void App::OnUpdate()
{
	m_pMenuManager->Update(cpuTime.delta);

	m_pGameManager->Update();
}

void App::OnExit()
{
	m_pMenuManager->Destroy();
	CPU_DELPTR(m_pMenuManager);

	if (m_pGameManager)
		m_pGameManager->Destroy();
	CPU_DELPTR(m_pGameManager);
}

void App::OnRender(int pass)
{
	m_pGameManager->OnRender(pass);
}
