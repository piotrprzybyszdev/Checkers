#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb_image.h>

#include <cassert>

#include "Renderer.h"
#include "RendererImpl.h"

namespace Checkers
{

static RendererImpl *s_Renderer;

void Renderer::Init(const Position &position)
{
	static RendererImpl renderer(position);
	s_Renderer = &renderer;
}

void Renderer::Shutdown()
{
}

uint32_t Renderer::GetTextureId()
{
	return s_Renderer->GetTextureId();
}

void Renderer::ClearHighlights()
{
	s_Renderer->ClearHighlights();
}

void Renderer::HighlightTile(int i, int j)
{
	s_Renderer->HighlightTile(i, j);
}

void Renderer::SelectTile(int i, int j)
{
	s_Renderer->SelectTile(i, j);
}

void Renderer::Flip()
{
	s_Renderer->Flip();
}

void Renderer::Resize(uint32_t width, uint32_t height)
{
	s_Renderer->Resize(width, height);
}

void Renderer::Render()
{
	s_Renderer->Render();
}

}
