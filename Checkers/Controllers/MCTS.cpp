#include <cmath>
#include <iostream>

#include "Core/Core.h"

#include "MCTS.h"

namespace Checkers
{

Tree::Tree(Simulator *simulator, unsigned int maxIterations, std::chrono::milliseconds maxTime, unsigned int selectCount, float explorationConstant, float virtualLoss)
	: m_Simulator(simulator), m_MaxIterations(maxIterations),
	m_ExplorationContant(explorationConstant), m_MaxSelectedCount(selectCount),
	m_VirtualLossIncrement(virtualLoss), m_MaxTime(maxTime - std::chrono::milliseconds(1))
{
	m_Nodes.reserve(StartNodeCount);
	m_VirtualLoss.reserve(StartNodeCount);
}

Tree::~Tree()
{
}

Position Tree::FindBestMove(Position position, const int &cancelled)
{
	Timer timer("MCTS Total");

	m_Nodes.clear();
	m_VirtualLoss.clear();
	m_Nodes.emplace_back(Node{
		.Position = position
	});
	m_VirtualLoss.emplace_back(0.0f);

	std::chrono::time_point start(std::chrono::high_resolution_clock::now());
	for (int i = 0; i < m_MaxIterations; i++)
	{
		if (cancelled)
			return Position();

		std::chrono::time_point now(std::chrono::high_resolution_clock::now());

		if (now - start > m_MaxTime)
			break;

		m_Paths.clear();
		m_Selected.clear();

		int pathCount = 0;
		while (m_Selected.size() < m_MaxSelectedCount)
		{
			node_index index;
			{
				Timer timer("MCTS Selection");
				index = SelectNode();
			}

			if (index == 0 && m_Nodes[0].Child != 0)
				break;

			{
				Timer timer("MCTS Expansion");
				Expand(index);
			}

			for (int j = pathCount; j < m_Paths.size(); j++)
				for (node_index index : m_Paths[j])
					m_VirtualLoss[index] += m_VirtualLossIncrement;

			pathCount = m_Paths.size();
		}

		std::vector<int> blackInc(m_Paths.size()), whiteInc(m_Paths.size()), visitsInc(m_Paths.size());

		{
			Timer timer("MCTS Simulation");
			m_Simulator->Simulate(m_Selected, blackInc, whiteInc, visitsInc);
		}

		{
			Timer timer("MCTS BackPropagation");
			BackPropagate(blackInc, whiteInc, visitsInc);
		}
	}

	return GetBestMove();
}

Position Tree::GetBestMove()
{
	uint32_t maxVisits = 0;
	node_index maxIndex = 0;
	for (node_index childIndex = m_Nodes[0].Child; childIndex != 0; childIndex = m_Nodes[childIndex].Next)
		if (m_Nodes[childIndex].Visits > maxVisits)
		{
			maxVisits = m_Nodes[childIndex].Visits;
			maxIndex = childIndex;
		}

	const std::string color = m_Nodes[0].Position.BlackTurn ? "Black" : "White";
	Stats::AddStat(std::format("{} NodeCount", color), "{} Node Count: {} / {}", color, m_Nodes.size(), m_Nodes.capacity());
	Stats::AddStat(std::format("{} Simulations", color), "{} Total Simulations: {:.3e}", color, m_Nodes[0].Visits / 2.0f);
	Stats::AddStat(std::format("{} Winrate", color), "{} Win Rate: {:.3f} %%", color, (m_Nodes[maxIndex].Wins / (float)m_Nodes[maxIndex].Visits) * 100);

	return m_Nodes[maxIndex].Position;
}

float Tree::GetNodeScore(node_index index)
{
	const Node &node = m_Nodes[index];

	const float totalVisits = m_Nodes[0].Visits;
	const float visits = node.Visits == 0 ? 1.0f : (float)node.Visits;
	float winrate = node.Wins / visits;

	float score = winrate + m_ExplorationContant * std::sqrt(std::log(totalVisits) / visits);
	return score;
}

void Tree::Print(node_index idx, node_index par, int h, int maxh)
{
	const Node &node = m_Nodes[idx];

	const float visits = node.Visits;
	float winrate = node.Wins / visits;
	const float totalVisits = m_Nodes[0].Visits;

	float score = winrate + m_ExplorationContant * std::sqrt(std::log(totalVisits) / visits);

	std::cout << "par: " << par << ", idx: " << idx << ", next: " << m_Nodes[idx].Next
		<< ", child: " << m_Nodes[idx].Child << std::hex << ", pos: " << node.Position.Black << " "
		<< node.Position.White << std::dec << " (" << node.Wins << ", " << node.Visits << ")"
		<< "wr: " << winrate << " score: " << score << std::endl;

	if (h == maxh) return;

	for (node_index childIndex = node.Child; childIndex != 0; childIndex = m_Nodes[childIndex].Next)
		Print(childIndex, idx, h + 1, maxh);
}

node_index Tree::SelectNode()
{
	m_Paths.push_back({});

	node_index nodeIndex = 0;

	while (m_Nodes[nodeIndex].Child != 0)
	{
		const Node &node = m_Nodes[nodeIndex];

		m_Paths.back().push_back(nodeIndex);

		const float totalVisits = node.Visits;
		float maxScore = -FLT_MAX;

		for (node_index childIndex = node.Child; childIndex != 0; childIndex = m_Nodes[childIndex].Next)
		{
			const Node &child = m_Nodes[childIndex];

			const float visits = child.Visits == 0 ? 1.0f : (float)child.Visits;
			float winrate = child.Wins / visits;

			float score = winrate + m_ExplorationContant * std::sqrt(std::log(totalVisits) / visits);
			score -= m_VirtualLoss[childIndex];

			if (score > maxScore)
			{
				maxScore = score;
				nodeIndex = childIndex;
			}
		}
	}

	return nodeIndex;
}

void Tree::Expand(node_index index)
{
	m_Paths.back().push_back(index);
	if (m_Nodes[index].Visits == 0 || m_Nodes[index].Child != 0 || m_Nodes[index].Position.HasLost() || m_Nodes[index].Position.IsDraw())
	{
		m_Selected.push_back(m_Nodes[index].Position);
		return;
	}

	m_Nodes[index].Child = m_Nodes.size();
	AddChildNodes(index);
	m_Nodes.back().Next = 0;

	node_index child = m_Nodes[index].Child;
	m_Paths.back().push_back(child);
	m_Selected.push_back(m_Nodes[child].Position);
	child = m_Nodes[child].Next;

	while (child != 0 && m_Selected.size() < m_MaxSelectedCount)
	{
		m_Paths.push_back(m_Paths.back());
		m_Paths.back().pop_back();
		m_Paths.back().push_back(child);
		m_Selected.push_back(m_Nodes[child].Position);

		child = m_Nodes[child].Next;
	}
}

void Tree::AddChildNodes(node_index index)
{
	Bitboard capturing = m_Nodes[index].Position.GetAllCapturing();

	if (capturing)
	{
		int choices[12];
		int choiceCnt = Board::GetBits(capturing, choices);

		for (int choiceIdx = 0; choiceIdx < choiceCnt; choiceIdx++)
			AddCaptures(choices[choiceIdx], m_Nodes[index].Position);

		return;
	}

	Bitboard moving = m_Nodes[index].Position.GetAllMoving();
	assert(!Board::IsEmpty(moving));

	int fromChoices[12];
	int fromChoiceCount = Board::GetBits(moving, fromChoices);
	for (int i = 0; i < fromChoiceCount; i++)
	{
		const int fromIndex = fromChoices[i];

		Bitboard moves = m_Nodes[index].Position.GetMoves(Board::FromIndex(fromIndex));
		assert(!Board::IsEmpty(moves));

		int toChoices[16];
		int toChoiceCount = Board::GetBits(moves, toChoices);
		for (int j = 0; j < toChoiceCount; j++)
		{
			const int toIndex = toChoices[j];

			Position next = m_Nodes[index].Position;
		
			next.Move(fromIndex, toIndex);
			next.EndTurn();

			m_Nodes.emplace_back(Node{
				.Position = next,
				.Next = (node_index)(m_Nodes.size() + 1),
			});
			m_VirtualLoss.emplace_back(0.0f);
		}
	}
}

void Tree::AddCaptures(int fromIndex, Position position)
{
	Bitboard captures = position.GetCaptures(Board::FromIndex(fromIndex));

	if (Board::IsEmpty(captures))
	{
		position.EndTurn();
		m_Nodes.emplace_back(Node{
			.Position = position,
			.Next = (node_index)(m_Nodes.size() + 1),
		});
		m_VirtualLoss.emplace_back(0.0f);

		return;
	}

	int choices[8];
	int choiceCnt = Board::GetBits(captures, choices);

	for (int choiceIdx = 0; choiceIdx < choiceCnt; choiceIdx++)
	{
		int toIndex = choices[choiceIdx];
		Position next = position;
		next.Capture(fromIndex, toIndex);

		AddCaptures(toIndex, next);
	}
}

void Tree::BackPropagate(const std::vector<int> &blackInc, const std::vector<int> &whiteInc, const std::vector<int> &visitsInc)
{
	for (int i = 0; i < m_Paths.size(); i++)
	{
		for (node_index index : m_Paths[i])
		{
			Node &node = m_Nodes[index];
			node.Visits += visitsInc[i];
			if (!node.Position.BlackTurn)
				node.Wins += blackInc[i];
			else
				node.Wins += whiteInc[i];
			m_VirtualLoss[index] = 0.0f;
		}
	}
}

}
