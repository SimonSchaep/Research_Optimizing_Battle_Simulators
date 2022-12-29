#pragma once
class AgentBase;
class AgentBasePooler;

class Cell
{
public:
	Cell(int id);
	~Cell();

	Cell(const Cell& other) = delete;
	Cell& operator=(const Cell& other) = delete;
	Cell(Cell&& other) = delete;
	Cell& operator=(Cell&& other) = delete;

	void Update(float dt, AgentBasePooler* pAgentBasePooler);

	const std::vector<AgentBase*>& GetAgents() { return m_Agents; };
	int GetAgentCount() { return m_AgentCount; };
	int GetAgentCountByTeam(int teamId) { return m_TeamAgentCounts[teamId]; };

	Cell* GetClosestCell(int teamId) { return m_pClosestCells[teamId]; };

	void RemoveAgent(AgentBase* pAgent); //only remove when certain it is part of this cell, otherwise count will be messed up
	void AddAgent(AgentBase* pAgent);

private:
	std::vector<AgentBase*> m_Agents{};

	int m_AgentCount{};

	std::vector<int> m_TeamAgentCounts{};

	int m_Id{};

	std::vector<Cell*> m_pClosestCells{4}; //holds closest cell for every team

	void CheckCell(AgentBasePooler* pAgentBasePooler, bool& stop, int row, int col);
};

