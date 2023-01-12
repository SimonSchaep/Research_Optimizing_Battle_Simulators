//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "AgentBase.h"
#include "AgentBasePooler.h"
#include <ppl.h>

AgentBase::AgentBase()
{
}

AgentBase::~AgentBase()
{
}

void AgentBase::Enable(int teamId, const Elite::Vector2& position, float radius, const Elite::Color& color, float healthAmount, float damage, float attackSpeed, float attackRange, float speed)
{
	m_pQuadTreeNode = nullptr;
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

void AgentBase::Update(float dt, AgentBasePooler* pAgentBasePooler, bool separation, bool checkCell)
{
	if (checkCell && m_Health.IsDead()) //disable also checks cell, so for multithreading, this check needs to happen in separate function
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
	
	//check if we need to update our node after we moved (not done during multithreading)
	else if(checkCell && (
		m_Position.x < m_pQuadTreeNode->GetMinBounds().x || m_Position.x >= m_pQuadTreeNode->GetMaxBounds().x ||
		m_Position.y < m_pQuadTreeNode->GetMinBounds().y || m_Position.y >= m_pQuadTreeNode->GetMaxBounds().y
		)) //check if left bounds of current node
	{
		pAgentBasePooler->GetQuadTreeRoot()->RemoveAgent(this);
		pAgentBasePooler->GetQuadTreeRoot()->AddAgent(this);
	}
}

void AgentBase::CheckIfCellChanged(AgentBasePooler* pAgentBasePooler)
{
	if (m_Health.IsDead())
	{
		m_IsEnabled = false;
		return;
	}

	if (
		m_Position.x < m_pQuadTreeNode->GetMinBounds().x || m_Position.x >= m_pQuadTreeNode->GetMaxBounds().x ||
		m_Position.y < m_pQuadTreeNode->GetMinBounds().y || m_Position.y >= m_pQuadTreeNode->GetMaxBounds().y
		) //check if left bounds of current node
	{
		pAgentBasePooler->GetQuadTreeRoot()->RemoveAgent(this);
		pAgentBasePooler->GetQuadTreeRoot()->AddAgent(this);
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
				collisionForce += ((m_Position + m_Velocity * m_Speed * dt) - m_Neighbors[i]->GetPosition()).GetNormalized(); //push away
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
	//reset vars
	m_pTargetAgent = nullptr;
	m_NeighborCount = 0;

	//find neighbors
	if (findNeighbors)
	{
		pAgentBasePooler->GetQuadTreeRoot()->GetNearestNeighbors(this, m_TeamId, m_Position, m_Neighbors, m_NeighborCount, 40);
	}	

	//find target
	float closestDistance{FLT_MAX};
	pAgentBasePooler->GetQuadTreeRoot()->FindClosestTarget(m_TeamId, m_Position, &m_pTargetAgent, closestDistance);

	if (m_pTargetAgent && !m_pTargetAgent->GetIsEnabled())
	{
		std::cerr << "error\n";
	}

	//set position for when no more valid targets
	if (m_pTargetAgent)
	{
		m_TargetPosition = m_pTargetAgent->GetPosition();
	}
}
