//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "QuadTreeNode.h"
#include "AgentBase.h"

QuadTreeNode::QuadTreeNode(const Elite::Vector2& minBounds, const Elite::Vector2& maxBounds, bool isRoot)
	: m_MinBounds{minBounds}
	, m_MaxBounds{ maxBounds }
	, m_IsRoot{isRoot}
{
	m_ChildNodes.resize(4);
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
	DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0 }, -1);
	DEBUGRENDERER2D->DrawString({ points[0].x + (m_MaxBounds.x - m_MinBounds.x) / 2, points[0].y + (m_MaxBounds.y - m_MinBounds.y) / 2 }, std::to_string(m_AgentCount).c_str());
}

void QuadTreeNode::RemoveAgent(AgentBase* pAgent)
{
	if (m_ChildNodes[0]) //if we have children
	{
		RemoveAgentFromChild(pAgent);
		--m_AgentCount;

		//if total agents <= max
		if (!m_IsRoot && m_AgentCount < m_MaxAgentCount)
		{
			//combine children into self
			//all child nodes won't have more children since they would have merged from a previous RemoveAgentFromChild() call
			int counter{};
			for (QuadTreeNode* pNode : m_ChildNodes)
			{
				for (int i{}; i < pNode->GetAgentCount(); ++i)
				{
					m_Agents[counter] = pNode->GetAgents()[i];
					++counter;
				}
			}

			for (size_t i{}; i < m_ChildNodes.size(); ++i)
			{
				m_ChildNodes[i] = nullptr;
			}
		}
		return;
	}

	--m_AgentCount;
	std::replace(m_Agents.begin(), m_Agents.end(), pAgent, m_Agents[m_AgentCount]);
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
	
	pAgent->SetCell(this);

	++m_AgentCount;

	//subdivide node
	if (m_AgentCount > m_MaxAgentCount) //dont divide if size of node is too small
	{
		//create child nodes
		const float halfCellWidth{ (m_MaxBounds.x - m_MinBounds.x) / 2 };
		const float halfCellHeight{ (m_MaxBounds.y - m_MinBounds.y) / 2 };
		m_ChildNodes[0] = new QuadTreeNode{ {m_MinBounds.x, m_MinBounds.y},{m_MinBounds.x + halfCellWidth, m_MinBounds.y + halfCellHeight}, false };
		m_ChildNodes[1] = new QuadTreeNode{ {m_MinBounds.x + halfCellWidth, m_MinBounds.y},{m_MaxBounds.x, m_MinBounds.y + halfCellHeight}, false };
		m_ChildNodes[2] = new QuadTreeNode{ {m_MinBounds.x + halfCellWidth, m_MinBounds.y + halfCellHeight},{m_MaxBounds.x, m_MaxBounds.y}, false };
		m_ChildNodes[3] = new QuadTreeNode{ {m_MinBounds.x, m_MinBounds.y + halfCellHeight},{m_MinBounds.x + halfCellWidth, m_MaxBounds.y}, false };

		//add agents to children
		for (int i{}; i < m_AgentCount; ++i)
		{
			AddAgentToChild(m_Agents[i]);
		}
	}
}

void QuadTreeNode::AddAgentToChild(AgentBase* pAgent)
{
	const Elite::Vector2& pos{ pAgent->GetPosition() };
	for (QuadTreeNode* pNode : m_ChildNodes)
	{
		if (pos.x >= pNode->GetMinBounds().x && pos.x < pNode->GetMaxBounds().x && pos.y >= pNode->GetMinBounds().y && pos.y < pNode->GetMaxBounds().y)
		{
			pNode->AddAgent(pAgent);
			return;
		}
	}
	std::cerr << "agent doesnt fit any nodes\n";
}

void QuadTreeNode::RemoveAgentFromChild(AgentBase* pAgent)
{
	const Elite::Vector2& pos{ pAgent->GetPosition() };
	for (QuadTreeNode* pNode : m_ChildNodes)
	{
		if (pos.x >= pNode->GetMinBounds().x && pos.x < pNode->GetMaxBounds().x && pos.y >= pNode->GetMinBounds().y && pos.y < pNode->GetMaxBounds().y)
		{
			pNode->RemoveAgent(pAgent);
			return;
		}
	}
	std::cerr << "agent doesnt fit any nodes\n";
}
