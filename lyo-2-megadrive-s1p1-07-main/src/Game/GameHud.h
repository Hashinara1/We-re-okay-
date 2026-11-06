#pragma once

#include "ShopManager.h"
#include "RunProgress.h"
#include "GameState.h"

class GameHud
{
public:
	struct Data
	{
		GameState state = GameState::Shop;
		float countdown = 0.f;
		float aiBaseDifficulty = 0.5f;
		bool runWon = false;
		bool suddenDeathGameOver = false;
		bool meteoriteGameOver = false;
		int money = 0;
		float versusTimeLeft = 0.f;
		bool versusOvertime = false;
		bool showVersusTimer = false;
	};

	void Create();
	void Render(const Data& data, RunProgress& run, ShopManager& shopManager);

private:
	cpu_font m_font;
	
	cpu_texture m_textureMoney;
	cpu_sprite* m_pSpriteMoney = nullptr;
};
