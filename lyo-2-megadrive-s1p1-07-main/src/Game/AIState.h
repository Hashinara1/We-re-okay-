#pragma once
class AIStriker;

struct StateAIDefend
{
	void OnEnter(AIStriker& ai, int from, void* pParam);
	void OnExecute(AIStriker& ai);
	void OnExit(AIStriker& ai, int to) {}
};

struct StateAIAttack
{
	void OnEnter(AIStriker& ai, int from, void* pParam);
	void OnExecute(AIStriker& ai);
	void OnExit(AIStriker& ai, int to) {}
};

struct StateAIConfused
{
	void OnEnter(AIStriker& ai, int from, void* pParam);
	void OnExecute(AIStriker& ai);
	void OnExit(AIStriker& ai, int to) {}
};

struct StateAIFrozen
{
	void OnEnter(AIStriker& ai, int from, void* pParam);
	void OnExecute(AIStriker& ai);
	void OnExit(AIStriker& ai, int to) {}
};
