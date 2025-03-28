#include <iostream>

#include "Core/Core.h"
#include "Renderer/Renderer.h"

#include "Game.h"
#include "Window.h"

using namespace Checkers;

int main(int argc, char *argv[])
{
	try
	{
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
