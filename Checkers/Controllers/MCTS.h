#include "Simulator.h"
#include "Position.h"

#include <vector>

namespace Checkers
{

using node_index = uint32_t;

struct Node
{
	Position Position;
	node_index Child;
	node_index Next;
	uint32_t Visits;
	uint32_t Wins;
};

static_assert(std::is_standard_layout_v<Node> == true);

class Tree
{
public:
	static constexpr float DefaultExplorationConstant = 1.41421356f;
	
	Tree(Simulator *simulator, unsigned int maxIterations, std::chrono::milliseconds maxTime, unsigned int selectCount, float explorationConstant = DefaultExplorationConstant, float virtualLoss = 0.01f);
	~Tree();

	Position FindBestMove(Position position, const int &cancelled);
	void Print(node_index idx = 0, node_index par = -1, int h = 0, int maxh = 2);

private:
	static constexpr size_t StartNodeCount = 250000;

	Simulator *m_Simulator;
	unsigned int m_MaxIterations;
	std::chrono::milliseconds m_MaxTime;
	float m_ExplorationContant;
	unsigned int m_MaxSelectedCount;
	float m_VirtualLossIncrement;

	std::vector<Node> m_Nodes = {};
	std::vector<float> m_VirtualLoss = {};
	std::vector<Position> m_Selected = {};
	std::vector<std::vector<node_index>> m_Paths = {};

	node_index SelectNode();
	void Expand(node_index index);

	void AddChildNodes(node_index index);
	void AddCaptures(int fromIndex, Position position);

	void BackPropagate(const std::vector<int> &blackInc, const std::vector<int> &whiteInc, const std::vector<int> &visitsInc);

	Position GetBestMove();

	float GetNodeScore(node_index index);
};

}