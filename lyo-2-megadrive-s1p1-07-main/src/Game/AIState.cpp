#include "pch.h"
#include "AIState.h"
#include "AIStriker.h"

void StateAIDefend::OnEnter(AIStriker& ai, int from, void* pParam)
{
	ai.m_behaviorTime = 0.f;
}

void StateAIDefend::OnExecute(AIStriker& ai)
{
	if (ai.m_simulationActive && ai.m_currentPuck)
		ai.UpdateDefend(*ai.m_currentPuck);
}


void StateAIAttack::OnEnter(AIStriker& ai, int from, void* pParam)
{
	ai.m_behaviorTime = 0.f; 
	ai.m_aimTimer = 0.f;
}

void StateAIAttack::OnExecute(AIStriker& ai)
{
	if (ai.m_simulationActive && ai.m_currentPuck)
		ai.UpdateAttack(*ai.m_currentPuck);
}


void StateAIConfused::OnEnter(AIStriker& ai, int from, void* pParam)
{
	ai.m_behaviorTime = 0.f; 
	ai.ChooseConfusedTarget();
}

void StateAIConfused::OnExecute(AIStriker& ai)
{
	if (ai.m_simulationActive) 
		ai.UpdateConfused(cpuTime.delta);
}


void StateAIFrozen::OnEnter(AIStriker& ai, int from, void* pParam)
{
	ai.m_behaviorTime = 0.f;
}

void StateAIFrozen::OnExecute(AIStriker& ai)
{
	if (ai.m_simulationActive && ai.m_currentPuck) 
		ai.UpdateFrozen(*ai.m_currentPuck, cpuTime.delta);
}