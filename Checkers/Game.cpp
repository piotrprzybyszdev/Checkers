#include <bit>
#include <format>
#include <iostream>

#include "Core/Core.h"

#include "Controllers/ComputerController.h"
#include "Controllers/PlayerController.h"
#include "Game.h"

namespace Checkers
{

Window *Game::s_Window = nullptr;

Position Game::s_Position = StartingPosition;
bool Game::s_Finished = false;

ControllerType Game::s_BlackControllerType = ControllerType::PlayerController;
ControllerType Game::s_WhiteControllerType = ControllerType::PlayerController;

static ComputerController s_ComputerHostBlack(ControllerType::ComputerHostController, Simulator::CreateHost(), 1e9, std::chrono::seconds(1));
static ComputerController s_ComputerHostWhite(ControllerType::ComputerHostController, Simulator::CreateHost(), 1e9, std::chrono::seconds(1));
static ComputerController s_ComputerDeviceBlack(ControllerType::ComputerDeviceController, Simulator::CreateDevice(96, 64), 1e9, std::chrono::seconds(1), 96);
static ComputerController s_ComputerDeviceWhite(ControllerType::ComputerDeviceController, Simulator::CreateDevice(24, 128), 1e9, std::chrono::seconds(1), 24);
static PlayerController s_PlayerBlack;
static PlayerController s_PlayerWhite;

static Controller *s_ControllerBlack = &s_PlayerBlack;
static Controller *s_ControllerWhite = &s_PlayerWhite;

static std::thread s_GameThread;

void Game::Init(Window *window)
{
	s_Window = window;
}

void Game::Start()
{
	s_Finished = false;
	s_Window->SetTextOverlay(nullptr);
	s_Position = StartingPosition;
	s_GameThread = std::thread(Game::Run);
}

void Game::End()
{
	s_Finished = true;
	s_ControllerBlack->CancelMove();
	s_ControllerWhite->CancelMove();
	s_GameThread.join();
}

ControllerType Game::GetBlackPlayerType()
{
	return s_BlackControllerType;
}

ControllerType Game::GetWhitePlayerType()
{
	return s_WhiteControllerType;
}

void Game::SelectBlackPlayer(ControllerType type)
{
	s_BlackControllerType = type;
	s_ControllerBlack->CancelMove();
}

void Game::SelectWhitePlayer(ControllerType type)
{
	s_WhiteControllerType = type;
	s_ControllerWhite->CancelMove();
}

Position &Game::GetPosition()
{
	return s_Position;
}

void Game::HandleClick(float x, float y)
{
	if (s_Finished) return;

	if (s_Position.BlackTurn)
		s_ControllerBlack->OnClick(x, y);
	else
		s_ControllerWhite->OnClick(x, y);
}

void Game::Run()
{
	while (!s_Finished)
	{
		Position newPosition;
		Controller *controller;
		
		do {
			if (s_Position.BlackTurn)
				ChangeBlackPlayer();
			else
				ChangeWhitePlayer();

			controller = s_Position.BlackTurn ? s_ControllerBlack : s_ControllerWhite;

			newPosition = controller->MakeMove(s_Position);

			if (s_Finished) return;
		} while (controller->GetControllerType() != (s_Position.BlackTurn ? s_BlackControllerType : s_WhiteControllerType));

		s_Position = newPosition;
		Stats::FlushTimers();
		s_Window->Refresh();

		CheckFinished();
		if (s_Finished) return;
	}
}

void Game::ChangeBlackPlayer()
{
	if (s_ControllerBlack->GetControllerType() == s_BlackControllerType) return;

	switch (s_BlackControllerType)
	{
	case ControllerType::ComputerDeviceController:
		s_ControllerBlack = &s_ComputerDeviceBlack;
		break;
	case ControllerType::ComputerHostController:
		s_ControllerBlack = &s_ComputerHostBlack;
		break;
	case ControllerType::PlayerController:
		s_ControllerBlack = &s_PlayerBlack;
		break;
	}
}

void Game::ChangeWhitePlayer()
{
	if (s_ControllerWhite->GetControllerType() == s_WhiteControllerType) return;

	switch (s_WhiteControllerType)
	{
	case ControllerType::ComputerDeviceController:
		s_ControllerWhite = &s_ComputerDeviceWhite;
		break;
	case ControllerType::ComputerHostController:
		s_ControllerWhite = &s_ComputerHostWhite;
		break;
	case ControllerType::PlayerController:
		s_ControllerWhite = &s_PlayerWhite;
		break;
	}
}

void Game::CheckFinished()
{
	if (s_Position.HasLost())
	{
		s_Finished = true;
		if (s_Position.BlackTurn)
			s_Window->SetTextOverlay("White won!");
		else
			s_Window->SetTextOverlay("Black won!");
	}

	if (s_Position.IsDraw())
	{
		s_Finished = true;
		s_Window->SetTextOverlay("Draw by 30 moves without captures!");
	}
}

}
