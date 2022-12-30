#pragma once
#include "AgentBase.h"

class Grid;

class AgentBasePooler
{
public:
	AgentBasePooler(int size);
	~AgentBasePooler();
	
	AgentBasePooler(const AgentBasePooler& other) = delete;
	AgentBasePooler& operator=(const AgentBasePooler& other) = delete;
	AgentBasePooler(AgentBasePooler&& other) = delete;
	AgentBasePooler& operator=(AgentBasePooler&& other) = delete;

	void Update(float dt);
	void Render(bool renderGrid);

	Grid* GetGrid() { return m_pGrid; };

	bool& GetUsingMultiThreading() { return m_UsingMultithreading; };
	bool& GetUsingSeparation() { return m_UsingSeparation; };

	const std::vector<AgentBase*>& GetEnabledAgents() { return m_EnabledAgentBasePointers; };
	int GetEnabledAgentsCount() { return m_EnabledAgentsCount; };

	void GetEnabledAgentCountsByTeamId(int& id0, int& id1, int& id2, int& id3);

	AgentBase* SpawnNewAgent(int teamId, const Elite::Vector2& position, float radius, const Elite::Color& color, float healthAmount, float damage, float attackSpeed, float attackRange, float speed);
	

private:
	std::vector<AgentBase*> m_DisabledAgentBasePointers{};
	std::vector<AgentBase*> m_EnabledAgentBasePointers{};

	int m_DisabledAgentsCount{};
	int m_EnabledAgentsCount{};

	Grid* m_pGrid{};

	std::vector<int> m_TeamAgentsCount{};

	bool m_UsingMultithreading{false};
	bool m_UsingSeparation{false};

	void AddToDisabledAgents(AgentBase* pAgent);
};

