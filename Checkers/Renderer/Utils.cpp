#include <stb_image.h>

#include <cassert>
#include <vector>

#include "Core/Core.h"

#include "Utils.h"

namespace Checkers
{

static GLuint vertexShader;
static GLuint fragmentShader;
static GLuint program;
static std::vector<GLchar> log;

enum class Stage
{
	Vertex, Fragment, Program
};

static GLint GetShaderCompilationStatus()
{
	GLint status = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	return status;
}

template<Stage stage>
static const char *GetShaderCompilationErrorMessage(GLint value)
{
	GLuint st = stage == Stage::Vertex ? vertexShader :
		stage == Stage::Fragment ? fragmentShader : program;

	GLint length = 0;
	glGetShaderiv(st, GL_INFO_LOG_LENGTH, &length);

	log = std::vector<GLchar>(length);
	glGetShaderInfoLog(st, length, &length, log.data());
	
	return log.data();
}

template<Stage stage>
using ShaderAssert = Assert<GLint, GetShaderCompilationStatus, GL_TRUE, GetShaderCompilationErrorMessage<stage>>;

Shader::Shader(const std::string &vertexSource, const std::string &fragmentSource) : m_ShaderId(0)
{
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const GLchar* source = (const GLchar*)vertexSource.c_str();
	glShaderSource(vertexShader, 1, &source, 0);
	glCompileShader(vertexShader);

	ShaderAssert<Stage::Vertex>();

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	source = (const GLchar*)fragmentSource.c_str();
	glShaderSource(fragmentShader, 1, &source, 0);
	glCompileShader(fragmentShader);

	ShaderAssert<Stage::Fragment>();

	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program);

	ShaderAssert<Stage::Program>();

	m_ShaderId = program;
	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
	glDeleteProgram(program);
}

void Shader::Enable() const
{
	glUseProgram(m_ShaderId);
	GlAssert();
}

void Shader::UploadSamplers(const std::string &name, int samplers[], size_t size) const
{
	GLint location = glGetUniformLocation(m_ShaderId, name.c_str());
	glUniform1iv(location, size, samplers);
	GlAssert();
}

Texture::Texture(uint32_t color)
{
	glGenTextures(1, &m_TextureId);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);
	GlAssert();
}

Texture::Texture(const std::string &data, bool flip)
{
	int width, height, channels;
	stbi_set_flip_vertically_on_load(flip);
	stbi_uc *loaded = stbi_load_from_memory((stbi_uc*)data.c_str(), data.size(), &width, &height, &channels, 4);
	assert(loaded != nullptr);

	glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureId);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, loaded);

	stbi_image_free(loaded);
	GlAssert();
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_TextureId);
}

void Texture::Resize(uint32_t width, uint32_t height) const
{
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
	GlAssert();
}

void Texture::BindToUnit(uint8_t unit) const
{
	glBindTextureUnit(unit, m_TextureId);
	GlAssert();
}

GLuint Texture::GetTextureId() const
{
	return m_TextureId;
}

Framebuffer::Framebuffer() : m_ColorTexture(0)
{
	glGenFramebuffers(1, &m_FramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferId);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_ColorTexture.GetTextureId(), 0);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GlAssert();
}

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &m_FramebufferId);
}

void Framebuffer::Resize(uint32_t width, uint32_t height) const
{
	m_ColorTexture.Resize(width, height);
}

void Framebuffer::Bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferId);
	GlAssert();
}

void Framebuffer::Unbind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GlAssert();
}

GLuint Framebuffer::GetTextureId() const
{
	return m_ColorTexture.GetTextureId();
}

}
