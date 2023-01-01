//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "QuadTreeNode.h"
#include "AgentBase.h"

QuadTreeNode::QuadTreeNode(const Elite::Vector2& minBounds, const Elite::Vector2& maxBounds)
	: m_MinBounds{ minBounds }
	, m_MaxBounds{ maxBounds }
{
	m_ChildNodes.resize(4);
	m_TeamAgentCounts.resize(4);
}

QuadTreeNode::~QuadTreeNode()
{
	for (QuadTreeNode* pNode : m_ChildNodes)
	{
		SAFE_DELETE(pNode);
	}
}

void QuadTreeNode::Render() const
{
	if (m_ChildNodes[0])
	{
		for (QuadTreeNode* pNode : m_ChildNodes)
		{
			pNode->Render();
		}
		return;
	}

	std::vector<Elite::Vector2> points{ 4 };
	points[0] = { m_MinBounds.x, m_MinBounds.y };
	points[1] = { m_MinBounds.x, m_MaxBounds.y };
	points[2] = { m_MaxBounds.x, m_MaxBounds.y };
	points[3] = { m_MaxBounds.x, m_MinBounds.y };
	//draw polygon through specified points
	DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0 }, -1);
	DEBUGRENDERER2D->DrawString({ points[0].x + (m_MaxBounds.x - m_MinBounds.x) / 2, points[0].y + (m_MaxBounds.y - m_MinBounds.y) / 2 }, std::to_string(m_AgentCount).c_str());
}

void QuadTreeNode::CheckSubDivide()
{
	if (m_ChildNodes[0]) //if we have children
	{
		for (QuadTreeNode* pNode : m_ChildNodes)
		{
			pNode->CheckSubDivide();
		}
		return;
	}

	//subdivide node
	if (m_AgentCount > m_MaxAgentCount)
	{
		const float sizeLimit{ 0.1f };

		const float halfCellWidth{ (m_MaxBounds.x - m_MinBounds.x) / 2 };
		const float halfCellHeight{ (m_MaxBounds.y - m_MinBounds.y) / 2 };
		//don't divide if size would become too small
		if (halfCellWidth < sizeLimit || halfCellHeight < sizeLimit)
		{
			return;
		}

		//create child nodes
		m_ChildNodes[0] = new QuadTreeNode{ {m_MinBounds.x, m_MinBounds.y},{m_MinBounds.x + halfCellWidth, m_MinBounds.y + halfCellHeight} };
		m_ChildNodes[1] = new QuadTreeNode{ {m_MinBounds.x + halfCellWidth, m_MinBounds.y},{m_MaxBounds.x, m_MinBounds.y + halfCellHeight} };
		m_ChildNodes[2] = new QuadTreeNode{ {m_MinBounds.x + halfCellWidth, m_MinBounds.y + halfCellHeight},{m_MaxBounds.x, m_MaxBounds.y} };
		m_ChildNodes[3] = new QuadTreeNode{ {m_MinBounds.x, m_MinBounds.y + halfCellHeight},{m_MinBounds.x + halfCellWidth, m_MaxBounds.y} };

		//add agents to children
		for (int i{}; i < m_AgentCount; ++i)
		{
			AddAgentToChild(m_Agents[i]);
			m_Agents[i] = nullptr;
		}
	}
}

void QuadTreeNode::FindClosestTarget(int ownTeamId, const Elite::Vector2& position, AgentBase** pClosestTarget,float& currentClosestDistanceSquared)
{
	if (m_ChildNodes[0])
	{
		//first check the node where our agent is
		for (QuadTreeNode* pNode : m_ChildNodes)
		{
			//first check the node that contains our agent
			if (position.x >= pNode->GetMinBounds().x && position.x < pNode->GetMaxBounds().x && position.y >= pNode->GetMinBounds().y && position.y < pNode->GetMaxBounds().y)
			{
				//find closest point to agent
				const float clampedX{ max(min(position.x, pNode->GetMaxBounds().x), pNode->GetMinBounds().x) };
				const float clampedY{ max(min(position.y, pNode->GetMaxBounds().y), pNode->GetMinBounds().y) };
				Elite::Vector2 closestPointToAgent{ clampedX, clampedY };
				//only search if it has agents that aren't all of our own team
				if (pNode->GetAgentCount() > pNode->GetTeamAgentCount(ownTeamId) && (!pClosestTarget ||
					//if the closest point to agent is within the radius
					closestPointToAgent.DistanceSquared(position) < currentClosestDistanceSquared
					))
				{
					pNode->FindClosestTarget(ownTeamId, position, pClosestTarget, currentClosestDistanceSquared);
					break;
				}
			}
		}
		for (QuadTreeNode* pNode : m_ChildNodes)
		{
			//find closest point to agent
			const float clampedX{ max(min(position.x, pNode->GetMaxBounds().x), pNode->GetMinBounds().x) };
			const float clampedY{ max(min(position.y, pNode->GetMaxBounds().y), pNode->GetMinBounds().y) };
			Elite::Vector2 closestPointToAgent{ clampedX, clampedY };
			//only search if it has agents that aren't all of our own team
			if (pNode->GetAgentCount() > pNode->GetTeamAgentCount(ownTeamId) && (!pClosestTarget ||
				//if the closest point to agent is within the radius
				closestPointToAgent.DistanceSquared(position) < currentClosestDistanceSquared
				))
			{
				pNode->FindClosestTarget(ownTeamId, position, pClosestTarget, currentClosestDistanceSquared);
			}
		}

		return;
	}

	//find closest target
	for (int i{}; i < m_AgentCount; ++i)
	{
		if (m_Agents[i]->GetTeamId() == ownTeamId)
		{
			continue;
		}
		const float dist{ m_Agents[i]->GetPosition().DistanceSquared(position) };
		if (dist < currentClosestDistanceSquared)
		{
			*pClosestTarget = m_Agents[i];
			currentClosestDistanceSquared = dist;
		}
	}
}

void QuadTreeNode::GetNearestNeighbors(AgentBase* pAgent, int ownTeamId, const Elite::Vector2& position, std::vector<AgentBase*>& neighbors, int& neighborCount, float neighborRadiusSquared)
{
	if (m_ChildNodes[0])
	{
		for (QuadTreeNode* pNode : m_ChildNodes)
		{
			//find closest point to agent
			const float clampedX{ max(min(position.x, pNode->GetMaxBounds().x), pNode->GetMinBounds().x) };
			const float clampedY{ max(min(position.y, pNode->GetMaxBounds().y), pNode->GetMinBounds().y) };
			Elite::Vector2 closestPointToAgent{ clampedX, clampedY };
			//only search if it has agents that are of our own team
			if (pNode->GetTeamAgentCount(ownTeamId) > 0 && (
				//if the closest point to agent is within the radius
				closestPointToAgent.DistanceSquared(position) < neighborRadiusSquared ||
				//or if the node contains the agent
				(position.x >= pNode->GetMinBounds().x && position.x < pNode->GetMaxBounds().x && position.y >= pNode->GetMinBounds().y && position.y < pNode->GetMaxBounds().y)
				))
			{
				pNode->GetNearestNeighbors(pAgent, ownTeamId, position, neighbors, neighborCount, neighborRadiusSquared);
			}
		}

		return;
	}

	//add all neighbors that are close enough
	for (int i{}; i < m_AgentCount; ++i)
	{
		//neighbor needs to be of our own team and not ourself
		if (m_Agents[i]->GetTeamId() != ownTeamId || m_Agents[i] == pAgent)
		{
			continue;
		}
		const float dist{ m_Agents[i]->GetPosition().DistanceSquared(position) };
		if (dist < neighborRadiusSquared)
		{
			if (neighbors.size() > neighborCount)
			{
				neighbors[neighborCount] = m_Agents[i];
			}
			else
			{
				neighbors.push_back(m_Agents[i]);
			}
			++neighborCount;
		}
	}
}

void QuadTreeNode::RemoveAgent(AgentBase* pAgent)
{
	if (m_ChildNodes[0]) //if we have children
	{
		RemoveAgentFromChild(pAgent);
		--m_AgentCount;
		--m_TeamAgentCounts[pAgent->GetTeamId()];

		if (m_AgentCount == m_MaxAgentCount)
		{
			//combine children into self
			//all child nodes won't have more children since they would have merged from a previous RemoveAgentFromChild() call
			int counter{};
			for (QuadTreeNode* pNode : m_ChildNodes)
			{
				for (int i{}; i < pNode->GetAgentCount(); ++i)
				{
					m_Agents[counter] = pNode->GetAgents()[i];
					m_Agents[counter]->SetCell(this);
					++counter;
				}
			}

			//delete children
			for (size_t i{}; i < m_ChildNodes.size(); ++i)
			{
				delete m_ChildNodes[i];
				m_ChildNodes[i] = nullptr;
			}
		}
		return;
	}

	
	--m_AgentCount;
	--m_TeamAgentCounts[pAgent->GetTeamId()];
	//put last agent in vector to position of the agent that needs to be removed
	std::replace(m_Agents.begin(), m_Agents.end(), pAgent, m_Agents[m_AgentCount]);
	//remove last agent
	m_Agents[m_AgentCount] = nullptr;
}

void QuadTreeNode::AddAgent(AgentBase* pAgent)
{	
	//add agent to child
	if (m_ChildNodes[0]) //if we have children
	{
		//look in which node the agent is and add it to that node
		AddAgentToChild(pAgent);
		++m_AgentCount;
		++m_TeamAgentCounts[pAgent->GetTeamId()];
		
		return;
	}	

	//add agent to self
	if (m_Agents.size() > m_AgentCount) //if enough nullptrs left in vector
	{
		m_Agents[m_AgentCount] = pAgent;
	}
	else
	{
		m_Agents.push_back(pAgent);
	}	

	++m_AgentCount;
	++m_TeamAgentCounts[pAgent->GetTeamId()];

	pAgent->SetCell(this);
}

void QuadTreeNode::AddAgentToChild(AgentBase* pAgent)
{
	const Elite::Vector2& pos{ pAgent->GetPosition() }; //take current position
	for (QuadTreeNode* pNode : m_ChildNodes)
	{
		//check which child should contain the agent
		if (pos.x >= pNode->GetMinBounds().x && pos.x < pNode->GetMaxBounds().x && pos.y >= pNode->GetMinBounds().y && pos.y < pNode->GetMaxBounds().y)
		{
			pNode->AddAgent(pAgent);
			return;
		}
	}

	//make sure everything still works when agents go out of bounds
	// try to still avoid this from happening since other things might still not work anymore
	const float clampedX{ max(min(pos.x, m_MaxBounds.x), m_MinBounds.x) };
	const float clampedY{ max(min(pos.y, m_MaxBounds.y), m_MinBounds.y) };
	const Elite::Vector2& altPos{ clampedX, clampedY };

	for (QuadTreeNode* pNode : m_ChildNodes)
	{
		//check which child should contain the agent
		if (altPos.x >= pNode->GetMinBounds().x && altPos.x <= pNode->GetMaxBounds().x && altPos.y >= pNode->GetMinBounds().y && altPos.y <= pNode->GetMaxBounds().y)
		{
			pNode->AddAgent(pAgent);
			return;
		}
	}
}

void QuadTreeNode::RemoveAgentFromChild(AgentBase* pAgent)
{
	const Elite::Vector2& pos{ pAgent->GetNodePosition() }; //take old position, since new position might be in different parent node
	for (QuadTreeNode* pNode : m_ChildNodes)
	{
		//check which child contained the agent
		if (pos.x >= pNode->GetMinBounds().x && pos.x < pNode->GetMaxBounds().x && pos.y >= pNode->GetMinBounds().y && pos.y < pNode->GetMaxBounds().y)
		{
			pNode->RemoveAgent(pAgent);
			return;
		}
	}
}
