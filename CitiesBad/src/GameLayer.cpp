#include "GameLayer.h"
#include "road/Lane.h"
#include "terrain/Terrain.h"
#include "render/Quad.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <GLCore/Core/KeyCodes.h>
#include <GLCore/Core/MouseButtonCodes.h>
#include <numbers>



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace GLCore;
using namespace GLCore::Utils;

static float cameraDistance = 0.0f;
static int16_t zoomIndex = 13;
static Shader* testShader;
static Shader* laneShader;

void updateCameraDistance(Camera* camera)
{
	if (zoomIndex < 7)
		zoomIndex = 7;
	else if (zoomIndex > 20)
		zoomIndex = 20;

	cameraDistance = pow(zoomIndex, 5.0f) / 100000.0f;
	camera->SetDistance(cameraDistance);
}

GameLayer::GameLayer()
	:m_Camera(glm::radians(70.0f), 16.0f / 9.0f, 0.01f, 10000.0f, cameraDistance)
{
	using std::numbers::pi;

	updateCameraDistance(&m_Camera);

	m_Camera.SetPosition({ 1.5f, 0.0f, 1.5f });
	m_Camera.SetRotation({ pi / -4.0f, 0.0f });
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
GLuint cube2VA, cube2VB, cube2IB;

static Terrain* terrain;
static Shader* depthShader;

uint32_t depthBufferFB;
uint32_t depthBufferTexture;
const unsigned int SHADOW_WIDTH = 8192, SHADOW_HEIGHT = 8192;

float cubeData[] = {
	0.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,

	0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 1.0f
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

Quad* quad;

void GameLayer::OnAttach()
{
	Terrain::LIGHT_POS = glm::vec3(0.0f, 1.0f, 0.0f);
	Terrain::LIGHT = glm::lookAt(
		Terrain::LIGHT_POS,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	//runDecode();

	// setup

	EnableGLDebugging();

	depthShader = Shader::FromGLSLTextFiles(
		"res/shaders/depth.vert.glsl",
		"res/shaders/depth.frag.glsl"
	);

	testShader = Shader::FromGLSLTextFiles(
		"res/shaders/test.vert.glsl",
		"res/shaders/test.frag.glsl"
	);

	laneShader = Shader::FromGLSLTextFiles(
		"res/shaders/lane.vert.glsl",
		"res/shaders/lane.frag.glsl"
	);

	quad = new Quad("res/textures/test.png");
	
	// debug

	//ln.SetPosition({ 0.0f, 0.0f, 0.0f }, { 5.0f, 0.0f, 1.0f });
	//ln.SetBezier({ 2.5f, 1.0f });
	//ln.SetPosition({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 5.0f });
	//ln.SetBezier({ 0.0f, 2.5f });
	ln.SetPosition({ 0.0f, 0.0f, 0.0f }, { 3.0f, 1.0f, 3.0f });
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

	glUseProgram(laneShader->GetRendererID());

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

	auto loc = glGetUniformLocation(laneShader->GetRendererID(), "u_Textures");
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

	glUseProgram(testShader->GetRendererID());

	glCreateVertexArrays(1, &cubeVA);
	glBindVertexArray(cubeVA);

	// VBO

	glCreateBuffers(1, &cubeVB);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVB);
	glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(float), cubeData, GL_STATIC_DRAW);

	// attribs

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	glCreateBuffers(1, &cubeIB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 36, cubeIndices, GL_STATIC_DRAW);

	//
	//
	//

	//float cube2Data[] = {
	//	0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
	//	1.0f, 0.0f, 0.0f, 0.8f, 0.8f, 0.8f, 1.0f, 1.0f, 0.0f, 0.0f,
	//	1.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f,
	//	0.0f, 1.0f, 0.0f, 0.3f, 0.3f, 0.3f, 1.0f, 0.0f, 1.0f, 0.0f,
	//
	//	0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
	//	1.0f, 0.0f, 1.0f, 1.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	//	1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	//	0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f,
	//};
	//
	//uint32_t cube2Indices[] = {
	//	// front
	//	0, 1, 2, 2, 3, 0,
	//	// back
	//	4, 5, 6, 6, 7, 4,
	//	// left
	//	0, 4, 7, 7, 3, 0,
	//	// right
	//	1, 5, 6, 6, 2, 1,
	//	// top
	//	4, 5, 1, 1, 0, 4,
	//	// bottom
	//	7, 6, 2, 2, 3, 7
	//};
	//
	//glCreateVertexArrays(1, &cube2VA);
	//glBindVertexArray(cube2VA);
	//
	//// VBO
	//
	//glCreateBuffers(1, &cube2VB);
	//glBindBuffer(GL_ARRAY_BUFFER, cube2VB);
	//glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(Vertex), cube2Data, GL_STATIC_DRAW);
	//
	//// attribs
	//
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Position));
	//
	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Color));
	//
	//glEnableVertexAttribArray(2);
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexCoord));
	//
	//glEnableVertexAttribArray(3);
	//glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexID));
	//
	//glCreateBuffers(1, &cube2IB);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube2IB);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 36, cube2Indices, GL_STATIC_DRAW);

	terrain = new Terrain({ 100, 100 }, 0.05f);

	m_TexPixel = LoadTexture("res/textures/pixel.png");
	m_TexTest = LoadTexture("res/textures/test.png");

	glGenFramebuffers(1, &depthBufferFB);
	glGenTextures(1, &depthBufferTexture);
	glBindTexture(GL_TEXTURE_2D, depthBufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthBufferFB);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBufferTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GameLayer::OnDetach()
{
	//glDeleteVertexArrays(1, &m_QuadVA);
	//glDeleteBuffers(1, &m_QuadVB);
	//glDeleteBuffers(1, &m_QuadIB);

	//glDeleteVertexArrays(1, &lnVA);
	//glDeleteBuffers(1, &lnVB);
	//glDeleteBuffers(1, &lnIB);
}

static bool movLeft = false;
static bool movRight = false;
static bool movForwards = false;
static bool movBackwards = false;

static bool movUp = false;
static bool movDown = false;

static bool rotXD = false;
static bool rotXU = false;
static bool rotYD = false;
static bool rotYU = false;

static bool mouseDragging = false;
static glm::vec2 mousePosition = { 0.0f, 0.0f };
static glm::vec2 mouseBegin = { 0.0f, 0.0f };
static glm::vec2 rotationBegin = { 0.0f, 0.0f };

void GameLayer::OnEvent(Event& event)
{
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
				movForwards = true;
			if (e.GetKeyCode() == HZ_KEY_S)
				movBackwards = true;
			if (e.GetKeyCode() == HZ_KEY_Q)
				movUp = true;
			if (e.GetKeyCode() == HZ_KEY_E)
				movDown = true;

			if (e.GetKeyCode() == HZ_KEY_UP)
				rotXD = true;
			if (e.GetKeyCode() == HZ_KEY_DOWN)
				rotXU = true;
			if (e.GetKeyCode() == HZ_KEY_LEFT)
				rotYD = true;
			if (e.GetKeyCode() == HZ_KEY_RIGHT)
				rotYU = true;

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
				movForwards = false;
			if (e.GetKeyCode() == HZ_KEY_S)
				movBackwards = false;
			if (e.GetKeyCode() == HZ_KEY_Q)
				movUp = false;
			if (e.GetKeyCode() == HZ_KEY_E)
				movDown = false;

			if (e.GetKeyCode() == HZ_KEY_UP)
				rotXD = false;
			if (e.GetKeyCode() == HZ_KEY_DOWN)
				rotXU = false;
			if (e.GetKeyCode() == HZ_KEY_LEFT)
				rotYD = false;
			if (e.GetKeyCode() == HZ_KEY_RIGHT)
				rotYU = false;

			return false;
		});
	dispatcher.Dispatch<MouseMovedEvent>(
		[&](MouseMovedEvent& e)
		{
			mousePosition = { e.GetX(), e.GetY() };

			return false;
		});
	dispatcher.Dispatch<MouseButtonPressedEvent>(
		[&](MouseButtonPressedEvent& e)
		{
			if (e.GetMouseButton() == HZ_MOUSE_BUTTON_3)
			{
				mouseDragging = true;
				mouseBegin = mousePosition;
				rotationBegin = m_Camera.GetRotation();
			}

			return false;
		});
	dispatcher.Dispatch<MouseButtonReleasedEvent>(
		[&](MouseButtonReleasedEvent& e)
		{
			if (e.GetMouseButton() == HZ_MOUSE_BUTTON_3)
			{
				mouseDragging = false;
				mouseBegin = { 0.0f, 0.0f };
				rotationBegin = { 0.0f, 0.0f };
			}

			return false;
		});
	dispatcher.Dispatch<MouseScrolledEvent>(
		[&](MouseScrolledEvent& e)
		{
			zoomIndex -= e.GetYOffset();
			updateCameraDistance(&m_Camera);

			return false;
		});
}

glm::vec4 GeneralColor = { 1.0f, 1.0f, 1.0f, 1.0f };

float obj1Pos[3] = { 2.0f, 0.0f, 0.0f };
float obj1Rot[3] = { 0.0f, 0.0f, 0.0f };

float obj2Pos[3] = { -2.0f, 0.0f, 0.0f };
float obj2Rot[3] = { 0.0f, 0.0f, 0.0f };

float camPos[3] = { 0.0f, 0.0f, -10.0f };
float camRot[3] = { 0.0f, 0.0f, 0.0f };

//void CreateMVP(glm::vec3 objectPosition, glm::vec3 objectRotation, glm::vec3 cameraPosition, glm::vec3 cameraRotation, GLuint locationMVP)
//{
//	glm::mat4 Model = glm::mat4(1.0f);
//	glm::mat4 View = glm::mat4(1.0f);
//	glm::mat4 Projection = glm::mat4(1.0f);
//
//	// Model
//
//	Model = glm::translate(Model, glm::vec3(1.0f, 1.0f, -1.0f) * objectPosition);
//	Model = glm::rotate(Model, objectRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
//	Model = glm::rotate(Model, objectRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
//	Model = glm::rotate(Model, objectRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
//
//	// View
//
//	View = glm::rotate(View, cameraRotation.x, glm::vec3(-1.0f, 0.0f, 0.0f));
//	View = glm::rotate(View, cameraRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
//	View = glm::rotate(View, cameraRotation.z, glm::vec3(0.0f, 0.0f, -1.0f));
//	View = glm::translate(View, glm::vec3(-1.0f, -1.0f, 1.0f) * cameraPosition);
//
//	// Projection
//
//	Projection = glm::perspective(glm::radians(90.0f), 16.0f / 9.0f, 0.1f, 10000.f);
//
//	glm::mat4 MVP = Projection * View * Model;
//	glUniformMatrix4fv(locationMVP, 1, GL_FALSE, glm::value_ptr(MVP));
//}

static float movSpeed = 0.0f;
static glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

void renderScene(Camera& camera, Timestep ts)
{
	float terrainSize = static_cast<float>(terrain->GetSize().x) * static_cast<float>(terrain->GetCellSize());
	float near_plane = -terrainSize, far_plane = terrainSize;
	float scale = terrainSize;

	std::chrono::milliseconds ms = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	float lz = sinl(static_cast<double>(ms.count()) / 1000.0) * 5;
	
	Terrain::LIGHT_POS.x = terrainSize * 0.5;
	Terrain::LIGHT_POS.z = (terrainSize * 0.5) + lz;
	Terrain::LIGHT = glm::lookAt(
		Terrain::LIGHT_POS,
		glm::vec3(terrainSize * 0.5, 0.0f, terrainSize * 0.5),
		glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 lightProjection = glm::ortho(scale, -scale, scale, -scale, near_plane, far_plane);
	glm::mat4 lightSpaceMatrix = lightProjection * Terrain::LIGHT;

	terrain->SetLightProjection(lightProjection);
	
	// Render Depth Buffer

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glCullFace(GL_FRONT);
	
	glUseProgram(depthShader->GetRendererID());
	glUniformMatrix4fv(glGetUniformLocation(depthShader->GetRendererID(), "u_VP"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

	glUniformMatrix4fv(glGetUniformLocation(depthShader->GetRendererID(), "u_M"), 1, GL_FALSE, glm::value_ptr(camera.ConstructModel(
		glm::vec3(0.0f),
		glm::vec3(0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
	)));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthBufferFB);
	glClear(GL_DEPTH_BUFFER_BIT);
	//glCullFace(GL_BACK);
	terrain->Render(camera, ts, depthShader);
	
	{
		//glUniform4f(glGetUniformLocation(depthShader->GetRendererID(), "u_Color"), 0.8f, 0.8f, 0.8f, 1.0f);
		//glUniformMatrix4fv(glGetUniformLocation(depthShader->GetRendererID(), "u_MVP"), 1, GL_FALSE, glm::value_ptr(camera.ConstructMVP(
		//	glm::vec3(obj1Pos[0], obj1Pos[1], obj1Pos[2]),
		//	glm::vec3(obj1Rot[0], obj1Rot[1], obj1Rot[2]),
		//	glm::vec3(1.0f, 1.0f, 1.0f)
		//)));

		glUniformMatrix4fv(glGetUniformLocation(depthShader->GetRendererID(), "u_M"), 1, GL_FALSE, glm::value_ptr(camera.ConstructModel(
			glm::vec3(obj1Pos[0], obj1Pos[1], obj1Pos[2]),
			glm::vec3(obj1Rot[0], obj1Rot[1], obj1Rot[2]),
			glm::vec3(1.0f)
		)));
	
		glBindVertexArray(cubeVA);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, 1920, 1080);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Full Render

	glBindTextureUnit(0, depthBufferTexture);
	
	terrain->Render(camera, ts);

	{
		glUseProgram(testShader->GetRendererID());

		glUniform4f(glGetUniformLocation(testShader->GetRendererID(), "u_Color"), 0.8f, 0.8f, 0.8f, 1.0f);
		glUniformMatrix4fv(glGetUniformLocation(testShader->GetRendererID(), "u_MVP"), 1, GL_FALSE, glm::value_ptr(camera.ConstructMVP(
			glm::vec3(obj1Pos[0], obj1Pos[1], obj1Pos[2]),
			glm::vec3(obj1Rot[0], obj1Rot[1], obj1Rot[2]),
			glm::vec3(1.0f)
		)));

		glBindVertexArray(cubeVA);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
	}

	quad->Render(camera, depthBufferTexture);
}

void GameLayer::OnUpdate(Timestep ts)
{
	using std::numbers::pi;

	m_Camera.FrameUpdate(ts);
	movSpeed = ts * cameraDistance * 1.5f;
	
	if (movForwards)
		m_Camera.Translate(movSpeed * m_Camera.GetLocationForward());
	if (movBackwards)
		m_Camera.Translate(-movSpeed * m_Camera.GetLocationForward());
	if (movLeft)
		m_Camera.Translate(-movSpeed * glm::normalize(glm::cross(m_Camera.GetLocationForward(), cameraUp)));
	if (movRight)
		m_Camera.Translate(movSpeed * glm::normalize(glm::cross(m_Camera.GetLocationForward(), cameraUp)));
	
	if (mouseDragging)
	{
		m_Camera.SetRotation({
			rotationBegin.x + ((mouseBegin.y - mousePosition.y) / 100.0f),
			rotationBegin.y + ((mouseBegin.x - mousePosition.x) / -100.0f)
			});
	}
	
	//glUseProgram(m_Shader->GetRendererID());
	//
	//auto loc = glGetUniformLocation(m_Shader->GetRendererID(), "u_Textures");
	//int samplers[2] = { 0, 1 };
	//glUniform1iv(loc, 2, samplers);
	//
	//glBindVertexArray(lnVA);
	//
	//if (lnPos0[0] != lnLPos0[0] || lnPos0[1] != lnLPos0[1] ||
	//	lnPos1[0] != lnLPos1[0] || lnPos1[1] != lnLPos1[1] ||
	//	lnBez[0] != lnLBez[0] || lnBez[1] != lnLBez[1])
	//{
	//	lnLPos0[0] = lnPos0[0];
	//	lnLPos0[1] = lnPos0[1];
	//
	//	lnLPos1[0] = lnPos1[0];
	//	lnLPos1[1] = lnPos1[1];
	//
	//	lnLBez[0] = lnBez[0];
	//	lnLBez[1] = lnBez[1];
	//
	//	ln.SetPosition({ lnPos0[0], 0.0f, lnPos0[1] }, { lnPos1[0], 1.0f, lnPos1[1] });
	//	ln.SetBezier({ lnBez[0], lnBez[1] });
	//	ln.Generate();
	//
	//	//glDeleteBuffers(1, &lnVB);
	//	//glDeleteBuffers(1, &lnIB);
	//
	//	//glGenBuffers(GL_ARRAY_BUFFER, &lnVB);
	//	glBindBuffer(GL_ARRAY_BUFFER, lnVB);
	//	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * lnVertices->size(), lnVertices->data(), GL_STATIC_DRAW);
	//
	//	//glGenBuffers(GL_ELEMENT_ARRAY_BUFFER, &lnIB);
	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lnIB);
	//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * lnIndices->size(), lnIndices->data(), GL_STATIC_DRAW);
	//}

	//glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	//glBindFramebuffer(GL_FRAMEBUFFER, depthBufferFB);
	//
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//
	//renderScene(m_Camera, ts);
	//
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glViewport(0, 0, 1920, 1080);
	//
	//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//
	//glBindFramebuffer(GL_TEXTURE_2D, depthBufferTexture);
	//
	//renderScene(m_Camera, ts);



	renderScene(m_Camera, ts);

	//glBindTextureUnit(0, m_TexPixel);
	//glBindTextureUnit(1, m_TexTest);

	//int location = 0;
	//
	//location = glGetUniformLocation(m_Shader->GetRendererID(), "u_Color");
	//glUniform4fv(location, 1, glm::value_ptr(GeneralColor));
	//
	//glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetRendererID(), "u_MVP"), 1, GL_FALSE, glm::value_ptr(m_Camera.ConstructMVP(
	//	glm::vec3(obj1Pos[0], obj1Pos[1], obj1Pos[2]),
	//	glm::vec3(obj1Rot[0], obj1Rot[1], obj1Rot[2]),
	//	glm::vec3(1.0f, 1.0f, 1.0f)
	//)));
	
	//glBindVertexArray(lnVA);
	//glDrawElements(GL_TRIANGLES, lnIndices->size(), GL_UNSIGNED_INT, nullptr);

	//glBindTextureUnit(0, m_TexPixel);
	//glBindTextureUnit(1, m_TexTest);
	//
	//glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetRendererID(), "u_MVP"), 1, GL_FALSE, glm::value_ptr(m_Camera.ConstructMVP(
	//	glm::vec3(obj1Pos[0], obj1Pos[1], obj1Pos[2]),
	//	glm::vec3(obj1Rot[0], obj1Rot[1], obj1Rot[2]),
	//	glm::vec3(1.0f, 1.0f, 1.0f)
	//)));
	//
	//glBindVertexArray(cubeVA);
	//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
	//
	//glBindTextureUnit(0, m_TexPixel);
	//glBindTextureUnit(1, m_TexTest);
	//
	//glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetRendererID(), "u_MVP"), 1, GL_FALSE, glm::value_ptr(m_Camera.ConstructMVP(
	//	glm::vec3(obj2Pos[0], obj2Pos[1], obj2Pos[2]),
	//	glm::vec3(obj2Rot[0], obj2Rot[1], obj2Rot[2]),
	//	glm::vec3(1.0f, 1.0f, 1.0f)
	//)));
	//
	//glBindVertexArray(cube2VA);
	//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

	//terrain->Render(m_Camera, ts);
}

void GameLayer::OnImGuiRender()
{
	using std::numbers::pi;

	ImGui::Begin("Lane");

	//ImGui::SliderFloat3("CAM position", camPos, -100.0f, 100.0f);
	//ImGui::SliderFloat3("CAM rotation", camRot, -pi, pi);
	//ImGui::NewLine();

	ImGui::SliderFloat3("OBJ1 position", obj1Pos, -50.0f, 50.0f);
	ImGui::SliderFloat3("OBJ1 rotation", obj1Rot, -pi, pi);
	//ImGui::NewLine();
	//
	//ImGui::SliderFloat3("OBJ2 position", obj2Pos, -100.0f, 100.0f);
	//ImGui::SliderFloat3("OBJ2 rotation", obj2Rot, -pi, pi);
	//ImGui::NewLine();

	//ImGui::SliderFloat2("P0", lnPos0, -10.0f, 10.0f);
	//ImGui::SliderFloat2("P1", lnPos1, -10.0f, 10.0f);
	//ImGui::SliderFloat2("Bez", lnBez, -10.0f, 10.0f);

	//ImGui::Columns(2, "rEEEe");
	//
	//for (uint32_t i = 0; i < Vertex::SLOPE_DEBUG.size(); i++)
	//	ImGui::Text(Vertex::SLOPE_DEBUG[i].c_str());
	//
	//ImGui::NextColumn();
	//
	//for (uint32_t i = 0; i < Vertex::NV_DEBUG.size(); i++)
	//	ImGui::Text(Vertex::NV_DEBUG[i].c_str());

	ImGui::End();
}