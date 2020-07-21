#include "Terrain.h"

#include <noise/noise.h>
#include <noise/noiseutils.h>
#include <random>

using namespace noise;

struct TerrainVertex
{
	TerrainVertex()
		:Position({ 0.0f, 0.0f, 0.0f }), Color({ 1.0f, 1.0f, 1.0f, 1.0f }) {}

	TerrainVertex(glm::vec3 position, glm::vec4 color)
		:Position(position), Color(color) {}

	glm::vec3 Position;
	glm::vec4 Color;
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

	glUseProgram(m_Shader->GetRendererID());

	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetRendererID(), "u_MVP"), 1, GL_FALSE, glm::value_ptr(camera.ConstructMVP(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
	)));

	glBindVertexArray(m_VA);
	glDrawElements(GL_TRIANGLES, 6 * (m_Size.x - 1) * (m_Size.y - 1), GL_UNSIGNED_INT, nullptr);
}

void Terrain::BufferDefault()
{
	if (!m_Setup)
		return;

	std::mt19937 gen;
	std::uniform_real_distribution dis = std::uniform_real_distribution(0.6, 1.0);

	module::Perlin perlin;

	for (uint64_t i = 0; i < m_Size.x * m_Size.y; i++)
	{
		glm::vec3 offset = glm::vec3(
			m_CellSize * (i % m_Size.x),
			(float)perlin.GetValue((float)(i % m_Size.x) / 500.0f, 0.124f, (float)floor((float)i / (float)m_Size.x) / 500.0f),
			m_CellSize * floor(i / m_Size.x)
		);

		TerrainVertex vertex = TerrainVertex(offset, { 1.0f, dis(gen), 0.0f, 1.0f });

		memcpy(m_Buffer + (i * 7), &vertex, sizeof(TerrainVertex));
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
