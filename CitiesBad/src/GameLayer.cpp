#include "GameLayer.h"
#include "road/Lane.h"

#include <stb_image/stb_image.h>
#include <GLCore/Core/KeyCodes.h>

using namespace GLCore;
using namespace GLCore::Utils;

GameLayer::GameLayer()
	: m_CameraController(16.0f / 9.0f, false)
{
	m_CameraController.GetCamera().SetPosition({0.0f, 0.0f, 0.0f});
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

static const uint8_t VertexCount = 4;
static const uint8_t ObjectCount = 2;

static const float DefaultPositions[] = {
	0.0f, 0.0f, 0.0f,
	0.1f, 0.0f, 0.0f,
	0.1f, 0.1f, 0.0f,
	0.0f, 0.1f, 0.0f,
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

GLuint lnVA, lnVB, lnIB;
Lane ln;
std::vector<Vertex>* lnVertices;
std::vector<uint32_t>* lnIndices;

float lnLPos0[2] = { 0.0f, 0.0f };
float lnLPos1[2] = { 0.0f, 0.0f };
float lnLBez[2] = { 0.0f, 0.0f };

float lnPos0[2] = { 0.0f, 0.0f };
float lnPos1[2] = { 0.0f, 0.0f };
float lnBez[2] = { 0.0f, 0.0f };

GLuint cubeVA, cubeVB, cubeIB;

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

	// debug

	//ln.SetPosition({ 0.0f, 0.0f, 0.0f }, { 5.0f, 0.0f, 1.0f });
	//ln.SetBezier({ 2.5f, 1.0f });
	//ln.SetPosition({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 5.0f });
	//ln.SetBezier({ 0.0f, 2.5f });
	ln.SetPosition({ 0.0f, 0.0f, 0.0f }, { 3.0f, 0.0f, 3.0f });
	ln.SetBezier({ 0.0f, 3.0f });
	ln.Generate();

	lnVertices = ln.GetVertices();
	lnIndices = ln.GetIndices();

	lnPos1[0] = 3.0f;
	lnPos1[1] = 3.0f;

	lnBez[0] = 0.0f;
	lnBez[1] = 3.0f;



	lnLPos1[0] = 3.0f;
	lnLPos1[1] = 3.0f;

	lnLBez[0] = 0.0f;
	lnLBez[1] = 3.0f;

	// Uniforms

	glUseProgram(m_Shader->GetRendererID());
	auto loc = glGetUniformLocation(m_Shader->GetRendererID(), "u_Textures");
	int samplers[2] = { 0, 1 };
	glUniform1iv(loc, 2, samplers);

	// VAO

	glCreateVertexArrays(1, &lnVA);
	glBindVertexArray(lnVA);

	// VBO

	glCreateBuffers(1, &lnVB);
	glBindBuffer(GL_ARRAY_BUFFER, lnVB);
	glBufferData(GL_ARRAY_BUFFER, lnVertices->size() * sizeof(Vertex), lnVertices->data(), GL_STATIC_DRAW);

	// attribs

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Color));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexCoord));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexID));

	glCreateBuffers(1, &lnIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lnIB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * lnIndices->size(), lnIndices->data(), GL_STATIC_DRAW);

	//
	//
	//

	float cubeData[] = {
		0.0f, 0.0f, 10.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		10.0f, 0.0f, 10.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		10.0f, 10.0f, 10.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 10.0f, 10.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 0.0f, 20.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		10.0f, 0.0f, 20.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		10.0f, 10.0f, 20.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 10.0f, 20.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	};

	uint32_t cubeIndices[] = {
		// front
		0, 1, 2, 2, 3, 0,
		// back
		4, 5, 6, 6, 7, 4,
		// left
		0, 4, 7, 7, 3, 0,
		// right
		1, 5, 6, 6, 2, 1,
		// top
		4, 5, 1, 1, 0, 4,
		// bottom
		7, 6, 2, 2, 3, 7
	};

	glCreateVertexArrays(1, &cubeVA);
	glBindVertexArray(cubeVA);

	// VBO

	glCreateBuffers(1, &cubeVB);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVB);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(Vertex), cubeData, GL_STATIC_DRAW);

	// attribs

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Color));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexCoord));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexID));

	glCreateBuffers(1, &cubeIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 36, cubeIndices, GL_STATIC_DRAW);

	m_TexPixel = LoadTexture("assets/textures/pixel.png");
	m_TexTest = LoadTexture("assets/textures/test.png");
}

void GameLayer::OnDetach()
{
	//glDeleteVertexArrays(1, &m_QuadVA);
	//glDeleteBuffers(1, &m_QuadVB);
	//glDeleteBuffers(1, &m_QuadIB);

	glDeleteVertexArrays(1, &lnVA);
	glDeleteBuffers(1, &lnVB);
	glDeleteBuffers(1, &lnIB);
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

glm::vec4 GeneralColor = { 1.0f, 1.0f, 1.0f, 1.0f };

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

	//glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * VertexCount * VertexFloatCount * ObjectCount, m_GrandBuffer);
	//
	//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//
	//glUseProgram(m_Shader->GetRendererID());
	//glBindTextureUnit(0, m_TexPixel);
	//glBindTextureUnit(1, m_TexTest);
	//
	//int location = 0;
	//
	//location = glGetUniformLocation(m_Shader->GetRendererID(), "u_ViewProjection");
	//glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m_CameraController.GetCamera().GetViewProjectionMatrix()));
	//
	//location = glGetUniformLocation(m_Shader->GetRendererID(), "u_Color");
	//glUniform4fv(location, 1, glm::value_ptr(GeneralColor));
	//
	//glBindVertexArray(m_QuadVA);
	//glDrawElements(GL_TRIANGLES, ObjectCount * 6, GL_UNSIGNED_INT, nullptr);

	glUseProgram(m_Shader->GetRendererID());
	glBindVertexArray(lnVA);

	if (lnPos0[0] != lnLPos0[0] || lnPos0[1] != lnLPos0[1] ||
		lnPos1[0] != lnLPos1[0] || lnPos1[1] != lnLPos1[1] ||
		lnBez[0] != lnLBez[0] || lnBez[1] != lnLBez[1])
	{
		lnLPos0[0] = lnPos0[0];
		lnLPos0[1] = lnPos0[1];

		lnLPos1[0] = lnPos1[0];
		lnLPos1[1] = lnPos1[1];

		lnLBez[0] = lnBez[0];
		lnLBez[1] = lnBez[1];

		ln.SetPosition({ lnPos0[0], 0.0f, lnPos0[1] }, { lnPos1[0], 0.0f, lnPos1[1] });
		ln.SetBezier({ lnBez[0], lnBez[1] });
		ln.Generate();
		puts("regen");

		//glDeleteBuffers(1, &lnVB);
		//glDeleteBuffers(1, &lnIB);

		//glGenBuffers(GL_ARRAY_BUFFER, &lnVB);
		glBindBuffer(GL_ARRAY_BUFFER, lnVB);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * lnVertices->size(), lnVertices->data(), GL_STATIC_DRAW);

		//glGenBuffers(GL_ELEMENT_ARRAY_BUFFER, &lnIB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lnIB);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * lnIndices->size(), lnIndices->data(), GL_STATIC_DRAW);
	}

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindTextureUnit(0, m_TexPixel);
	glBindTextureUnit(1, m_TexTest);

	int location = 0;

	location = glGetUniformLocation(m_Shader->GetRendererID(), "u_ViewProjection");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m_CameraController.GetCamera().GetViewProjectionMatrix()));

	location = glGetUniformLocation(m_Shader->GetRendererID(), "u_Color");
	glUniform4fv(location, 1, glm::value_ptr(GeneralColor));

	//glBindVertexArray(lnVA);
	//glDrawElements(GL_TRIANGLES, lnIndices->size(), GL_UNSIGNED_INT, nullptr);

	glBindVertexArray(cubeVA);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}

void GameLayer::OnImGuiRender()
{
	ImGui::Begin("Lane");

	ImGui::SliderFloat2("P0", lnPos0, -10.0f, 10.0f);
	ImGui::SliderFloat2("P1", lnPos1, -10.0f, 10.0f);
	ImGui::SliderFloat2("Bez", lnBez, -10.0f, 10.0f);

	ImGui::Columns(2, "rEEEe");
	
	for (uint32_t i = 0; i < Vertex::SLOPE_DEBUG.size(); i++)
		ImGui::Text(Vertex::SLOPE_DEBUG[i].c_str());

	ImGui::NextColumn();

	for (uint32_t i = 0; i < Vertex::NV_DEBUG.size(); i++)
		ImGui::Text(Vertex::NV_DEBUG[i].c_str());

	ImGui::End();
}
