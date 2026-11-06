#include "pch.h"
#include "BonusCatalog.h"

namespace
{
	const BonusInfo BONUS_INFO[(int)BonusId::Count] =
	{
		{ "Guardian", "Bloque temporairement votre cage\nlors d'un tir adverse.", 15, true, cpu::ToColor(65, 170, 230) },
		{ "Strength", "Renvoie le puck plus fort\npendant 8 secondes.", 10, true, cpu::ToColor(235, 85, 55) },
		{ "Freeze", "Ralentit fortement l'IA\npendant 3 secondes.", 15, true, cpu::ToColor(100, 220, 240) },
		{ "Extra Life", "Evite la defaite du match.", 25, false, cpu::ToColor(230, 90, 130) },
		{ "Bonus de cages", "Retrecit votre cage", 20, false, cpu::ToColor(110, 190, 85) },
		{ "Gros striker", "Agrandit votre striker\npendant 8 secondes.", 10, true, cpu::ToColor(180, 105, 220) },
		{ "Confusion", "Desoriente l'IA pendant\n 3 secondes.", 15, true, cpu::ToColor(230, 135, 205) },
		{ "Argent a gogo", "Fait tomber des pieces\npendant la partie.", 25, false, cpu::ToColor(238, 190, 55) },
		{ "Laser", "Indique la direction\ndu puck.", 5, false, cpu::ToColor(235, 55, 55) },
		{ "Ralentissement", "Ralentit le puck dans\nvotre camp.", 15, false, cpu::ToColor(100, 170, 235) },
		{ "Sudden Death", "Detruit le striker adverse\nau prochain contact.\nduré : 1/2 secondes", 0, true, cpu::ToColor(245, 48, 32) }
	};
}

const FusionRecipe FUSION_RECIPES[] =
{
	{ BonusId::Force, BonusId::Force, BonusId::SuddenDeath },
};

const BonusInfo& BonusCatalog::Get(BonusId id)
{
	return BONUS_INFO[(int)id];
}

const FusionRecipe* BonusCatalog::FindRecipe(BonusId a, BonusId b)
{
	for (const FusionRecipe& recipe : FUSION_RECIPES)
	{
		if ((recipe.ingredientA == a && recipe.ingredientB == b) ||
			(recipe.ingredientA == b && recipe.ingredientB == a))
		{
			return &recipe;
		}
	}
	return nullptr;
}
