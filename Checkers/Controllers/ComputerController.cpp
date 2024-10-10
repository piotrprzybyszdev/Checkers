#include "ComputerController.h"

namespace Checkers
{

void ComputerController::OnClick(float x, float y)
{
}

Position ComputerController::MakeMove(Position position)
{
	m_Cancelled = false;

	return m_Tree.FindBestMove(position, m_Cancelled);
}

void ComputerController::CancelMove()
{
	if (m_Cancelled) return;

	m_Cancelled = true;
}

}
