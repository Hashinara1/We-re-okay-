#pragma once

#include "pch.h"

enum class BonusId
{
	Guardian,
	Force,
	Freeze,
	ExtraLife,
	Cages,
	BigStriker,
	Confusion,
	GoldShower,
	Laser,
	PuckSlowdown,
	SuddenDeath,
	Count
};

struct BonusInfo
{
	const char* name;
	const char* description;
	int price;
	bool active;
	XMFLOAT3 color;
};

struct FusionRecipe
{
	BonusId ingredientA;
	BonusId ingredientB;
	BonusId result;
};

class BonusCatalog
{
public:
	static const BonusInfo& Get(BonusId id);
	static const FusionRecipe* FindRecipe(BonusId a, BonusId b);
};
