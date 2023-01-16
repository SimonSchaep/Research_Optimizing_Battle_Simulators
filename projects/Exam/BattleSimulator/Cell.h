#pragma once
class AgentBase;

class Cell final
{
public:
	Cell();
	~Cell();

	Cell(const Cell& other) = delete;
	Cell& operator=(const Cell& other) = delete;
	Cell(Cell&& other) = delete;
	Cell& operator=(Cell&& other) = delete;

	const std::vector<AgentBase*>& GetAgents() { return m_Agents; };
	int GetAgentCount() { return m_AgentCount; };

	void RemoveAgent(AgentBase* pAgent); //only remove when certain it is part of this cell, otherwise count will be messed up
	void AddAgent(AgentBase* pAgent);

private:
	std::vector<AgentBase*> m_Agents{};

	int m_AgentCount{};
};

