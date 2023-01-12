//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "AgentBase.h"
#include "AgentBasePooler.h"
#include "Cell.h"
#include "Grid.h"
#include <ppl.h>

AgentBase::AgentBase()
{
}

AgentBase::~AgentBase()
{
}

void AgentBase::Enable(int teamId, const Elite::Vector2& position, float radius, const Elite::Color& color, float healthAmount, float damage, float attackSpeed, float attackRange, float speed)
{
	m_pTargetAgent = nullptr;
	m_TargetPosition = {};
	m_IsEnabled = true;
	m_TeamId = teamId;
	m_Position = position;
	m_Radius = radius;
	m_Color = color;
	m_Health.Reset(healthAmount);
	m_MeleeAttack.Reset(damage, attackSpeed, attackRange);
	m_Speed = speed;
}

void AgentBase::Disable()
{
	m_IsEnabled = false;

	assert(m_pCell);
	m_pCell->RemoveAgent(this);
	m_pCell = nullptr;
}

void AgentBase::Update(float dt, AgentBasePooler* pAgentBasePooler, bool separation, bool checkCell)
{
	if (checkCell && m_Health.IsDead()) //disable also checks cell, so for multithreading, this check needs to happen in separate function
	{
		Disable();
		return;
	}

	FindTarget(pAgentBasePooler, separation);

	//update timers for meleeattack component
	m_MeleeAttack.Update(dt);

	CalculateVelocity(dt, separation);
	if (!Move(dt) && m_pTargetAgent)
	{
		m_MeleeAttack.TryAttack(m_pTargetAgent);
	}
	
	//check if we need to update our cell after we moved
	else if(checkCell && pAgentBasePooler->GetGrid()->GetCells()[pAgentBasePooler->GetGrid()->GetCellId(m_Position)] != m_pCell)
	{
		m_pCell->RemoveAgent(this);
		pAgentBasePooler->GetGrid()->GetCells()[pAgentBasePooler->GetGrid()->GetCellId(m_Position)]->AddAgent(this);		
	}
}

void AgentBase::CheckIfCellChanged(AgentBasePooler* pAgentBasePooler)
{
	if (m_Health.IsDead())
	{
		Disable();
		return;
	}

	if (pAgentBasePooler->GetGrid()->GetCells()[pAgentBasePooler->GetGrid()->GetCellId(m_Position)] != m_pCell)
	{
		m_pCell->RemoveAgent(this);
		pAgentBasePooler->GetGrid()->GetCells()[pAgentBasePooler->GetGrid()->GetCellId(m_Position)]->AddAgent(this);
	}
}

void AgentBase::Render()
{
	DEBUGRENDERER2D->DrawSolidCircle(m_Position, m_Radius, { 0,0 }, m_Color);
}

void AgentBase::CalculateVelocity(float dt, bool separation)
{
	//go towards target agent or position
	if (m_pTargetAgent)
	{
		m_Velocity = m_pTargetAgent->GetPosition() - m_Position;
		m_Velocity.Normalize();
	}
	else
	{
		m_Velocity = m_TargetPosition - m_Position;
		m_Velocity.Normalize();
	}

	//separation
	for (int i{}; i < m_NeighborCount; ++i)
	{
		float modifier{ 2.f };

		const float minDistance{ 0.5f };//make sure velocity can't go infinitely high
		m_Velocity += modifier * ((m_Position - m_Neighbors[i]->GetPosition()) / max(m_Neighbors[i]->GetPosition().DistanceSquared(m_Position), minDistance));
	}

	m_Velocity.Normalize();

	//collision
	Elite::Vector2 collisionForce{};

	if (separation)
	{
		for (int i{}; i < m_NeighborCount; ++i)
		{
			if (m_Neighbors[i]->GetPosition().DistanceSquared(m_Position + m_Velocity * m_Speed * dt) < 4) //if our new position would collide
			{
				collisionForce += ((m_Position + m_Velocity * m_Speed * dt) - m_Neighbors[i]->GetPosition()).GetNormalized();
			}
		}
	}

	m_Velocity += collisionForce.GetNormalized();

	m_Velocity *= m_Speed;
}

bool AgentBase::Move(float dt)
{
	if (m_pTargetAgent && m_MeleeAttack.IsTargetInRange(m_pTargetAgent, m_Position))
	{
		return false;
	}

	m_Position += m_Velocity * dt;
	return true;
}

void AgentBase::FindTarget(AgentBasePooler* pAgentBasePooler, bool findNeighbors)
{
	if (!m_pCell->GetClosestCell(m_TeamId))
	{
		return;
	}

	//reset vars
	m_pTargetAgent = nullptr;
	m_NeighborCount = 0;

	//get closest cell for this team
	const std::vector<AgentBase*>& agents{ m_pCell->GetClosestCell(m_TeamId)->GetAgents() };

	//find target
	for (int i{}; i < m_pCell->GetClosestCell(m_TeamId)->GetAgentCount(); ++i)
	{
		if (agents[i]->GetTeamId() != m_TeamId && (!m_pTargetAgent || !m_pTargetAgent->GetIsEnabled() || agents[i]->GetPosition().DistanceSquared(m_Position) < m_pTargetAgent->GetPosition().DistanceSquared(m_Position)))
		{
			m_pTargetAgent = agents[i];
		}
	}

	//set position for when no more valid targets
	if (m_pTargetAgent)
	{
		m_TargetPosition = m_pTargetAgent->GetPosition();
	}

	if (!findNeighbors)
	{
		return;
	}

	//find neighbors
	int row{};
	int col{};

	int range{};
	const int maxRange{ 2 };

	//get current row and col
	pAgentBasePooler->GetGrid()->GetRowCol(pAgentBasePooler->GetGrid()->GetCellId(m_Position), row, col);
	//check own cell
	CheckCellForNeighbors(pAgentBasePooler, row, col);

	while (range < maxRange)
	{
		++range;

		for (int r{ -range }; r <= range; ++r)
		{
			CheckCellForNeighbors(pAgentBasePooler, row + r, col + (range - abs(r)));
			if (abs(r) != range) //if not at very bottom or very top
			{
				//check second cell opposite to previous one checked
				CheckCellForNeighbors(pAgentBasePooler, row + r, col - (range - abs(r)));
			}
		}
	}
}

void AgentBase::CheckCellForNeighbors(AgentBasePooler* pAgentBasePooler, int row, int col)
{
	const float neighborRadiusSquared{ 10 };

	int cellId{ pAgentBasePooler->GetGrid()->GetCellId(row, col) };
	Cell* pCell{ pAgentBasePooler->GetGrid()->GetCells()[cellId] };
	const std::vector<AgentBase*>& agents = pCell->GetAgents();
	for (int agentId{}; agentId < pCell->GetAgentCount(); ++agentId)
	{
		if (agents[agentId]->GetTeamId() == m_TeamId && agents[agentId] != this && agents[agentId]->GetPosition().DistanceSquared(m_Position) <= neighborRadiusSquared)
		{
			if (int(m_Neighbors.size()) > m_NeighborCount)
			{
				m_Neighbors[m_NeighborCount] = agents[agentId];
			}
			else
			{
				m_Neighbors.push_back(agents[agentId]);
			}
			++m_NeighborCount;
		}
	}
}
