#pragma once

#include <functional>
#include "Button3D.h"

class MenuManager
{
public:
	enum class Menu
	{
		None,
		Start,
		Pause,
		GameOver
	};

	static MenuManager* Get() { return s_pMenuManager; }

	MenuManager();

	void Create();
	void Update(float dt);
	void Destroy();
	void SetMenu(Menu menu);
	Menu GetMenu() const { return m_menu; }

	void SetCallbacks(std::function<void()> onStart, std::function<void()> onContinue, std::function<void()> onRestart, std::function<void()> onQuit);

private:

	inline static MenuManager* s_pMenuManager = nullptr;

	Button3D m_startButton;
	Button3D m_continueButton;
	Button3D m_restartButton;
	Button3D m_quitButton;

	std::function<void()> m_onStart;
	std::function<void()> m_onContinue;
	std::function<void()> m_onRestart;
	std::function<void()> m_onQuit;

	Menu m_menu = Menu::None;
};
