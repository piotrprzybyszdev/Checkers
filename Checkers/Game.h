#pragma once

#include "Controllers/Controller.h"
#include "Window.h"

namespace Checkers
{

class Game
{
public:
	static void Init(Window *window);
	static void Start();
	static void End();

	static ControllerType GetBlackPlayerType();
	static ControllerType GetWhitePlayerType();

	static void SelectBlackPlayer(ControllerType type);
	static void SelectWhitePlayer(ControllerType type);

	static Position &GetPosition();

	static void HandleClick(float x, float y);

private:
	static Window *s_Window;

	static Position s_Position;
	static bool s_Finished;

	static ControllerType s_BlackControllerType;
	static ControllerType s_WhiteControllerType;

	static void Run();
	static void ChangeBlackPlayer();
	static void ChangeWhitePlayer();
	static void CheckFinished();
};

}
