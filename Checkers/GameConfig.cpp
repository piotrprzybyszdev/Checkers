#include <fstream>
#include <iostream>

#include "Controllers/PlayerController.h"
#include "Controllers/ComputerController.h"

#include "GraphicsCardConfig.h"
#include "GameConfig.h"

namespace Checkers
{

Controller *ReadPlayer(std::string color, std::chrono::seconds time)
{
	char c;
	do {
		std::cout << "Who should play " << color << " (Cpu, Gpu or Human)? [C/G/H] ";
		std::cin >> c;
	} while (c != 'c' && c != 'C' && c != 'g' && c != 'G' && c != 'h' && c != 'H');

	switch (c)
	{
	case 'c':
	case 'C':
		return new ComputerController(ControllerType::ComputerHostController, Simulator::CreateHost(), 1e9, time);
	case 'g':
	case 'G':
	{
		int cores = GetSmCount() * GetThreadsPerSm();
		int threadsPerBlock = 64;
		int blocks = cores / threadsPerBlock * 2;
		return new ComputerController(ControllerType::ComputerDeviceController, Simulator::CreateDevice(blocks, threadsPerBlock), 1e9, time, blocks);
	}
	case 'h':
	case 'H':
		return new ConsoleController();
	}

	return new ConsoleController();
}

void NotationToCoords(int &i, int &j, char col, char row)
{
	i = 8 - (row - '1');
	j = 8 - (col - 'a');
}

void IndexToNotation(int index, char &col, char &row)
{
	int i, j;
	Board::IndexToCoords(index, i, j);

	row = (7 - j) + '1';
	col = (7 - i) + 'a';
}

static std::filesystem::path s_Path;

void SetFilePath(std::filesystem::path path)
{
	s_Path = path;
}

void PrintIndex(int index)
{
	char col, row;
	IndexToNotation(index, col, row);

	std::ofstream out(s_Path, std::ios_base::app);
	out << col << row;
	std::cout << col << row;
}

void TraverseCaptures(Position current, int fromIndex, Position after, std::vector<char> move)
{
	Bitboard captures = current.GetCaptures(Board::FromIndex(fromIndex));

	if (Board::IsEmpty(captures))
	{
		current.EndTurn();
		if (current.Black == after.Black && current.White == after.White && current.Queens == after.Queens && current.SinceCapture == after.SinceCapture)
		{
			std::ofstream out(s_Path, std::ios_base::app);
			for (char c : move)
			{
				out << c;
				std::cout << c;
			}
			out << std::endl;
			std::cout << std::endl;
		}

		return;
	}

	int choices[8];
	int choiceCnt = Board::GetBits(captures, choices);

	for (int choiceIdx = 0; choiceIdx < choiceCnt; choiceIdx++)
	{
		int toIndex = choices[choiceIdx];
		Position next = current;
		next.Capture(fromIndex, toIndex);

		char col, row;
		IndexToNotation(toIndex, col, row);

		move.push_back(':');
		move.push_back(col);
		move.push_back(row);

		TraverseCaptures(next, toIndex, after, move);

		move.pop_back();
		move.pop_back();
		move.pop_back();
	}
}

void PrintMove(Position before, Position after)
{
	Bitboard capturing = before.GetAllCapturing();

	if (Board::IsEmpty(capturing))
	{
		Bitboard move = before.BlackTurn ? before.Black ^ after.Black : before.White ^ after.White;

		int fromIndex = std::countr_zero(move);
		move &= move - 1;
		int toIndex = std::countr_zero(move);

		if (Board::FromIndex(fromIndex) & after.GetOpponent())
			std::swap(fromIndex, toIndex);

		
		PrintIndex(fromIndex);
		{
			std::ofstream out(s_Path, std::ios_base::app);
			out << '-';
			std::cout << '-';
		}
		PrintIndex(toIndex);
		{
			std::ofstream out(s_Path, std::ios_base::app);
			out << std::endl;
			std::cout << std::endl;
		}

		return;
	}

	int choices[12];
	int choiceCnt = Board::GetBits(capturing, choices);

	for (int choiceIdx = 0; choiceIdx < choiceCnt; choiceIdx++)
	{
		char row, col;
		IndexToNotation(choices[choiceIdx], col, row);

		TraverseCaptures(before, choices[choiceIdx], after, {col, row});
	}
}

ConsoleController::ConsoleController() : Controller(ControllerType::PlayerController)
{
}

ConsoleController::~ConsoleController()
{
}

void ConsoleController::OnClick(float x, float y)
{
}

Position ConsoleController::MakeMove(Position position)
{
	std::string move;
	std::cin >> move;

	int i, j;
	NotationToCoords(i, j, move[0], move[1]);
	int fromIndex = Board::CoordsToIndex(i, j);

	if (move[2] == '-')
	{
		NotationToCoords(i, j, move[3], move[4]);
		int toIndex = Board::CoordsToIndex(i, j);

		position.Move(fromIndex, toIndex);
		position.EndTurn();

		return position;
	}

	int index = 2;
	while (index < move.size())
	{
		assert(move[index] == ':');
		
		NotationToCoords(i, j, move[index + 1], move[index + 2]);
		int toIndex = Board::CoordsToIndex(i, j);

		position.Capture(fromIndex, toIndex);

		fromIndex = toIndex;
	}

	position.EndTurn();
	return position;
}

void ConsoleController::CancelMove()
{
}

}