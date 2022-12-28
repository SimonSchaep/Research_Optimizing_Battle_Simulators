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
	std::replace(m_Agents.begin(), m_Agents.end(), pAgent, m_Agents[m_AgentCount]);
	m_Agents[m_AgentCount] = nullptr;
}

void Cell::AddAgent(AgentBase* pAgent)
{	
	if (m_Agents.size() > m_AgentCount) //if enough nullptrs left in vector
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
