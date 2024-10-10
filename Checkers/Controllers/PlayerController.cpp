#include "PlayerController.h"

namespace Checkers
{

void PlayerController::OnClick(float x, float y)
{
	Renderer::ClearHighlights();

	int i = x * 8.0f, j = y * 8.0f;

	if (Board::IsWhiteSquare(i, j))
	{
		m_SelectedIndex = -1;
		m_Position = m_SavedPosition;
		return;
	}

	const int toIndex = Board::CoordsToIndex(i, j);
	const Bitboard to = Board::FromCoords(i, j);

	if (m_SelectedIndex != -1 && Board::HasBit(m_SelectedMoves, toIndex))
	{
		if (!m_Capture)
			m_Position.Move(m_SelectedIndex, toIndex);
		else
			m_Position.Capture(m_SelectedIndex, toIndex);

		Bitboard captures = m_Position.GetCaptures(to);

		if (m_Capture && captures)
		{
			// player has to continue capturing with this checker

			Renderer::SelectTile(i, j);
			SelectChecker(toIndex);

			return;
		}

		// move end
		m_SelectedIndex = -1;
		m_Working = false;
		m_Position.EndTurn();
		m_Promise.set_value(m_Position);

		return;
	}

	// clicked on tile with no checkers
	if (Board::IsEmpty((m_Position.GetCheckers() & to)))
	{
		m_SelectedIndex = -1;
		m_Position = m_SavedPosition;
		return;
	}

	Renderer::SelectTile(i, j);
	SelectChecker(Board::CoordsToIndex(i, j));
}

Position PlayerController::MakeMove(Position position)
{
	m_Promise = std::promise<Position>();
	m_SavedPosition = position;
	m_Position = position;
	m_Working = true;
	return m_Promise.get_future().get();
}

void PlayerController::CancelMove()
{
	if (!m_Working) return;

	Renderer::ClearHighlights();
	m_Promise.set_value(Position());
}

void PlayerController::SelectChecker(int index)
{
	Bitboard from = Board::FromIndex(index);
	Bitboard choices = m_Position.GetCaptures(from);
	m_Capture = true;

	if (Board::IsEmpty(choices))
	{
		bool hasCaptures = m_Position.GetAllCapturing();
		if (hasCaptures)
		{
			// there are captures in this m_Position but the selected checker can't make a capture
			// than the player can't make a move with it (it doesn't become selected)
			m_SelectedIndex = -1;
			return;
		}

		m_Capture = false;
		choices = m_Position.GetMoves(from);
	}

	m_SelectedIndex = index;
	m_SelectedMoves = choices;

	int bits[16];
	int count = Board::GetBits(choices, bits);

	for (int k = 0; k < count; k++)
	{
		int i, j;
		Board::IndexToCoords(bits[k], i, j);
		Renderer::HighlightTile(i, j);
	}
}

}