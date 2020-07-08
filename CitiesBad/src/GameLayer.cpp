#include "GameLayer.h"

#include <stb_image/stb_image.h>
#include <GLCore/Core/KeyCodes.h>

using namespace GLCore;
using namespace GLCore::Utils;

GameLayer::GameLayer()
	: m_CameraController(16.0f / 9.0f, false)
{

}

GameLayer::~GameLayer()
{

}

static GLuint LoadTexture(const std::string& path)
{
	int w, h, bits;

	stbi_set_flip_vertically_on_load(1);
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

struct Vertex
{
	float Position[3];
	float Color[4];
	float TexCoord[2];
	float TexID;
};

static const size_t VertexFloatCount = sizeof(Vertex) / sizeof(float);
static const uint8_t VertexCount = 4;
static const uint8_t ObjectCount = 3;

static const float DefaultPositions[] = {
	0.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
};

static const float DefaultColors[] = {
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f
};

static const float DefaultTexCoords[] = {
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 1.0f
};

static const float DefaultTexID[] = {
	1.0f,
	1.0f,
	1.0f,
	1.0f,
};

class SomeObject
{
public:
	glm::vec2 Position;
};

static float* CreateBuffer(uint32_t count, uint64_t* bufferSize)
{
	size_t re = sizeof(Vertex) * VertexCount * count;
	float* buffer = (float*)malloc(re);
	memset(buffer, 0.0f, re);
	*bufferSize = re;
	assert(("out of memory", buffer != nullptr));
	return buffer;
}

static void Default(float* buffer, uint32_t idx)
{
	for (uint16_t i = 0; i < VertexCount; i++)
	{
		// position
		memcpy(
			buffer + (VertexFloatCount * i) + (idx * VertexFloatCount * VertexCount),
			DefaultPositions + (3 * i),
			sizeof(float) * 3);

		// color
		memcpy(
			buffer + 3 + (VertexFloatCount * i) + (idx * VertexFloatCount * VertexCount),
			DefaultColors + (4 * i),
			sizeof(float) * 4);

		// texCoord
		memcpy(
			buffer + 7 + (VertexFloatCount * i) + (idx * VertexFloatCount * VertexCount),
			DefaultTexCoords + (2 * i),
			sizeof(float) * 2);

		// texID
		memcpy(
			buffer + 9 + (VertexFloatCount * i) + (idx * VertexFloatCount * VertexCount),
			DefaultTexID + (1 * i),
			sizeof(float) * 1);
	}
}

static void PositionSet(float* buffer, uint32_t idx, glm::vec2 position)
{
	for (uint16_t i = 0; i < VertexCount; i++)
	{
		float* bufferIdx = buffer + (VertexFloatCount * i) + (VertexCount * idx * VertexFloatCount);

		bufferIdx[0] = position.x + DefaultPositions[i * 3];
		bufferIdx[1] = position.y + DefaultPositions[i * 3 + 1];
	}
}

void GameLayer::OnAttach()
{
	// setup

	EnableGLDebugging();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_Shader = Shader::FromGLSLTextFiles(
		"assets/shaders/test.vert.glsl",
		"assets/shaders/test.frag.glsl"
	);

	// uniforms

	glUseProgram(m_Shader->GetRendererID());
	auto loc = glGetUniformLocation(m_Shader->GetRendererID(), "u_Textures");
	int samplers[2] = { 0, 1 };
	glUniform1iv(loc, 2, samplers);

	// VAO

	glCreateVertexArrays(1, &m_QuadVA);
	glBindVertexArray(m_QuadVA);

	// VBO

	glCreateBuffers(1, &m_QuadVB);
	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
	glBufferData(GL_ARRAY_BUFFER, VertexFloatCount * ObjectCount * VertexCount * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

	// attribs

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Color));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexCoord));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexID));

	// generate GrandBuffer

	m_GrandBuffer = CreateBuffer(ObjectCount, &m_GradeBufferCount);

	// generate indices

	uint32_t indices[ObjectCount * 6];

	for (uint32_t i = 0; i < ObjectCount; i++)
	{
		Default(m_GrandBuffer, i);
		PositionSet(m_GrandBuffer, i, { i * 1.5f, 0.0f });

		indices[(i * 6) + 0] = (i * 4) + 0;
		indices[(i * 6) + 1] = (i * 4) + 1;
		indices[(i * 6) + 2] = (i * 4) + 2;
		indices[(i * 6) + 3] = (i * 4) + 2;
		indices[(i * 6) + 4] = (i * 4) + 3;
		indices[(i * 6) + 5] = (i * 4) + 0;
	}

	glCreateBuffers(1, &m_QuadIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	m_TexPixel = LoadTexture("assets/textures/pixel.png");
	m_TexTest = LoadTexture("assets/textures/test.png");

	// print GrandBuffer

	for (uint32_t i = 0; i < ObjectCount * VertexCount * VertexFloatCount; i++)
	{
		if (i % (sizeof(Vertex) / sizeof(float)) == 0)
			printf("\n");

		if (i % ((sizeof(Vertex) / sizeof(float)) * VertexCount) == 0)
			printf("\n");

		printf("%f, ", m_GrandBuffer[i]);
	}
}

void GameLayer::OnDetach()
{
	glDeleteVertexArrays(1, &m_QuadVA);
	glDeleteBuffers(1, &m_QuadVB);
	glDeleteBuffers(1, &m_QuadIB);
}

static bool movLeft = false;
static bool movRight = false;
static bool movUp = false;
static bool movDown = false;

void GameLayer::OnEvent(Event& event)
{
	m_CameraController.OnEvent(event);

	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<MouseButtonPressedEvent>(
		[&](MouseButtonPressedEvent& e)
		{
			return false;
		});
	dispatcher.Dispatch<MouseButtonReleasedEvent>(
		[&](MouseButtonReleasedEvent& e)
		{
			return false;
		});
	dispatcher.Dispatch<KeyPressedEvent>(
		[&](KeyPressedEvent& e)
		{
			if (e.GetKeyCode() == HZ_KEY_A)
				movLeft = true;
			if (e.GetKeyCode() == HZ_KEY_D)
				movRight = true;
			if (e.GetKeyCode() == HZ_KEY_W)
				movUp = true;
			if (e.GetKeyCode() == HZ_KEY_S)
				movDown = true;

			return false;
		});
	dispatcher.Dispatch<KeyReleasedEvent>(
		[&](KeyReleasedEvent& e)
		{
			if (e.GetKeyCode() == HZ_KEY_A)
				movLeft = false;
			if (e.GetKeyCode() == HZ_KEY_D)
				movRight = false;
			if (e.GetKeyCode() == HZ_KEY_W)
				movUp = false;
			if (e.GetKeyCode() == HZ_KEY_S)
				movDown = false;

			return false;
		});
}

static const glm::vec4 GeneralColor = { 1.0f, 1.0f, 1.0f, 1.0f };

void GameLayer::OnUpdate(Timestep ts)
{
	if (movLeft)
		m_CameraController.GetCamera().SetPosition(m_CameraController.GetCamera().GetPosition() + glm::vec3(ts * -3.0f, 0.0f, 0.0f));
	if (movRight)
		m_CameraController.GetCamera().SetPosition(m_CameraController.GetCamera().GetPosition() + glm::vec3(ts * 3.0f, 0.0f, 0.0f));
	if (movUp)
		m_CameraController.GetCamera().SetPosition(m_CameraController.GetCamera().GetPosition() + glm::vec3(0.0f, ts * 3.0f, 0.0f));
	if (movDown)
		m_CameraController.GetCamera().SetPosition(m_CameraController.GetCamera().GetPosition() + glm::vec3(0.0f, ts * -3.0f, 0.0f));

	m_CameraController.OnUpdate(ts);

	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * VertexCount * VertexFloatCount * ObjectCount, m_GrandBuffer);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_Shader->GetRendererID());
	glBindTextureUnit(0, m_TexPixel);
	glBindTextureUnit(1, m_TexTest);

	int location = 0;

	location = glGetUniformLocation(m_Shader->GetRendererID(), "u_ViewProjection");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m_CameraController.GetCamera().GetViewProjectionMatrix()));

	location = glGetUniformLocation(m_Shader->GetRendererID(), "u_Color");
	glUniform4fv(location, 1, glm::value_ptr(GeneralColor));

	glBindVertexArray(m_QuadVA);
	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, nullptr);
}

void GameLayer::OnImGuiRender()
{

}
