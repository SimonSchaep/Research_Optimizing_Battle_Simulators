#pragma once
#include "framework/EliteAI/EliteNavigation/ENavigation.h"

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		std::vector<T_NodeType*> path;
		std::vector<NodeRecord> openList;
		std::vector<NodeRecord> closedList;
		NodeRecord currentNodeRecord;

		currentNodeRecord.pNode = pStartNode;
		currentNodeRecord.pConnection = nullptr;
		currentNodeRecord.costSoFar = 0;
		currentNodeRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);

		openList.push_back(currentNodeRecord);

		while (!openList.empty())
		{
			currentNodeRecord = *std::min_element(openList.begin(), openList.end());

			if (currentNodeRecord.pConnection && currentNodeRecord.pConnection->GetTo() == 87)
			{
				int i{};
			}

			if (currentNodeRecord.pNode == pGoalNode)
			{

				break;
			}

			for (const auto& connection : m_pGraph->GetNodeConnections(currentNodeRecord.pNode))
			{
				const float totalCost = currentNodeRecord.costSoFar + connection->GetCost();

				bool foundInClosedList{};

				bool skipConnection{};

				for (const auto& nodeRecord : closedList)
				{
					if (nodeRecord.pNode == m_pGraph->GetNode(connection->GetTo()))
					{
						if (nodeRecord.costSoFar >= totalCost)
						{
							closedList.erase(std::remove(closedList.begin(), closedList.end(), nodeRecord));
							foundInClosedList = true;
						}
						else
						{
							skipConnection = true;
							break;
						}
					}
				}

				if (!foundInClosedList)
				{
					for (const auto& nodeRecord : openList)
					{
						if (nodeRecord.pNode == m_pGraph->GetNode(connection->GetTo()))
						{
							if (nodeRecord.costSoFar >= totalCost)
							{
								openList.erase(std::remove(openList.begin(), openList.end(), nodeRecord));
							}
							else
							{
								skipConnection = true;
								break;
							}
						}
					}
				}	

				if (!skipConnection)
				{
					NodeRecord newNodeRecord{};
					newNodeRecord.pNode = m_pGraph->GetNode(connection->GetTo());
					newNodeRecord.costSoFar = totalCost;
					newNodeRecord.estimatedTotalCost = totalCost + GetHeuristicCost(m_pGraph->GetNode(connection->GetTo()), pGoalNode);
					newNodeRecord.pConnection = connection;
					if (foundInClosedList)
					{
						closedList.push_back(newNodeRecord);
					}
					else
					{
						openList.push_back(newNodeRecord);
					}					
				}

				
			}

			openList.erase(std::remove(openList.begin(), openList.end(), currentNodeRecord));
			closedList.push_back(currentNodeRecord);
		}

		if (openList.empty())
		{
			return path;
		}

		while (currentNodeRecord.pNode != pStartNode)
		{
			if (path.size() > 1000)
			{
				break;
			}
			path.push_back(currentNodeRecord.pNode);
			
			for (auto& nodeRecord : closedList)
			{
				if (nodeRecord.pNode == m_pGraph->GetNode(currentNodeRecord.pConnection->GetFrom()))
				{
					currentNodeRecord = nodeRecord;
					break;
				}
			}
		}

		path.push_back(pStartNode);
		std::reverse(path.begin(), path.end());

		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}