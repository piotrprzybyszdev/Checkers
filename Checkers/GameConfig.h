#pragma once

#include <filesystem>

#include "Controllers/Controller.h"

namespace Checkers
{

void NotationToCoords(int &i, int &j, char col, char row);

Controller *ReadPlayer(std::string color, std::chrono::seconds time);

void SetFilePath(std::filesystem::path path);
void PrintMove(Position before, Position after);

class ConsoleController : public Controller
{
public:
	ConsoleController();
	~ConsoleController();

	void OnClick(float x, float y) override;
	Position MakeMove(Position position) override;
	void CancelMove() override;
};

}
