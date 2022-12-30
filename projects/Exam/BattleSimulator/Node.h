#pragma once
class AgentBase;

class Node
{
public:
	Node();
	~Node();

	Node(const Node& other) = delete;
	Node& operator=(const Node& other) = delete;
	Node(Node&& other) = delete;
	Node& operator=(Node&& other) = delete;

	const std::vector<AgentBase*>& GetAgents() { return m_Agents; };
	int GetAgentCount() { return m_AgentCount; };

	void RemoveAgent(AgentBase* pAgent); //only remove when certain it is part of this cell, otherwise count will be messed up
	void AddAgent(AgentBase* pAgent);

private:
	std::vector<AgentBase*> m_Agents{};

	int m_AgentCount{};
};

