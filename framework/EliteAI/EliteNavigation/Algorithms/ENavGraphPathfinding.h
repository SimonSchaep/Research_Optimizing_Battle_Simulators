#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

namespace Elite
{
	class NavMeshPathfinding
	{
	public:
		static std::vector<Vector2> FindPath(Vector2 startPos, Vector2 endPos, NavGraph* pNavGraph, std::vector<Vector2>& debugNodePositions, std::vector<Portal>& debugPortals)
		{
			//Create the path to return
			std::vector<Vector2> finalPath{};

			//Get the start and endTriangle
			const Triangle* pStartTriangle{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos) };
			const Triangle* pEndTriangle{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos) };

			if (!pStartTriangle || !pEndTriangle)
			{
				return finalPath;
			}
			if (pStartTriangle == pEndTriangle)
			{
				finalPath.push_back(endPos);
				return finalPath;
			}
		
			//We have valid start/end triangles and they are not the same
			//=> Start looking for a path
			//Copy the graph
			auto clonedGraph{ pNavGraph->Clone()};

			//Create extra node for the Start Node (Agent's position
			NavGraphNode* pStartNode{ new NavGraphNode{int(clonedGraph->GetAllNodes().size()), -1, startPos} };
			clonedGraph->AddNode(pStartNode);
			for (int lineIndex : pStartTriangle->metaData.IndexLines)
			{
				for (auto node : clonedGraph->GetAllNodes())
				{
					if (node->GetLineIndex() == lineIndex)
					{
						//Add connection
						GraphConnection2D* pConnection{ new GraphConnection2D{pStartNode->GetIndex(), node->GetIndex()}};
						pConnection->SetCost(Distance(startPos, clonedGraph->GetNodePos(node)));
						clonedGraph->AddConnection(pConnection);
						break;
					}
				}
			}			

			//Create extra node for the endNode
			NavGraphNode* pEndNode{ new NavGraphNode{int(clonedGraph->GetAllNodes().size()), -1, endPos} };
			clonedGraph->AddNode(pEndNode);
			for (int lineIndex : pEndTriangle->metaData.IndexLines)
			{
				for (auto node : clonedGraph->GetAllNodes())
				{
					if (node->GetLineIndex() == lineIndex)
					{
						//Add connection
						GraphConnection2D* pConnection{ new GraphConnection2D{pEndNode->GetIndex(), node->GetIndex()} };
						pConnection->SetCost(Distance(endPos, node->GetPosition()));
						clonedGraph->AddConnection(pConnection);
						break;
					}
				}
			}


			//Run A star on new graph
			auto pathfinder = AStar<NavGraphNode, GraphConnection2D>(clonedGraph.get(), Elite::HeuristicFunctions::Euclidean);
			auto nodePath = pathfinder.FindPath(pStartNode, pEndNode);
			//OPTIONAL BUT ADVICED: Debug Visualisation
			debugNodePositions.clear();
			for (auto node : nodePath)
			{
				debugNodePositions.push_back(node->GetPosition());
			}

			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			debugPortals = SSFA::FindPortals(nodePath, pNavGraph->GetNavMeshPolygon()); //debug portals not only used for debugging purposes
			finalPath = SSFA::OptimizePortals(debugPortals);

			return finalPath;
		}
	};
}
