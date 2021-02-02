#include "Quad.h"

#include <stb_image.h>

static GLuint LoadTexture(const std::string& path)
{
	int w, h, bits;

	stbi_set_flip_vertically_on_load(0);
	auto* pixels = stbi_load(path.c_str(), &w, &h, &bits, STBI_rgb_alpha);
	GLuint textureID;
	glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	stbi_image_free(pixels);

	return textureID;
}

Quad::Quad(glm::vec4 color)
	:m_HasTexture(false), m_VA(0), m_VB(0), m_IB(0), m_Texture(0)
{
	float positions[] = {
		0.0f, 0.0f,
		0.5f, 0.0f,
		0.5f, 0.5f,
		0.0f, 0.5f,
	};

	uint32_t indices[] = {
		0, 1, 2, 2, 3, 0,
	};

	m_Shader = Shader::FromGLSLTextFiles(
		"res/shaders/test.vert.glsl",
		"res/shaders/test.frag.glsl"
	);

	glUseProgram(m_Shader->GetRendererID());
	glUniform4f(glGetUniformLocation(m_Shader->GetRendererID(), "u_Color"), color.r, color.g, color.b, color.a);

	glCreateVertexArrays(1, &m_VA);
	glBindVertexArray(m_VA);

	glCreateBuffers(1, &m_VB);
	glBindBuffer(GL_ARRAY_BUFFER, m_VB);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), positions, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*)0);

	glCreateBuffers(1, &m_IB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6, indices, GL_STATIC_DRAW);
}

Quad::Quad(const std::string& path)
	:m_HasTexture(true), m_VA(0), m_VB(0), m_IB(0), m_Texture(0)
{
	float positions[] = {
		0.0f, 0.0f, 0.0f, 1.0f,
		0.5f, 0.0f, 1.0f, 1.0f,
		0.5f, 0.5f, 1.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f
	};

	uint32_t indices[] = {
		0, 1, 2, 2, 3, 0,
	};

	m_Shader = Shader::FromGLSLTextFiles(
		"res/shaders/test2.vert.glsl",
		"res/shaders/test2.frag.glsl"
	);

	glUseProgram(m_Shader->GetRendererID());
	glUniform1i(glGetUniformLocation(m_Shader->GetRendererID(), "u_Texture"), 0);

	glCreateVertexArrays(1, &m_VA);
	glBindVertexArray(m_VA);

	glCreateBuffers(1, &m_VB);
	glBindBuffer(GL_ARRAY_BUFFER, m_VB);
	glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), positions, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)(sizeof(float) * 2));

	glCreateBuffers(1, &m_IB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6, indices, GL_STATIC_DRAW);

	m_Texture = LoadTexture(path);
}

Quad::~Quad()
{
	glDeleteVertexArrays(1, &m_VA);
	glDeleteBuffers(1, &m_VB);
	glDeleteBuffers(1, &m_IB);
}

void Quad::Render(Camera& camera)
{
	glUseProgram(m_Shader->GetRendererID());
	glBindVertexArray(m_VA);

	glm::mat4 projection = glm::ortho(0.0f, (16.0f / 9.0f), 1.0f, 0.0f, 0.0f, 100.0f);
	glm::mat4 view(1.0f);

	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetRendererID(), "u_MVP"), 1, GL_FALSE, glm::value_ptr(projection * view * camera.ConstructModel(
		glm::vec3(0.0f),
		glm::vec3(0.0f),
		glm::vec3(1.0f)
	)));

	if (m_HasTexture)
		glBindTextureUnit(0, m_Texture);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Quad::Render(Camera& camera, uint32_t texture)
{
	glUseProgram(m_Shader->GetRendererID());
	glBindVertexArray(m_VA);

	glm::mat4 projection = glm::ortho(16.0f / 9.0f, 0.0f, 0.0f, 1.0f, 0.0f, 100.0f);
	glm::mat4 view(1.0f);

	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetRendererID(), "u_MVP"), 1, GL_FALSE, glm::value_ptr(projection * view * camera.ConstructModel(
		glm::vec3(0.0f),
		glm::vec3(0.0f),
		glm::vec3(1.0f)
	)));
	
	glBindTextureUnit(0, texture);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}
