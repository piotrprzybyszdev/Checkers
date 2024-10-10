#pragma once

#include "Position.h"

namespace Checkers
{

enum class ControllerType
{
	PlayerController,
	ComputerHostController,
	ComputerDeviceController
};

class Controller
{
public:
	Controller(ControllerType type) : m_Type(type) {}
	virtual ~Controller() = default;

	ControllerType GetControllerType()
	{
		return m_Type;
	}

	// Just passing an event (UI thread)
	virtual void OnClick(float x, float y) = 0;

	// Blocking call (Game thread), removes cancellation
	virtual Position MakeMove(Position position) = 0;

	// Non-blocking (UI thread)
	// MakeMove should return soon after call to this
	virtual void CancelMove() = 0;

private:
	const ControllerType m_Type;
};

}
