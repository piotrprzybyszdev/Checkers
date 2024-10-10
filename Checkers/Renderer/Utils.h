#pragma once

#include <glad/glad.h>

#include <cstdint>
#include <string>

#include "Core/Core.h"

namespace Checkers
{

static inline GLenum GlGetError()
{
	return glGetError();
}

using GlAssert = Assert<GLenum, GlGetError, GL_NO_ERROR>;

class Shader
{
public:
	Shader(const std::string &vertexSource, const std::string &fragmentSource);
	~Shader();

	void Enable() const;
	void UploadSamplers(const std::string &name, int samplers[], size_t size) const;

private:
	GLuint m_ShaderId;
};

class Texture
{
public:
	Texture(uint32_t color);
	Texture(const std::string &data, bool flip = false);
	~Texture();

	void Resize(uint32_t width, uint32_t height) const;

	void BindToUnit(uint8_t unit) const;
	GLuint GetTextureId() const;

private:
	GLuint m_TextureId = 0;
};

class Framebuffer
{
public:
	Framebuffer();
	~Framebuffer();

	void Resize(uint32_t width, uint32_t height) const;

	void Bind() const;
	void Unbind() const;
	GLuint GetTextureId() const;

private:
	Texture m_ColorTexture;
	GLuint m_FramebufferId = 0;
};

}
