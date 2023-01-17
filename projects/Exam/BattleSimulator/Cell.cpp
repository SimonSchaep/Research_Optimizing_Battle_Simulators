//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "Cell.h"
#include "AgentBase.h"
#include "AgentBasePooler.h"
#include "Grid.h"

Cell::Cell(int id)
	: m_Id{id}
{
	m_TeamAgentCounts.resize(4);
}

Cell::~Cell()
{
	
}

void Cell::Update(float dt, AgentBasePooler* pAgentBasePooler)
{
	if (m_AgentCount == 0) //no need updating closestcell if no agent is going to use it
	{
		return;
	}

	m_pClosestCells[0] = nullptr;
	m_pClosestCells[1] = nullptr;
	m_pClosestCells[2] = nullptr;
	m_pClosestCells[3] = nullptr;

	int row{};
	int col{};

	int range{};
	const int minRange{ 3 };
	const int maxRange{ 100 };

	bool stop{};

	//get current row and col
	pAgentBasePooler->GetGrid()->GetRowCol(m_Id, row, col);
	//check self
	CheckCell(pAgentBasePooler, stop, row, col);

	while (range < maxRange && (!stop || range < minRange))
	{
		++range;

		for (int r{ -range }; r <= range; ++r)
		{
			CheckCell(pAgentBasePooler, stop, row + r, col + (range - abs(r)));
			if (abs(r) != range) //if not at very bottom or very top
			{
				//check second cell opposite to previous one checked
				CheckCell(pAgentBasePooler, stop, row + r, col - (range - abs(r)));
			}
		}
	}
}

void Cell::RemoveAgent(AgentBase* pAgent)
{
	--m_AgentCount;
	std::replace(m_Agents.begin(), m_Agents.end(), pAgent, m_Agents[m_AgentCount]);
	m_Agents[m_AgentCount] = nullptr;

	--m_TeamAgentCounts[pAgent->GetTeamId()];
}

void Cell::AddAgent(AgentBase* pAgent)
{	
	if (int(m_Agents.size()) > m_AgentCount) //if enough nullptrs left in vector
	{
		m_Agents[m_AgentCount] = pAgent;
	}
	else
	{
		m_Agents.push_back(pAgent);
	}
	
	pAgent->SetCell(this);

	++m_AgentCount;

	++m_TeamAgentCounts[pAgent->GetTeamId()];
}

void Cell::CheckCell(AgentBasePooler* pAgentBasePooler, bool& stop, int row, int col)
{
	//get cell
	int cellId{ pAgentBasePooler->GetGrid()->GetCellId(row, col) };
	Cell* pCell{ pAgentBasePooler->GetGrid()->GetCells()[cellId] };

	//if we there are agents of a team in the cell
	//set the closest cell of the other teams to this one
	//but only if they didn't already have a (closer) cell
	for (int i{}; i < 4; ++i)
	{
		if (pCell->GetAgentCountByTeam(i) > 0)
		{
			if (m_pClosestCells[(i + 1) % 4] == 0)
				m_pClosestCells[(i + 1) % 4] = pCell;
			if (m_pClosestCells[(i + 2) % 4] == 0)
				m_pClosestCells[(i + 2) % 4] = pCell;
			if (m_pClosestCells[(i + 3) % 4] == 0)
				m_pClosestCells[(i + 3) % 4] = pCell;
		}
	}

	if ((m_pClosestCells[0] != 0 || m_TeamAgentCounts[0] == 0) && 
		(m_pClosestCells[1] != 0 || m_TeamAgentCounts[1] == 0) && 
		(m_pClosestCells[2] != 0 || m_TeamAgentCounts[2] == 0) &&
		(m_pClosestCells[3] != 0 || m_TeamAgentCounts[3] == 0))
	{
		stop = true;
	}
}
