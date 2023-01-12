//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "Cell.h"
#include "AgentBase.h"

Cell::Cell()
{

}

Cell::~Cell()
{
}

void Cell::RemoveAgent(AgentBase* pAgent)
{
	--m_AgentCount;
	//put last agent in vector to position of the agent that needs to be removed
	std::replace(m_Agents.begin(), m_Agents.end(), pAgent, m_Agents[m_AgentCount]);
	//remove last agent
	m_Agents[m_AgentCount] = nullptr;
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
}
