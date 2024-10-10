#pragma once

#include <future>

#include "Renderer/Renderer.h"

#include "Controller.h"

namespace Checkers
{

class PlayerController : public Controller
{
public:
	PlayerController() : Controller(ControllerType::PlayerController) {}
	~PlayerController() override {}

	void OnClick(float x, float y) override;
	Position MakeMove(Position position) override;
	void CancelMove() override;

private:
	bool m_Working = false;

	Position m_SavedPosition = Position();
	Position m_Position = Position();
	std::promise<Position> m_Promise = std::promise<Position>();

	int m_SelectedIndex = -1;
	Bitboard m_SelectedMoves = Board::Empty;
	bool m_Capture = false;

	void SelectChecker(int index);
};

}
