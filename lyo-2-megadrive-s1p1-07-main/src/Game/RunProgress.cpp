#include "pch.h"
#include "RunProgress.h"

std::string RunProgress::GetStageLabel() const
{
	if (IsBossStage())
		return "BOSS";

	int versusCount = 0;

	for (const StageDefinition& stage : STAGES)
	{
		if (stage.type == StageType::Versus)
			++versusCount;
	}

	return "COMBAT " + std::to_string(m_stageIndex + 1) + "/" + std::to_string(versusCount);
}
