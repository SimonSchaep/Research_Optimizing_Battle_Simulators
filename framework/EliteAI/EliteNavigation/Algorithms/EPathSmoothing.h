#pragma once

#include <vector>
#include "framework/EliteGeometry/EGeometry2DTypes.h"
#include "framework/EliteAI/EliteGraphs/EGraphNodeTypes.h"

namespace Elite
{
	//Portal struct (only contains line info atm, you can expand this if needed)
	struct Portal
	{
		Portal() {}
		explicit Portal(const Elite::Line& line) :
			Line(line)
		{
		}
		Elite::Line Line = {};
	};


	class SSFA final
	{
	public:
		//=== SSFA Functions ===
		//--- References ---
		//http://digestingduck.blogspot.be/2010/03/simple-stupid-funnel-algorithm.html
		//https://gamedev.stackexchange.com/questions/68302/how-does-the-simple-stupid-funnel-algorithm-work
		static std::vector<Portal> FindPortals(
			const std::vector<NavGraphNode*>& nodePath,
			Polygon* navMeshPolygon)
		{
			//Container
			std::vector<Portal> vPortals = {};

			vPortals.push_back(Portal(Line(nodePath[0]->GetPosition(), nodePath[0]->GetPosition())));

			std::vector<Line*> lines = navMeshPolygon->GetLines();

			//For each node received, get its corresponding line
			for (size_t nodeIdx = 1; nodeIdx < nodePath.size() - 1; ++nodeIdx)
			{
				//Local variables
				NavGraphNode* pNode = nodePath[nodeIdx]; //Store node, except last node, because this is our target node!
				Line* pLine = lines[pNode->GetLineIndex()];

				//Redetermine it's "orientation" based on the required path (left-right vs right-left) - p1 should be right point
				Vector2 centerLine = (pLine->p1 + pLine->p2) / 2.0f;
				Vector2 previousPosition = nodeIdx == 0 ? nodePath[0]->GetPosition() : nodePath[nodeIdx - 1]->GetPosition();

				float cross = Cross((centerLine - previousPosition), (pLine->p1 - previousPosition));

				Line portalLine = {};
				if (cross > 0)//Left
					portalLine = Line(pLine->p2, pLine->p1);
				else //Right
					portalLine = Line(pLine->p1, pLine->p2);

				//Store portal
				vPortals.push_back(Portal(portalLine));
			}
			//Add degenerate portal to force end evaluation
			vPortals.push_back(Portal(Line(nodePath[nodePath.size() - 1]->GetPosition(), nodePath[nodePath.size() - 1]->GetPosition())));

			return vPortals;
		}

		static std::vector<Vector2> OptimizePortals(const std::vector<Portal>& portals)
		{
			//P1 == right point of portal, P2 == left point of portal
			std::vector<Vector2> vPath = {};
			const unsigned int amtPortals{ static_cast<unsigned int>(portals.size()) };

			int leftLegIdx{ 1 }, rightLegIdx{ 1 };

			Vector2 apexPos = portals[0].Line.p1;
			Vector2 rightLeg = portals[rightLegIdx].Line.p1 - apexPos;
			Vector2 leftLeg = portals[leftLegIdx].Line.p2 - apexPos;

			for (unsigned int portalIdx = 2; portalIdx < amtPortals; ++portalIdx)
			{
				//Local
				const auto& portal = portals[portalIdx];

				const float rightLegMagnitude{ rightLeg.Magnitude() };
				const float leftLegMagnitude{ leftLeg.Magnitude() };

				float currentAngle{ acosf(rightLeg.Dot(leftLeg) / (rightLegMagnitude * leftLegMagnitude)) };

				//--- RIGHT CHECK ---
				//Calculate new leg
				Vector2 newRightLeg = portal.Line.p1 - apexPos;
				float newAngle{ acosf(newRightLeg.Dot(leftLeg) / (newRightLeg.Magnitude() * leftLegMagnitude)) };
				//See if moving funnel inwards
				if (newAngle <= currentAngle || leftLegMagnitude == 0 || rightLegMagnitude == 0)
				{
					//Check if legs cross
					if (newRightLeg.Cross(leftLeg) < 0)
					{
						apexPos += leftLeg;
						portalIdx = leftLegIdx++;
						rightLegIdx = portalIdx;

						vPath.push_back(apexPos);

						if (portalIdx < amtPortals)
						{
							rightLeg = portals[rightLegIdx].Line.p1 - apexPos;
							leftLeg = portals[leftLegIdx].Line.p2 - apexPos;							
						}
						continue;
					}
					else
					{
						//Update leg
						rightLeg = newRightLeg;
						rightLegIdx = portalIdx;
					}
				}
				//Check if legs cross
				else if (newRightLeg.Cross(leftLeg) < 0)
				{
					apexPos += leftLeg;
					portalIdx = leftLegIdx++;
					rightLegIdx = portalIdx;

					vPath.push_back(apexPos);

					if (portalIdx < amtPortals)
					{
						rightLeg = portals[rightLegIdx].Line.p1 - apexPos;
						leftLeg = portals[leftLegIdx].Line.p2 - apexPos;
					}
					continue;
				}
				currentAngle = acosf(rightLeg.Dot(leftLeg) / (rightLeg.Magnitude() * leftLegMagnitude));

				//--- LEFT CHECK ---
				//Calculate new leg
				Vector2 newLeftLeg = portal.Line.p2 - apexPos;
				newAngle = acosf(rightLeg.Dot(newLeftLeg) / (rightLeg.Magnitude() * newLeftLeg.Magnitude()));
				//See if moving funnel inwards
				if (newAngle <= currentAngle || leftLegMagnitude == 0 || rightLegMagnitude == 0)
				{
					//Check if legs cross
					if (rightLeg.Cross(newLeftLeg) < 0)
					{
						apexPos += rightLeg;
						portalIdx = rightLegIdx++;
						leftLegIdx = portalIdx;

						vPath.push_back(apexPos);

						if (portalIdx < amtPortals)
						{
							rightLeg = portals[rightLegIdx].Line.p1 - apexPos;
							leftLeg = portals[leftLegIdx].Line.p2 - apexPos;
						}
						continue;
					}
					else
					{
						//Update leg
						leftLeg = newLeftLeg;
						leftLegIdx = portalIdx;
					}
				}
				//Check if legs cross
				else if (rightLeg.Cross(newLeftLeg) < 0)
				{
					apexPos += rightLeg;
					portalIdx = rightLegIdx++;
					leftLegIdx = portalIdx;

					vPath.push_back(apexPos);

					if (portalIdx < amtPortals)
					{
						rightLeg = portals[rightLegIdx].Line.p1 - apexPos;
						leftLeg = portals[leftLegIdx].Line.p2 - apexPos;
					}
					continue;
				}
			}

			// Add last path point (You can use the last portal p1 or p2 points as both are equal to the endPoint of the path
			vPath.push_back(portals[amtPortals - 1].Line.p1);

			return vPath;
		}
	private:
		SSFA() {};
		~SSFA() {};
	};
}
