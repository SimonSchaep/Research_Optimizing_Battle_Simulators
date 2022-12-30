//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "AgentBasePooler.h"
#include <ppl.h>

AgentBasePooler::AgentBasePooler(int size)
{
	m_DisabledAgentBasePointers.resize(size);
	m_EnabledAgentBasePointers.resize(size);

	for (int i{}; i < size; ++i)
	{
		m_DisabledAgentBasePointers[i] = new AgentBase{};
	}

	m_DisabledAgentsCount = size;
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
}

void AgentBasePooler::Update(float dt)
{
	std::vector<int> toDisableIds;
	toDisableIds.reserve(m_EnabledAgentsCount); //need to reserve space in the vector since resizing during parallel_for can cause issues

	concurrency::parallel_for(0, m_EnabledAgentsCount, [this, dt, &toDisableIds](int i)
		{
			if (!m_EnabledAgentBasePointers[i]->GetIsEnabled())
			{
				toDisableIds.push_back(i); //has pushed back a negative value once in debug mode, no idea why
			}
			else
			{
				m_EnabledAgentBasePointers[i]->Update(dt, this);
			}
		});

	//since we are using multiple threads, the vector won't be sorted
	//but we need it to be sorted for the disabling to work
	std::sort(toDisableIds.begin(), toDisableIds.end());

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
}

void AgentBasePooler::Render()
{
	for (int i{}; i < m_EnabledAgentsCount; ++i)
	{
		m_EnabledAgentBasePointers[i]->Render();
	}
}

void AgentBasePooler::GetEnabledAgentCountsByTeamId(int& id0, int& id1, int& id2, int& id3)
{
	id0 = 0;
	id1 = 0;
	id2 = 0;
	id3 = 0;

	for (int i{}; i < m_EnabledAgentsCount; ++i)
	{
		switch (m_EnabledAgentBasePointers[i]->GetTeamId())
		{
		case 0:
			++id0;
			break;
		case 1:
			++id1;
			break;
		case 2:
			++id2;
			break;
		case 3:
			++id3;
			break;
		};
	}
}

AgentBase* AgentBasePooler::SpawnNewAgent(int teamId, const Elite::Vector2& position, float radius, const Elite::Color& color, float healthAmount, float damage, float attackSpeed, float attackRange, float speed)
{
	--m_DisabledAgentsCount;
	m_EnabledAgentBasePointers[m_EnabledAgentsCount] = m_DisabledAgentBasePointers[m_DisabledAgentsCount];
	m_DisabledAgentBasePointers[m_DisabledAgentsCount] = nullptr;
	AgentBase* pNewAgent = m_EnabledAgentBasePointers[m_EnabledAgentsCount];
	++m_EnabledAgentsCount;

	pNewAgent->Enable(teamId, position, radius, color, healthAmount, damage, attackSpeed, attackRange, speed);
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
