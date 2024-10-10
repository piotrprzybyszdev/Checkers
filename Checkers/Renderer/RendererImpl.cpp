#include <glad/glad.h>

#include "Core/Core.h"

#include "Position.h"

#include "RendererImpl.h"
#include "Resources.h"

namespace Checkers
{

namespace Colors
{

static constexpr glm::vec3 Black = glm::vec3(181.0f, 136.0f, 99.0f) / 255.0f;
static constexpr glm::vec3 White = glm::vec3(240.0f, 217.0f, 181.0f) / 255.0f;
static constexpr glm::vec3 Selected = glm::vec3(82.0f, 176.0f, 220.0f) / 255.0f;
static constexpr glm::vec3 Highlighted = glm::vec3(235.0f, 97.0f, 80.0f) / 255.0f;

}

RendererImpl::RendererImpl(const Position &m_Position)
	: m_Shader(Resources::VertexShaderSource, Resources::FragmentShaderSource), m_Textures{ Texture(0xffffffff),
	Texture(Resources::BlackPawnImage), Texture(Resources::WhitePawnImage),
	Texture(Resources::BlackQueenImage), Texture(Resources::WhiteQueenImage),
	Texture(Resources::BlackPawnImage, true), Texture(Resources::WhitePawnImage, true),
	Texture(Resources::BlackQueenImage, true), Texture(Resources::WhiteQueenImage, true)},
	m_Position(m_Position), m_Flipped(false)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GlAssert();

	GLuint vertexArray, indexBuffer;

	glCreateVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	glCreateBuffers(1, &m_VertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, 4 * MaxQuads * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Color));

	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), (const void*)offsetof(Vertex, TexIndex));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexCoords));
	GlAssert();

	uint32_t indices[6 * MaxQuads];
	for (int i = 0; i < MaxQuads; i++)
	{
		indices[i * 6] = i * 4;
		indices[i * 6 + 1] = i * 4 + 1;
		indices[i * 6 + 2] = i * 4 + 2;
		indices[i * 6 + 3] = i * 4 + 2;
		indices[i * 6 + 4] = i * 4 + 3;
		indices[i * 6 + 5] = i * 4;
	}

	glCreateBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	GlAssert();

	m_Shader.Enable();
	int samplers[5] = { 0, 1, 2, 3, 4 };
	m_Shader.UploadSamplers("u_Textures", samplers, 5);

	ClearHighlights();
}

RendererImpl::~RendererImpl()
{
}

uint32_t RendererImpl::GetTextureId() const
{
	return m_Framebuffer.GetTextureId();
}

void RendererImpl::ClearHighlights()
{
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			m_Colors[i][j] = (i + j) % 2 == 0 ? Colors::Black : Colors::White;
}

void RendererImpl::HighlightTile(int i, int j)
{
	m_Colors[i][j] = Colors::Highlighted;
}

void RendererImpl::SelectTile(int i, int j)
{
	m_Colors[i][j] = Colors::Selected;
}

void RendererImpl::Flip()
{
	m_Flipped = !m_Flipped;
}

void RendererImpl::Resize(uint32_t width, uint32_t height)
{
	if (m_ViewportWidth == width && m_ViewportHeight == height)
		return;

	m_ViewportWidth = width; m_ViewportHeight = height;

	glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
	m_Framebuffer.Resize(m_ViewportWidth, m_ViewportHeight);
}

void RendererImpl::Render()
{
	m_QuadCount = 0;
	for (int j = 0; j < 8; j++)
		for (int i = 0; i < 8; i++)
		{
			AddQuad(i, j, 0, m_Colors[i][j]);

			if ((i + j) % 2 == 1) continue;

			const Bitboard board = Board::FromCoords(i, j);

			if (m_Position.Black & m_Position.Queens & board)
				AddQuad(i, j, 3);
			else if (m_Position.Black & board)
				AddQuad(i, j, 1);

			if (m_Position.White & m_Position.Queens & board)
				AddQuad(i, j, 4);
			else if (m_Position.White & board)
				AddQuad(i, j, 2);
		}

	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_QuadCount * 4 * sizeof(Vertex), m_Vertices);

	m_Framebuffer.Bind();
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	GlAssert();

	m_Textures[0].BindToUnit(0);
	int offset = m_Flipped ? 4 : 0;
	for (int i = 1; i < 5; i++)
		m_Textures[offset + i].BindToUnit(i);

	m_Shader.Enable();
	glDrawElements(GL_TRIANGLES, m_QuadCount * 6, GL_UNSIGNED_INT, nullptr);

	m_Framebuffer.Unbind();
	GlAssert();
}

void RendererImpl::AddQuad(int i, int j, uint8_t texture, glm::vec3 color)
{
	static constexpr float tileSize = 0.25f;
	const glm::vec2 bottomLeft = -1.0f + glm::vec2(i, j) * tileSize;
	const glm::vec2 topRight = bottomLeft + tileSize;

	m_Vertices[m_QuadCount * 4] = Vertex(glm::vec3(bottomLeft.x, bottomLeft.y, 0.0f), color, texture, glm::vec2(0, 0));
	m_Vertices[m_QuadCount * 4 + 1] = Vertex(glm::vec3(topRight.x, bottomLeft.y, 0.0f), color, texture, glm::vec2(1, 0));
	m_Vertices[m_QuadCount * 4 + 2] = Vertex(glm::vec3(topRight.x, topRight.y, 0.0f), color, texture, glm::vec2(1, 1));
	m_Vertices[m_QuadCount * 4 + 3] = Vertex(glm::vec3(bottomLeft.x, topRight.y, 0.0f), color, texture, glm::vec2(0, 1));
	m_QuadCount++;
}

}
