#pragma once

#include <glm/glm.hpp>

#include <array>
#include <cstdint>

#include "Utils.h"

namespace Checkers
{

class RendererImpl
{
public:
	RendererImpl(const Position &position);
	~RendererImpl();

	uint32_t GetTextureId() const;

	void ClearHighlights();
	void HighlightTile(int i, int j);
	void SelectTile(int i, int j);

	void Flip();

	void Resize(uint32_t width, uint32_t height);
	void Render();

private:
	static constexpr size_t MaxQuads = 100;

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Color;
		uint32_t TexIndex;
		glm::vec2 TexCoords;
	};

	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;

	Framebuffer m_Framebuffer;

	size_t m_QuadCount = 0;
	Vertex m_Vertices[4 * MaxQuads] = {};
	uint32_t m_VertexBufferId = 0;

	Shader m_Shader;

	bool m_Flipped;
	std::array<Texture, 9> m_Textures;

	std::array<std::array<glm::vec3, 8>, 8> m_Colors = {};

	const Position &m_Position;

	void AddQuad(int i, int j, uint8_t texture, glm::vec3 color = glm::vec3(1));
};

}
