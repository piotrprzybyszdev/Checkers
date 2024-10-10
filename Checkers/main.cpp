#include <iostream>

#include "Core/Core.h"
#include "Renderer/Renderer.h"

#include "Game.h"
#include "GameConfig.h"
#include "Window.h"

using namespace Checkers;

void HeadlessGame()
{
	int sec;
	std::cout << "How many seconds a move: ";
	std::cin >> sec;
	std::chrono::seconds time(sec);

	char c;
	std::cout << "Should statistics about the MCTS be printed to the console? [Y/N]: ";
	std::cin >> c;
	bool printStats = c == 'y' || c == 'Y';

	Controller *blackPlayer = ReadPlayer("Black", time);
	Controller *whitePlayer = ReadPlayer("White", time);

	Position position(StartingPosition);
	Position next;

	while (!position.IsDraw() && !position.HasLost())
	{
		next = whitePlayer->MakeMove(position);

		PrintMove(position, next);
		position = next;

		Stats::FlushTimers();
		for (const auto &stat : Stats::GetStats())
			std::cout << stat.second << std::endl;
		Stats::Clear();

		if (position.IsDraw() || position.HasLost())
			break;

		next = blackPlayer->MakeMove(position);
		
		PrintMove(position, next);
		position = next;

		Stats::FlushTimers();
		for (const auto &stat : Stats::GetStats())
			std::cout << stat.second << std::endl;
		Stats::Clear();
	}

	if (position.IsDraw())
		std::cout << "Game ended in a draw" << std::endl;
	if (position.HasLost())
		std::cout << (position.BlackTurn ? "White" : "Black") << " won!" << std::endl;
}

int main(int argc, char *argv[])
{
	std::string outputFileName;

	if (argc > 1)
		outputFileName = argv[1];
	else
	{
		std::cout << "Output file name wasn't given" << std::endl;
		std::cout << "You can provide it now or leave it empty for no output file: ";
		std::getline(std::cin, outputFileName);
	}

	if (outputFileName != "")
		SetFilePath(outputFileName);

	try
	{
		char c;
		do {
			std::cout << "Launch UI? [Y/N]: ";
			std::cin >> c;
		} while (c != 'y' && c != 'Y' && c != 'n' && c != 'N');

		if (c == 'n' || c == 'N')
		{
			HeadlessGame();
			return EXIT_SUCCESS;
		}

		Window window(1280, 720, "Checkers", true);

		Game::Init(&window);
		Game::Start();
		Renderer::Init(Game::GetPosition());

		while (!window.ShouldClose())
		{
			window.OnUpdate();
			window.OnRender();

			window.WaitEvents();
		}

		Game::End();
		Renderer::Shutdown();
	}
	catch (std::exception exception)
	{
		std::cerr << exception.what() << std::endl;

		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}
