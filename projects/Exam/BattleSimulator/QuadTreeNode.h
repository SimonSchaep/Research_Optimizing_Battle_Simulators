#pragma once
class AgentBase;

class QuadTreeNode final
{
public:
	QuadTreeNode(const Elite::Vector2& minBounds, const Elite::Vector2& maxBounds);
	~QuadTreeNode();

	QuadTreeNode(const QuadTreeNode& other) = delete;
	QuadTreeNode& operator=(const QuadTreeNode& other) = delete;
	QuadTreeNode(QuadTreeNode&& other) = delete;
	QuadTreeNode& operator=(QuadTreeNode&& other) = delete;

	void Render()const;
	void CheckSubDivide();

	void FindClosestTarget(int ownTeamId, const Elite::Vector2& position, AgentBase** pClosestTarget, float& currentClosestDistanceSquared);
	void GetNearestNeighbors(AgentBase* pAgent, int ownTeamId, const Elite::Vector2& position, std::vector<AgentBase*>& neighbors, int& neighborCount, float neighborRadiusSquared);

	const Elite::Vector2& GetMinBounds() { return m_MinBounds; };
	const Elite::Vector2& GetMaxBounds() { return m_MaxBounds; };

	const std::vector<AgentBase*>& GetAgents() { return m_Agents; };
	int GetAgentCount() { return m_AgentCount; };

	int GetTeamAgentCount(int teamId) { return m_TeamAgentCounts[teamId]; };

	void RemoveAgent(AgentBase* pAgent); //only call this on the root!!!
	void AddAgent(AgentBase* pAgent);

	bool HasChildNodes() { return m_ChildNodes[0]; };

private:
	std::vector<AgentBase*> m_Agents{};
	int m_AgentCount{};

	std::vector<int> m_TeamAgentCounts{};

	const int m_MaxAgentCount{20};

	std::vector<QuadTreeNode*> m_ChildNodes{};

	Elite::Vector2 m_MinBounds{};
	Elite::Vector2 m_MaxBounds{};

	void AddAgentToChild(AgentBase* pAgent);
	void RemoveAgentFromChild(AgentBase* pAgent);
};

