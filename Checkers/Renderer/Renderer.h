#pragma once

#include <cstdint>

#include "Position.h"

namespace Checkers
{

class Renderer
{
public:
	static void Init(const Position &position);
	static void Shutdown();

	static uint32_t GetTextureId();

	static void ClearHighlights();
	static void HighlightTile(int i, int j);
	static void SelectTile(int i, int j);

	static void Flip();

	static void Resize(uint32_t width, uint32_t height);
	static void Render();
};

}
