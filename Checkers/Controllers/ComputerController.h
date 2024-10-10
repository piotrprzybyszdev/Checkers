#pragma once

#include <bit>
#include <cassert>
#include <chrono>
#include <memory>
#include <thread>

#include "Core/Core.h"
#include "Controller.h"
#include "MCTS.h"

namespace Checkers
{

class ComputerController : public Controller
{
public:
	ComputerController(ControllerType type, Simulator *simulator, unsigned int iterationCount, std::chrono::milliseconds maxTime, unsigned int selectedCount = 1, float explorationConstant = Tree::DefaultExplorationConstant, float virtualLoss = 0.01f)
		: Controller(type), m_Tree(simulator, iterationCount, maxTime, selectedCount, explorationConstant, virtualLoss)
	{
	}
	~ComputerController() override {}

	void OnClick(float x, float y) override;
	Position MakeMove(Position position) override;
	void CancelMove() override;

private:
	Tree m_Tree;

	bool m_Cancelled = false;
};

}
