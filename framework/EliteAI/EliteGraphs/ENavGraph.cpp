#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	/*for (auto& node : m_Nodes)
	{
		SAFE_DELETE(node);
	}
	for (auto& connectionList : m_Connections)
	{
		for (auto& connection : connectionList)
		{
			SAFE_DELETE(connection);
		}		
	}*/

	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes
	int i{};
	for (auto& line : m_pNavMeshPolygon->GetLines())
	{
		if (m_pNavMeshPolygon->GetTrianglesFromLineIndex(line->index).size() > 1)
		{
			NavGraphNode* pNode{ new NavGraphNode{i, line->index, (line->p1 + line->p2) / 2 }}; //line->index doesnt work			
			AddNode(pNode);
			++i;
		}
	}

	//2. Create connections now that every node is created
	for (auto& triangle : m_pNavMeshPolygon->GetTriangles())
	{
		std::vector<int> nodeIndexes{};
		for (int lineIndex : triangle->metaData.IndexLines)
		{
			for (auto& node : m_Nodes)
			{
				if (lineIndex == node->GetLineIndex())
				{
					nodeIndexes.push_back(node->GetIndex());
					break;
				}
			}
		}
		if (nodeIndexes.size() == 2)
		{
			GraphConnection2D* pConnection{ new GraphConnection2D{nodeIndexes[0], nodeIndexes[1]}};
			//setting cost here cause more efficient
			pConnection->SetCost(Distance(GetNodePos(nodeIndexes[0]), GetNodePos(nodeIndexes[1])));
			AddConnection(pConnection);
		}
		else if (nodeIndexes.size() == 3)
		{
			for (int i{}; i < 3; ++i)
			{
				GraphConnection2D* pConnection{ new GraphConnection2D{nodeIndexes[i], nodeIndexes[(i + 1) % 3]}};
				//setting cost here cause more efficient
				pConnection->SetCost(Distance(GetNodePos(nodeIndexes[i]), GetNodePos(nodeIndexes[(i + 1) % 3])));
				AddConnection(pConnection);
			}
		}
	}	
	//3. Set the connections cost to the actual distance
}

