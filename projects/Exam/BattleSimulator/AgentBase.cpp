//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "AgentBase.h"
#include "AgentBasePooler.h"

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

void AgentBase::Update(float dt, AgentBasePooler* pAgentBasePooler, bool separation)
{
	if (m_Health.IsDead())
	{
		m_IsEnabled = false;
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
	const float neighborRadiusSquared{ 40 };
	m_NeighborCount = 0;

	const std::vector<AgentBase*>& agents{ pAgentBasePooler->GetEnabledAgents() };

	for (int i{}; i < pAgentBasePooler->GetEnabledAgentsCount(); ++i)
	{
		//find target
		if (agents[i]->GetTeamId() != m_TeamId && (!m_pTargetAgent || !m_pTargetAgent->GetIsEnabled() || agents[i]->GetPosition().DistanceSquared(m_Position) < m_pTargetAgent->GetPosition().DistanceSquared(m_Position)))
		{
			m_pTargetAgent = agents[i];
		}
		//find neighbors
		else if (findNeighbors && agents[i]->GetTeamId() == m_TeamId && agents[i] != this && agents[i]->GetPosition().DistanceSquared(m_Position) <= neighborRadiusSquared)
		{
			if (int(m_Neighbors.size()) > m_NeighborCount)
			{
				m_Neighbors[m_NeighborCount] = agents[i];
			}
			else
			{
				m_Neighbors.push_back(agents[i]);
			}
			++m_NeighborCount;
		}
	}
}
