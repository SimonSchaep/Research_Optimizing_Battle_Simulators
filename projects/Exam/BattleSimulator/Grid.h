#pragma once
class AgentBase;
class AgentBasePooler;
class Cell;

class Grid final
{
public:
	Grid(int rows, int cols, float cellSize);
	~Grid();

	Grid(const Grid& other) = delete;
	Grid& operator=(const Grid& other) = delete;
	Grid(Grid&& other) = delete;
	Grid& operator=(Grid&& other) = delete;

	void Update(float dt, AgentBasePooler* pAgentBasePooler, bool isUsingMultithreading);
	void Render()const;

	const std::vector<Cell*>& GetCells()const { return m_CellPointers; };

	int GetCellId(int row, int col)const { return min(max( row, 0), m_Rows - 1) * m_Cols + min(max(col, 0), m_Cols - 1); };
	int GetCellId(const Elite::Vector2& position)const { return min(max(GetCellId(int(position.y / m_CellSize), int(position.x / m_CellSize)), 0), (m_Rows * m_Cols) - 1); };

	void GetRowCol(int cellId, int& row, int& col)const;

private:
	std::vector<Cell*> m_CellPointers{};

	int m_Rows{};
	int m_Cols{};

	float m_CellSize{};

};

