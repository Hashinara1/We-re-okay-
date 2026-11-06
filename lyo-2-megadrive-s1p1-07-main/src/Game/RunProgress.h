#pragma once

#include <string>

enum class StageType
{
	Versus,
	Boss
};

struct StageDefinition
{
	StageType type;
	float aiDifficulty;
};

class RunProgress
{
public:
	void Reset() { m_stageIndex = 0; }

	void Advance()
	{
		if (IsLastStage() == false)
			++m_stageIndex;
	}

	const StageDefinition& GetCurrentStage() const { return STAGES[m_stageIndex]; }
	int GetStageIndex() const { return m_stageIndex; }
	bool IsLastStage() const { return m_stageIndex >= STAGE_COUNT - 1; }
	bool IsBossStage() const { return GetCurrentStage().type == StageType::Boss; }

	std::string GetStageLabel() const;

private:
	static constexpr int STAGE_COUNT = 5;

	static constexpr StageDefinition STAGES[STAGE_COUNT] =
	{
		{ StageType::Versus, 0.6f },
		{ StageType::Versus, 0.7f },
		{ StageType::Versus, 0.8f },
		{ StageType::Versus, 0.9f },
		{ StageType::Boss,   1.f }

		//{ StageType::Boss, 0.2f }
	};

	int m_stageIndex = 0;
};
