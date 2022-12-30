#pragma once
class AgentBase;

class QuadTreeNode
{
public:
	QuadTreeNode(const Elite::Vector2& minBounds, const Elite::Vector2& maxBounds, bool isRoot);
	~QuadTreeNode();

	QuadTreeNode(const QuadTreeNode& other) = delete;
	QuadTreeNode& operator=(const QuadTreeNode& other) = delete;
	QuadTreeNode(QuadTreeNode&& other) = delete;
	QuadTreeNode& operator=(QuadTreeNode&& other) = delete;

	void Render()const;

	const Elite::Vector2& GetMinBounds() { return m_MinBounds; };
	const Elite::Vector2& GetMaxBounds() { return m_MaxBounds; };

	const std::vector<AgentBase*>& GetAgents() { return m_Agents; };
	int GetAgentCount() { return m_AgentCount; };

	void RemoveAgent(AgentBase* pAgent); //only remove when certain it is part of this cell, otherwise count will be messed up
	void AddAgent(AgentBase* pAgent);

	bool HasChildNodes() { return m_ChildNodes[0]; };

private:
	std::vector<AgentBase*> m_Agents{};
	int m_AgentCount{};

	const int m_MaxAgentCount{20};

	bool m_IsRoot{};

	std::vector<QuadTreeNode*> m_ChildNodes{};

	Elite::Vector2 m_MinBounds{};
	Elite::Vector2 m_MaxBounds{};

	void AddAgentToChild(AgentBase* pAgent);
	void RemoveAgentFromChild(AgentBase* pAgent);
};

