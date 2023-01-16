#pragma once
#include "AgentBase.h"

class AgentBasePooler final
{
public:
	AgentBasePooler(int size);
	~AgentBasePooler();
	
	AgentBasePooler(const AgentBasePooler& other) = delete;
	AgentBasePooler& operator=(const AgentBasePooler& other) = delete;
	AgentBasePooler(AgentBasePooler&& other) = delete;
	AgentBasePooler& operator=(AgentBasePooler&& other) = delete;

	void Update(float dt);
	void Render();

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

	bool m_UsingMultithreading{true};
	bool m_UsingSeparation{true};

	void AddToDisabledAgents(AgentBase* pAgent);
};

