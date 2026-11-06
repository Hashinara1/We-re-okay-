#pragma once

enum class GameState
{
	Playing,
	LoweringStrikers,
	HammeringNail,
	RaisingNails,
	RaisingStrikers,
	ClosingTable,
	GameOver,
	Shop,
	OpeningTable,
	Countdown,
	DroppingPuck,
	OpeningShopTable,
	DroppingOffers,
	RaisingOffers,
	Paused
};

struct PauseSnapshot
{
	GameState state = GameState::Shop;
	XMFLOAT3 cameraPosition = { 0.f, 0.f, 0.f };
	bool playerMovement = false;
	bool cursorVisible = true;
};
