//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "AgentBasePooler.h"
#include "Grid.h"
#include "Cell.h"
#include <ppl.h>

AgentBasePooler::AgentBasePooler(int size, const Elite::Vector2& worldDimensions)
{
	m_DisabledAgentBasePointers.resize(size);
	m_EnabledAgentBasePointers.resize(size);

	for (int i{}; i < size; ++i)
	{
		m_DisabledAgentBasePointers[i] = new AgentBase{};
	}

	m_DisabledAgentsCount = size;


	m_pGrid = new Grid{ 20, 20, worldDimensions.x / 20.f };
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
	std::vector<int> toDisableIds; //store agents that need to be disabled in here to disable them later, cause disabling while looping will cause issues
	toDisableIds.reserve(m_EnabledAgentsCount); //reserve is necessary when multithreading since resizing asynchronously would cause issues, will also make it faster to disable many agents in one frame

	if (m_UsingMultithreading) //multithreading
	{
		concurrency::parallel_for(0, m_EnabledAgentsCount, [this, dt, &toDisableIds](int i)
			{
				if (!m_EnabledAgentBasePointers[i]->GetIsEnabled())
				{
					toDisableIds.push_back(i);
				}
				else
				{
					m_EnabledAgentBasePointers[i]->Update(dt, this, m_UsingSeparation, false);
				}
			});

		for (int i{}; i < m_EnabledAgentsCount; ++i)
		{
			if (m_EnabledAgentBasePointers[i]->GetIsEnabled())
			{
				m_EnabledAgentBasePointers[i]->CheckIfCellChanged(this);
			}			
		}

		//since we are using multiple threads, the vector won't be sorted
		//but we need it to be sorted for the disabling to work
		std::sort(toDisableIds.begin(), toDisableIds.end());
	}
	else //no multithreading
	{
		for (int i{}; i < m_EnabledAgentsCount; ++i)
		{
			if (!m_EnabledAgentBasePointers[i]->GetIsEnabled())
			{
				toDisableIds.push_back(i);
			}
			else
			{
				m_EnabledAgentBasePointers[i]->Update(dt, this, m_UsingSeparation, true);
			}
		}
	}

	//start from the back because we swap the agent to be removed with the last agent in the vector
	//if the last one is also in the to remove list, it would no longer be removed if we started from the front
	for (int i{ int(toDisableIds.size()) - 1}; i >= 0; --i)
	{
		if (toDisableIds[i] < 0 || toDisableIds[i] >= m_EnabledAgentsCount) //sometimes contains very high or very low values when using multithreading, no idea why
		{
			continue;
		}
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
	//if not enough agents in pool
	if (m_DisabledAgentsCount <= 0)
	{
		//allocate memory for 1000 more
		const int extraAgentsCount{ 1000 };
		m_DisabledAgentsCount += extraAgentsCount;
		m_DisabledAgentBasePointers.resize(m_DisabledAgentBasePointers.size() + extraAgentsCount); //increase size of the vector
		m_EnabledAgentBasePointers.resize(m_EnabledAgentBasePointers.size() + extraAgentsCount); //increase size of the vector
		//set first 1000 elements to new agents
		for (int i{}; i < extraAgentsCount; ++i)
		{
			m_DisabledAgentBasePointers[i] = new AgentBase{};
		}
	}

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
