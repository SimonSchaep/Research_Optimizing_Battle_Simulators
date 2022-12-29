//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "AgentBase.h"
#include "AgentBasePooler.h"
#include "Cell.h"
#include "Grid.h"

AgentBase::AgentBase()
{
}

AgentBase::~AgentBase()
{
}

void AgentBase::Enable(int teamId, const Elite::Vector2& position, float radius, const Elite::Color& color, float healthAmount, float damage, float attackSpeed, float attackRange, float speed)
{
	m_pTargetAgent = nullptr;
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

void AgentBase::Update(float dt, AgentBasePooler* pAgentBasePooler)
{
	if (m_Health.IsDead())
	{
		Disable();
		return;
	}

	FindTarget(pAgentBasePooler);

	m_MeleeAttack.Update(dt);

	CalculateVelocity();
	if (!Move(dt) && m_pTargetAgent)
	{
		m_MeleeAttack.TryAttack(m_pTargetAgent);
	}
	//check if we need to update our cell after we moved
	else if(pAgentBasePooler->GetGrid()->GetCells()[pAgentBasePooler->GetGrid()->GetCellId(m_Position)] != m_pCell)
	{
		m_pCell->RemoveAgent(this);
		pAgentBasePooler->GetGrid()->GetCells()[pAgentBasePooler->GetGrid()->GetCellId(m_Position)]->AddAgent(this);		
	}
}

void AgentBase::Render()
{
	DEBUGRENDERER2D->DrawSolidCircle(m_Position, m_Radius, { 0,0 }, m_Color);
}

void AgentBase::CalculateVelocity()
{
	if (m_pTargetAgent)
	{
		m_Velocity = m_pTargetAgent->GetPosition() - m_Position;
		m_Velocity.Normalize();
		m_Velocity *= m_Speed;
		return;
	}

	m_Velocity = m_TargetPosition - m_Position;
	m_Velocity.Normalize();
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

void AgentBase::FindTarget(AgentBasePooler* pAgentBasePooler)
{
	if (!m_pCell->GetClosestCell(m_TeamId))
	{
		return;
	}

	//get closest cell for this team
	const std::vector<AgentBase*>& agents{ m_pCell->GetClosestCell(m_TeamId)->GetAgents()};

	for (int i{}; i < m_pCell->GetClosestCell(m_TeamId)->GetAgentCount(); ++i)
	{
		if (agents[i]->GetTeamId() != m_TeamId && (!m_pTargetAgent || !m_pTargetAgent->GetIsEnabled() || agents[i]->GetPosition().DistanceSquared(m_Position) < m_pTargetAgent->GetPosition().DistanceSquared(m_Position)))
		{
			m_pTargetAgent = agents[i];
		}
	}
}
