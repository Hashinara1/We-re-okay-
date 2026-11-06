#pragma once

class GameManager;
class MenuManager;

class App
{
public:
	App();
	virtual ~App();

	static App& GetInstance() { return *s_pApp; }

	void OnStart();
	void OnUpdate();
	void OnExit();
	void OnRender(int pass);

private:
	inline static App* s_pApp = nullptr;
	GameManager* m_pGameManager = nullptr;
	MenuManager* m_pMenuManager = nullptr;
};

