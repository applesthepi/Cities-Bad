#include "Terrain.h"

#include <noise/noise.h>
#include <noise/noiseutils.h>
#include <random>
#include <chrono>
#include <stb_image.h>

using namespace noise;

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

struct TerrainVertex
{
	TerrainVertex()
		:Position({ 0.0f, 0.0f, 0.0f }), Color({ 1.0f, 1.0f, 1.0f, 1.0f }), Normal({ 0.0f, 1.0f, 0.0f }), ShadowCoords({ 0.0f, 0.0f }), TextureCoords({ 0.0f, 0.0f }) {}

	TerrainVertex(glm::vec3 position, glm::vec4 color, glm::vec2 textureCoords)
		:Position(position), Color(color), Normal({ 0.0f, 1.0f, 0.0f }), ShadowCoords({ 0.0f, 0.0f }), TextureCoords(textureCoords) {}

	glm::vec3 Position;
	glm::vec4 Color;
	glm::vec3 Normal;
	glm::vec2 ShadowCoords;
	glm::vec2 TextureCoords;
};

static size_t terrainCellSize = sizeof(TerrainVertex) * 4;
static size_t terrainCellFloatCount = terrainCellSize / sizeof(float);

Terrain::Terrain(glm::vec<2, uint32_t> size, float cellSize)
	:m_VA(0), m_VB(0), m_IB(0), m_Setup(false), m_CellSize(cellSize), m_Size(size)
{
	m_Shader = Shader::FromGLSLTextFiles(
		"res/shaders/terrain.vert.glsl",
		"res/shaders/terrain.frag.glsl"
	);

	m_Setup = m_Shader->GetRendererID() > 0;

	if (!m_Setup)
	{
		LOG_ERROR("failed to setup terrain");
		return;
	}

	m_TextureDiffuse = LoadTexture("res/textures/GroundForest_DIF.jpg");
	m_TextureNormal = LoadTexture("res/textures/GroundForest_NOR.jpg");
	
	m_Buffer = (float*)malloc(sizeof(TerrainVertex) * size.x * size.y);
	
	if (m_Buffer == nullptr)
	{
		LOG_ERROR("out of memory");
		m_Setup = false;
		return;
	}

	BufferDefault();

	uint32_t* indices = (uint32_t*)malloc(sizeof(uint32_t) * (size.x - 1) * (size.y - 1) * 6);

	if (indices == nullptr)
	{
		LOG_ERROR("out of memory");
		m_Setup = false;
		return;
	}

	for (uint64_t y = 0; y < (size.y - 1); y++)
	{
		for (uint64_t x = 0; x < (size.x - 1); x++)
		{
			uint64_t itile = ((y * (size.x - 1)) + x);
			uint64_t vtile = ((y * size.x) + x);

			indices[itile * 6 + 0] = vtile;
			indices[itile * 6 + 1] = vtile + 1;
			indices[itile * 6 + 2] = vtile + m_Size.x + 1;
			indices[itile * 6 + 3] = vtile + m_Size.x + 1;
			indices[itile * 6 + 4] = vtile + m_Size.x;
			indices[itile * 6 + 5] = vtile;
		}
	}

	glUseProgram(m_Shader->GetRendererID());
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUniform4f(
		glGetUniformLocation(m_Shader->GetRendererID(), "u_Color"),
		1.0f, 1.0f, 1.0f, 1.0f
	);

	glCreateVertexArrays(1, &m_VA);
	glBindVertexArray(m_VA);

	glCreateBuffers(1, &m_VB);
	glBindBuffer(GL_ARRAY_BUFFER, m_VB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TerrainVertex) * size.x * size.y, m_Buffer, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (const void*)offsetof(TerrainVertex, Position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (const void*)offsetof(TerrainVertex, Color));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (const void*)offsetof(TerrainVertex, Normal));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (const void*)offsetof(TerrainVertex, ShadowCoords));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (const void*)offsetof(TerrainVertex, TextureCoords));

	glCreateBuffers(1, &m_IB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6 * (m_Size.x - 1) * (m_Size.y - 1), indices, GL_STATIC_DRAW);

	free(indices);
}

Terrain::~Terrain()
{
	if (!m_Setup)
		return;

	glDeleteVertexArrays(1, &m_VA);
	glDeleteBuffers(1, &m_VB);
	glDeleteBuffers(1, &m_IB);

	free(m_Buffer);
	delete m_Shader;
}

void Terrain::Render(Camera& camera, Timestep ts)
{
	if (!m_Setup)
		return;

	glm::mat4 light = m_LightProjection * LIGHT;

	glUseProgram(m_Shader->GetRendererID());

	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetRendererID(), "u_Projection"), 1, GL_FALSE, glm::value_ptr(camera.GetProjection()));
	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetRendererID(), "u_View"), 1, GL_FALSE, glm::value_ptr(camera.GetView()));
	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetRendererID(), "u_Model"), 1, GL_FALSE, glm::value_ptr(camera.ConstructModel(
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f }
	)));
	glUniform1i(glGetUniformLocation(m_Shader->GetRendererID(), "u_ShadowTexture"), 0);
	glUniform1i(glGetUniformLocation(m_Shader->GetRendererID(), "u_TextureDiffuse"), 1);
	glUniform1i(glGetUniformLocation(m_Shader->GetRendererID(), "u_TextureNormal"), 2);
	glUniform1f(glGetUniformLocation(m_Shader->GetRendererID(), "u_HalfWidth"), m_Size.x * 0.5f * m_CellSize);
	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetRendererID(), "u_Light"), 1, GL_FALSE, glm::value_ptr(light));
	glUniform3f(glGetUniformLocation(m_Shader->GetRendererID(), "u_LightPos"), LIGHT_POS.x, LIGHT_POS.y, LIGHT_POS.z);
	glUniform3f(glGetUniformLocation(m_Shader->GetRendererID(), "u_ViewPos"), camera.GetRealPosition().x, camera.GetRealPosition().y, camera.GetRealPosition().z);

	//glUniform3f(glGetUniformLocation(m_Shader->GetRendererID(), "u_LightDirection"), 0.5f, 0.3f, 0.5f);
	//glUniform3f(glGetUniformLocation(m_Shader->GetRendererID(), "u_LightColor"), 1.0f, 1.0f, 0.8f);
	//glUniform4f(glGetUniformLocation(m_Shader->GetRendererID(), "u_Color"), 1.0f, 1.0f, 1.0f, 1.0f);

	glBindTextureUnit(1, m_TextureDiffuse);
	glBindTextureUnit(2, m_TextureNormal);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	RenderSelf(camera, ts);
}

void Terrain::Render(Camera& camera, Timestep ts, Shader* shader)
{
	if (!m_Setup)
		return;

	//std::chrono::milliseconds ms = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	//float lz = sinl(static_cast<double>(ms.count()) / 1000.0) * 5;
	
	//LIGHT_POS.z = lz;

	//LIGHT = glm::lookAt(
	//	LIGHT_POS,
	//	glm::vec3(0.0f, 0.0f, 0.0f),
	//	glm::vec3(0.0f, 1.0f, 0.0f));
	
	glUseProgram(shader->GetRendererID());

	glBindBuffer(GL_ARRAY_BUFFER, m_VB);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (const void*)offsetof(TerrainVertex, Position));

	RenderSelf(camera, ts);
}

void Terrain::SetLightProjection(glm::mat4 projection)
{
	m_LightProjection = projection;
}

glm::vec<2, glm::uint32_t> Terrain::GetSize()
{
	return m_Size;
}

float Terrain::GetCellSize()
{
	return m_CellSize;
}

glm::vec3 Terrain::LIGHT_POS;

glm::mat4 Terrain::LIGHT;

void Terrain::RenderSelf(Camera& camera, Timestep ts)
{
	glBindVertexArray(m_VA);
	glDrawElements(GL_TRIANGLES, 6 * (m_Size.x - 1) * (m_Size.y - 1), GL_UNSIGNED_INT, nullptr);
}

void Terrain::BufferDefault()
{
	if (!m_Setup)
		return;

	std::mt19937 gen;
	std::uniform_real_distribution dis = std::uniform_real_distribution(0.5, 0.6);

	module::Perlin perlinGenerator;

	glm::vec3 colorTip = { 0.5f, 0.5f, 0.5f };
	glm::vec3 colorThick = { -0.05f, -0.03f, -0.05f };
	glm::vec3 colorBase = { 0.1f, 0.2f, 0.1f };

	for (uint64_t i = 0; i < m_Size.x * m_Size.y; i++)
	{
		//double perlinTerrain = perlinGenerator.GetValue((float)(i % m_Size.x) / 500.0f, 1.0f, (float)floor((float)i / (float)m_Size.x) / 500.0f) / 0.5f;
		double perlinTerrain = perlinGenerator.GetValue((float)(i % m_Size.x) / 500.0f, 1.0f, (float)floor((float)i / (float)m_Size.x) / 500.0f) / 0.3f;
		double perlinTip = perlinGenerator.GetValue((float)(i % m_Size.x) / 500.0f, 1.0f, (float)floor((float)i / (float)m_Size.x) / 500.0f) / 5.0f;
		double perlinThick = (1.0f - perlinGenerator.GetValue((float)(i % m_Size.x) / 500.0f, 1.0f, (float)floor((float)i / (float)m_Size.x) / 500.0f) / 1.0f) - 1.0f;
		double perlinGrass = (perlinGenerator.GetValue((float)(i % m_Size.x) / 5.123f, 1.0f, (float)floor((float)i / (float)m_Size.x) / 5.123f) / 100.0f + 0.15f) / 2.0f;
		double perlinGrassLow = (perlinGenerator.GetValue((float)(i % m_Size.x) / 100.123f, 1.0f, (float)floor((float)i / (float)m_Size.x) / 100.123f) / 50.0f + 0.0f) / 1.0f;

		perlinTip = glm::clamp(static_cast<float>(perlinTip), 0.0f, 1.0f);
		perlinThick = glm::clamp(static_cast<float>(perlinThick), 0.0f, 1.0f);
		perlinGrass = glm::clamp(static_cast<float>(perlinGrass), 0.0f, 1.0f);
		perlinGrass = glm::clamp(static_cast<float>(perlinGrassLow), 0.0f, 1.0f);

		glm::vec3 offset;

		if (i % m_Size.x == 0 || floor(i / m_Size.x) == 0 ||
			i % m_Size.x == m_Size.x - 1 || floor(i / m_Size.x) == m_Size.y - 1)
		{
			offset = glm::vec3(
				m_CellSize * (i % m_Size.x),
				-10.0f,
				m_CellSize * floor(i / m_Size.x)
			);
		}
		else
		{
			offset = glm::vec3(
				m_CellSize * (i % m_Size.x),
				static_cast<float>(perlinTerrain),
				m_CellSize * floor(i / m_Size.x)
			);
		}

		//float sinCoords = sinf(static_cast<float>(i) * 0.01f) * 0.5f + 0.5f;
		//float cosCoords = sinf(static_cast<float>(i % m_Size.x) * 1.0f) * 0.5f + 0.5f;

		glm::vec3 thisColor = colorBase + (static_cast<float>(perlinTip) * colorTip) + (static_cast<float>(perlinThick) * colorThick) - static_cast<float>(perlinGrass + perlinGrassLow);
		TerrainVertex vertex = TerrainVertex(offset, glm::vec4(thisColor, 1.0f), glm::vec2(
			(static_cast<float>(abs(static_cast<int>(i % (m_Size.x * 2)) - static_cast<int>(m_Size.x))) / static_cast<float>(m_Size.x)),
			abs((static_cast<int>(i % (m_Size.x / 5)) / static_cast<int>(m_Size.x / 5)) - static_cast<int>(m_Size.x / 10)) / static_cast<float>(m_Size.x / 10)
		));

		//printf("%f\n", static_cast<float>(i % (m_Size.x / 5)) / static_cast<float>(m_Size.x / 5));

		//if (i % m_Size.x == 0)
			//printf("%f\n", static_cast<float>(abs(static_cast<int>(i % (m_Size.x * 2)) - static_cast<int>(m_Size.x))) / static_cast<float>(m_Size.x));
		//std::cout << static_cast<float>(i % m_Size.x) / 8192.0f << std::endl;

		memcpy(m_Buffer + (i * (sizeof(TerrainVertex) / sizeof(float))), &vertex, sizeof(TerrainVertex));
	}

	for (uint64_t y = 0; y < (m_Size.y - 1); y++)
	{
		for (uint64_t x = 0; x < (m_Size.x - 1); x++)
		{
			uint64_t vtile = ((y * m_Size.x) + x);

			glm::vec3 side1 = {
				m_Buffer[(vtile + m_Size.x) * (sizeof(TerrainVertex) / sizeof(float))],
				m_Buffer[(vtile + m_Size.x) * (sizeof(TerrainVertex) / sizeof(float)) + 1],
				m_Buffer[(vtile + m_Size.x) * (sizeof(TerrainVertex) / sizeof(float)) + 2]
			};

			glm::vec3 side2 = {
				m_Buffer[(vtile + m_Size.x + 1) * (sizeof(TerrainVertex) / sizeof(float))],
				m_Buffer[(vtile + m_Size.x + 1) * (sizeof(TerrainVertex) / sizeof(float)) + 1],
				m_Buffer[(vtile + m_Size.x + 1) * (sizeof(TerrainVertex) / sizeof(float)) + 2]
			};

			side1 -= glm::vec3(
				m_Buffer[(vtile) * (sizeof(TerrainVertex) / sizeof(float))],
				m_Buffer[(vtile) * (sizeof(TerrainVertex) / sizeof(float)) + 1],
				m_Buffer[(vtile) * (sizeof(TerrainVertex) / sizeof(float)) + 2]
			);

			side2 -= glm::vec3(
				m_Buffer[(vtile) * (sizeof(TerrainVertex) / sizeof(float))],
				m_Buffer[(vtile) * (sizeof(TerrainVertex) / sizeof(float)) + 1],
				m_Buffer[(vtile) * (sizeof(TerrainVertex) / sizeof(float)) + 2]
			);

			glm::vec3 normal = glm::cross(side1, side2);

			glm::vec2 shadowCoords = {
				1.0f - static_cast<float>(x) / (static_cast<float>(m_Size.x) - 1.0f),
				1.0f - static_cast<float>(y) / (static_cast<float>(m_Size.y) - 1.0f)
			};

			memcpy(m_Buffer + (vtile * (sizeof(TerrainVertex) / sizeof(float))) + 7, &normal, sizeof(glm::vec3));
			memcpy(m_Buffer + (vtile * (sizeof(TerrainVertex) / sizeof(float))) + 10, &shadowCoords, sizeof(glm::vec2));
		}
	}
}

void Terrain::BufferPosition(glm::vec<2, uint32_t> vertex, glm::vec3 position)
{
	if (!m_Setup)
		return;
}

void Terrain::BufferColor(glm::vec<2, uint32_t> vertex, glm::vec4 color)
{
	if (!m_Setup)
		return;
}
