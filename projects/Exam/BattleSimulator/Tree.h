#pragma once
class AgentBase;
class Node;

class Tree
{
public:
	Tree(int rows, int cols, float cellSize);
	~Tree();

	Tree(const Tree& other) = delete;
	Tree& operator=(const Tree& other) = delete;
	Tree(Tree&& other) = delete;
	Tree& operator=(Tree&& other) = delete;

	void Render()const;

	const std::vector<Node*>& GetCells()const { return m_NodePointers; };

	int GetCellId(int row, int col)const { return min(max( row, 0), m_Rows - 1) * m_Cols + min(max(col, 0), m_Cols - 1); };
	int GetCellId(const Elite::Vector2& position)const { return min(max(GetCellId(int(position.y / m_CellSize), int(position.x / m_CellSize)), 0), (m_Rows * m_Cols) - 1); };

	void GetRowCol(int cellId, int& row, int& col)const;

private:
	std::vector<Node*> m_NodePointers{};

	int m_Rows{};
	int m_Cols{};

	float m_CellSize{};

};

