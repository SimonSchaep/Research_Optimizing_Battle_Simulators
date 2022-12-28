//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "Grid.h"
#include "AgentBase.h"
#include "Cell.h"

Grid::Grid(int rows, int cols, float cellSize)
	: m_Rows{rows}, m_Cols{cols}, m_CellSize{cellSize}
{
	m_CellPointers.reserve(rows * cols);
	for (int r{}; r < rows; ++r)
	{
		for (int c{}; c < cols; ++c)
		{
			m_CellPointers.push_back(new Cell{GetCellId(r,c)});
		}
	}
}

Grid::~Grid()
{
	for (Cell* pCell : m_CellPointers)
	{
		SAFE_DELETE(pCell);
	}
}

void Grid::Update(float dt, AgentBasePooler* pAgentBasePooler)
{
	for (Cell* pCell : m_CellPointers)
	{
		pCell->Update(dt, pAgentBasePooler);
	}
}

void Grid::Render() const
{
	std::vector<Elite::Vector2> points{ 4 };
	
	for (int r{}; r < m_Rows; ++r)
	{
		for (int c{}; c < m_Cols; ++c)
		{
			points[0] = { c * m_CellSize, r * m_CellSize };
			points[1] = { c * m_CellSize, (r + 1) * m_CellSize };
			points[2] = { (c + 1) * m_CellSize, (r + 1) * m_CellSize };
			points[3] = { (c + 1) * m_CellSize, r * m_CellSize };
			DEBUGRENDERER2D->DrawPolygon(&points[0], 4, {1,0,0}, -1);
			DEBUGRENDERER2D->DrawString({ points[0].x + m_CellSize/2, points[0].y + m_CellSize/2}, std::to_string(m_CellPointers[GetCellId(r,c)]->GetAgentCount()).c_str());
		}
	}
}

void Grid::GetRowCol(int cellId, int& row, int& col) const
{
	row = cellId / m_Cols;
	col = cellId % m_Cols;
}
