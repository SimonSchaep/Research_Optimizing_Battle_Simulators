//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "AgentBasePooler.h"
#include "Grid.h"
#include "Cell.h"

AgentBasePooler::AgentBasePooler(int size)
{
	m_TeamAgentsCount.resize(4);

	m_DisabledAgentBasePointers.resize(size);
	m_EnabledAgentBasePointers.resize(size);

	for (int i{}; i < size; ++i)
	{
		m_DisabledAgentBasePointers[i] = new AgentBase{};
	}

	m_DisabledAgentsCount = size;


	m_pGrid = new Grid{ 100, 100, 5 };
}

AgentBasePooler::~AgentBasePooler()
{
	//making sure all the agents get deleted
	for (AgentBase* pAgent :  m_DisabledAgentBasePointers)
	{
		SAFE_DELETE(pAgent);
	}

	for (AgentBase* pAgent : m_EnabledAgentBasePointers)
	{
		SAFE_DELETE(pAgent);
	}

	SAFE_DELETE(m_pGrid);
}

void AgentBasePooler::Update(float dt)
{
	m_TeamAgentsCount[0] = 0;
	m_TeamAgentsCount[1] = 0;
	m_TeamAgentsCount[2] = 0;
	m_TeamAgentsCount[3] = 0;

	std::vector<int> toDisableIds{};
	for (int i{}; i < m_EnabledAgentsCount; ++i)
	{
		if (!m_EnabledAgentBasePointers[i]->GetIsEnabled())
		{
			toDisableIds.push_back(i);
			continue;
		}
		m_EnabledAgentBasePointers[i]->Update(dt, this);
		++m_TeamAgentsCount[m_EnabledAgentBasePointers[i]->GetTeamId()];
	}

	//start from the back because we swap the agent to be removed with the last agent in the vector
	//if the last one is also in the to remove list, it would no longer be removed if we started from the front
	for (int i{ int(toDisableIds.size()) - 1}; i >= 0; --i)
	{
		// add to disabled agents
		m_DisabledAgentBasePointers[m_DisabledAgentsCount] = m_EnabledAgentBasePointers[toDisableIds[i]];
		++m_DisabledAgentsCount;

		// remove from enabled agents
		--m_EnabledAgentsCount;
		m_EnabledAgentBasePointers[toDisableIds[i]] = m_EnabledAgentBasePointers[m_EnabledAgentsCount];
		m_EnabledAgentBasePointers[m_EnabledAgentsCount] = nullptr;
	}

	m_pGrid->Update(dt, this);
}

void AgentBasePooler::Render(bool renderGrid)
{
	for (int i{}; i < m_EnabledAgentsCount; ++i)
	{
		m_EnabledAgentBasePointers[i]->Render();
	}

	if (renderGrid)
	{
		m_pGrid->Render();
	}	
}

void AgentBasePooler::GetEnabledAgentCountsByTeamId(int& id0, int& id1, int& id2, int& id3)
{
	id0 = m_TeamAgentsCount[0];
	id1 = m_TeamAgentsCount[1];
	id2 = m_TeamAgentsCount[2];
	id3 = m_TeamAgentsCount[3];
}

AgentBase* AgentBasePooler::SpawnNewAgent(int teamId, const Elite::Vector2& position, float radius, const Elite::Color& color, float healthAmount, float damage, float attackSpeed, float attackRange, float speed)
{
	--m_DisabledAgentsCount;
	m_EnabledAgentBasePointers[m_EnabledAgentsCount] = m_DisabledAgentBasePointers[m_DisabledAgentsCount];
	m_DisabledAgentBasePointers[m_DisabledAgentsCount] = nullptr;
	AgentBase* pNewAgent = m_EnabledAgentBasePointers[m_EnabledAgentsCount];
	++m_EnabledAgentsCount;

	pNewAgent->Enable(teamId, position, radius, color, healthAmount, damage, attackSpeed, attackRange, speed);

	m_pGrid->GetCells()[m_pGrid->GetCellId(position)]->AddAgent(pNewAgent);

	return pNewAgent;
}

void AgentBasePooler::AddToDisabledAgents(AgentBase* pAgent)
{
	// add to disabled agents
	m_DisabledAgentBasePointers[m_DisabledAgentsCount] = pAgent;
	++m_DisabledAgentsCount;

	// remove from enabled agents
	--m_EnabledAgentsCount;
	std::replace(m_EnabledAgentBasePointers.begin(), m_EnabledAgentBasePointers.end(), pAgent, m_EnabledAgentBasePointers[m_EnabledAgentsCount]);
	m_EnabledAgentBasePointers[m_EnabledAgentsCount] = nullptr;
}
