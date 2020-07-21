#include "Lane.h"

#include <cmath>
#include <numbers>

//#define HELP_ME
//#define RIBBON_ME

float DEG_RAD = (std::numbers::pi / 180.0f);
float RAD_DEG = (180.0f / std::numbers::pi);

Lane::Lane()
{
	m_P1 = { 0, 0, 0 };
	m_P2 = { 0, 0, 0 };

	m_B = { 0, 0 };
	m_Length = 0.0f;

	// TODO OH GOD PLEASE FIX ME OH PEASE NO OH GOD PLEASE
	m_Length = 10.0f;
}

void Lane::SetPosition(glm::vec3 p1, glm::vec3 p2)
{
	m_P1 = p1;
	m_P2 = p2;
}

void Lane::SetBezier(glm::vec2 b)
{
	m_B = b;
}

void Lane::Generate()
{
	Vertex::SLOPE_DEBUG.clear();
	Vertex::NV_DEBUG.clear();
	m_Vertices.clear();
	m_Indices.clear();

	float a = glm::distance(glm::vec2(m_P2.x, m_P2.z), m_B);
	float b = glm::distance(glm::vec2(m_P1.x, m_P1.z), glm::vec2(m_P2.x, m_P2.z));
	float c = glm::distance(glm::vec2(m_P1.x, m_P1.z), m_B);

	std::vector<glm::vec3> centerPoints;
	float sss = (pow(c, 2) + pow(a, 2) - pow(b, 2)) / (2 * c * a);

	sss = 180.0f - glm::degrees(acos(sss));

	uint32_t resolution = floor(sss / 5.0f);
	if (resolution < 30)
		resolution = 30;

	float highestAngle = 1.0f;

	for (uint32_t i = 0; i < resolution + 1; i++)
	{
		double t = static_cast<double>(i) / static_cast<double>(resolution);

		double t2 = pow(t, 2);
		double t2i = pow(1.0f - t, 2);

		glm::vec3 point = (glm::vec3(t2i) * m_P1) + glm::vec3(
			2 * (1.0f - t) * (t * m_B.x),
			0.0f,
			2 * (1.0f - t) * (t * m_B.y)
		) + (glm::vec3(t2) * m_P2);

		point.y = glm::mix(m_P1.y, m_P2.y, static_cast<float>(t));
		centerPoints.push_back(point);
	}

	std::vector<glm::vec3> leftPoints;
	std::vector<glm::vec3> rightPoints;

	{
		std::pair<glm::vec2, glm::vec2> result = GenerateExteriorCorner(
			glm::vec2(centerPoints[1].x, centerPoints[1].z),
			glm::vec2(centerPoints[0].x, centerPoints[0].z));

		leftPoints.push_back({
			result.first.x,
			centerPoints.front().y,
			result.first.y
		});

		rightPoints.push_back({
			result.second.x,
			centerPoints.front().y,
			result.second.y
		});
	}

	for (uint32_t i = 1; i < centerPoints.size() - 1; i++)
	{
		std::pair<glm::vec2, glm::vec2> result = GenerateExterior(
			glm::vec2(centerPoints[i - 1].x, centerPoints[i - 1].z),
			glm::vec2(centerPoints[i].x, centerPoints[i].z),
			glm::vec2(centerPoints[i + 1].x, centerPoints[i + 1].z));

		leftPoints.push_back({
			result.first.x,
			centerPoints[i].y,
			result.first.y
		});

		rightPoints.push_back({
			result.second.x,
			centerPoints[i].y,
			result.second.y
		});
	}

	{
		// jank things have been acquired

		std::pair<glm::vec2, glm::vec2> result = GenerateExteriorCorner(
			glm::vec2(centerPoints[centerPoints.size() - 2].x, centerPoints[centerPoints.size() - 2].z),
			glm::vec2(centerPoints[centerPoints.size() - 1].x, centerPoints[centerPoints.size() - 1].z));

		leftPoints.push_back({
			result.second.x,
			centerPoints.back().y,
			result.second.y
			});

		rightPoints.push_back({
			result.first.x,
			centerPoints.back().y,
			result.first.y
			});
	}

	m_Vertices.clear();
	m_Vertices.reserve(leftPoints.size() + rightPoints.size() + 2);

	// ==========================================================================================
	// Add Vertices
	// ==========================================================================================

#ifdef HELP_ME
	AddDebugTriangle(glm::vec3(m_B.x, m_B.y, 0.0f), { 1.0f, 0.0f, 1.0f, 1.0f });
#endif

#ifdef HELP_ME
	AddDebugTriangle(glm::vec3(centerPoints.front().x, centerPoints.front().z, 0.0f), { 0.0f, 1.0f, 0.0f, 1.0f });
	AddDebugTriangle(glm::vec3(centerPoints.back().x, centerPoints.back().z, 0.0f), { 0.0f, 1.0f, 0.0f, 1.0f });
#else
	m_Vertices.push_back(Vertex(glm::vec3(centerPoints.front().x, centerPoints.front().y, centerPoints.front().z), glm::vec<4, float, glm::packed_mediump>(0.0f, 1.0f, 0.0f, 1.0f)));
	m_Vertices.push_back(Vertex(glm::vec3(centerPoints.back().x, centerPoints.back().y, centerPoints.back().z), glm::vec<4, float, glm::packed_mediump>(0.0f, 1.0f, 0.0f, 1.0f)));
#endif

	for (uint32_t i = 0; i < rightPoints.size(); i++)
	{
#ifndef HELP_ME
		m_Vertices.push_back(Vertex(glm::vec3(rightPoints[i].x, rightPoints[i].y, rightPoints[i].z), glm::vec<4, float, glm::packed_mediump>(1.0f, 0.0f, 0.0f, 1.0f)));
#else
		AddDebugTriangle(rightPoints[i], { 1.0f, 0.0f, 0.0f, 1.0f });
#endif
	}

	for (uint32_t i = 0; i < leftPoints.size(); i++)
	{
#ifndef HELP_ME
		m_Vertices.push_back(Vertex(glm::vec3(leftPoints[i].x, leftPoints[i].y, leftPoints[i].z), glm::vec<4, float, glm::packed_mediump>(0.0f, 0.0f, 1.0f, 1.0f)));
#else
		AddDebugTriangle(leftPoints[i], { 0.0f, 0.0f, 1.0f, 1.0f });
#endif
	}

#ifdef HELP_ME
	for (uint32_t i = 1; i < centerPoints.size() - 1; i++)
	{
		AddDebugTriangle(glm::vec3(centerPoints[i].x, centerPoints[i].y, centerPoints[i].z), { 1.0f, 1.0f, 1.0f, 1.0f });
	}
#endif

	// ==========================================================================================
	// Create Indices
	// ==========================================================================================

	m_Indices.clear();
	m_Indices.reserve(leftPoints.size() * 2 * 3 + 6);

	// Left

	for (uint32_t i = 0; i < leftPoints.size() - 1; i++)
	{
		m_Indices.push_back(i + 2);
		m_Indices.push_back(leftPoints.size() + i + 3);
		m_Indices.push_back(i + 3);
	}
	
	//m_Indices.push_back(leftPoints.size() + 1);
	//m_Indices.push_back(leftPoints.size() + rightPoints.size() + 1);
	//m_Indices.push_back(1);

	// Right

	//m_Indices.push_back(leftPoints.size() + 2);
	//m_Indices.push_back(2);
	//m_Indices.push_back(0);
	
	for (uint32_t i = 1; i < rightPoints.size(); i++)
	{
		m_Indices.push_back(i + leftPoints.size() + 2);
		m_Indices.push_back(i + 1);
		m_Indices.push_back(i + leftPoints.size() + 1);
	}

	//m_Vertices.clear();
	//m_Vertices.push_back(Vertex({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.4f, 0.0f, 1.0f }));
	//m_Vertices.push_back(Vertex({ 1.0f, 0.0f, 0.0f }, { 1.0f, 0.4f, 0.0f, 1.0f }));
	//m_Vertices.push_back(Vertex({ 1.0f, 1.0f, 0.0f }, { 1.0f, 0.8f, 0.0f, 1.0f }));
	//m_Vertices.push_back(Vertex({ 0.0f, 1.0f, 0.0f }, { 1.0f, 0.8f, 0.0f, 1.0f }));
	//
	//m_Indices.clear();
	//m_Indices.push_back(0);
	//m_Indices.push_back(1);
	//m_Indices.push_back(2);
	
	//m_Indices.push_back(2);
	//m_Indices.push_back(3);
	//m_Indices.push_back(0);

#ifdef HELP_ME
	m_Indices.clear();
	
	for (uint32_t i = 0; i < m_Vertices.size(); i++)
		m_Indices.push_back(i);
#endif
}

float Lane::GetLength()
{
#ifndef NDEBUG
	assert((m_Length > 0.01f && m_Vertices.size() > 0 && m_Indices.size() > 0));
#endif

	return m_Length;
}

std::vector<Vertex>* Lane::GetVertices()
{
#ifndef NDEBUG
	assert((m_Length > 0.01f && m_Vertices.size() > 0 && m_Indices.size() > 0));
#endif

	return &m_Vertices;
}

std::vector<uint32_t>* Lane::GetIndices()
{
#ifndef NDEBUG
	assert((m_Length > 0.01f && m_Vertices.size() > 0 && m_Indices.size() > 0));
#endif
	return &m_Indices;
}

std::pair<glm::vec2, glm::vec2> Lane::GenerateExterior(glm::vec2 a, glm::vec2 b, glm::vec2 c)
{
	glm::vec2 resLeft, resRight;

	bool useHor = std::abs(a.y - c.y) < 0.01;
	bool useVer = std::abs(a.x - c.x) < 0.01;

	if (useHor)
	{
		// slope is to small

		glm::vec2 p0 = {
			b.x,
			b.y - LANE_HALF_WIDTH
		};

		glm::vec2 p1 = {
			b.x,
			b.y + LANE_HALF_WIDTH
		};

		if (a.x > c.x)
		{
			resRight = p0;
			resLeft = p1;
		}
		else
		{
			resRight = p1;
			resLeft = p0;
		}
	}
	else if (useVer)
	{
		// slope is to big

		glm::vec2 p0 = {
			b.x - LANE_HALF_WIDTH,
			b.y
		};

		glm::vec2 p1 = {
			b.x + LANE_HALF_WIDTH,
			b.y
		};

		if (a.y > c.y)
		{
			resRight = p1;
			resLeft = p0;
		}
		else
		{
			resRight = p0;
			resLeft = p1;
		}
	}
	else
	{
		// abs this for ribbon
#ifdef RIBBON_ME
		float slopeInv = std::abs((a.x - c.x) / (a.y - c.y) * -1.0f);
#else
		float slopeInv = (a.x - c.x) / (a.y - c.y) * -1.0f;
#endif
	
		// slope is not bad

		glm::vec2 p = {
			a.x + 1.0f,
			slopeInv * ((a.x + 1.0f) - b.x) + b.y
		};

		glm::vec2 v = p - b;
		glm::vec2 nv = glm::normalize(v);

		glm::vec2 p0 = b - (nv * LANE_HALF_WIDTH);
		glm::vec2 p1 = b + (nv * LANE_HALF_WIDTH);

		glm::vec2 v0 = c - a;

#ifdef RIBBON_ME
		resLeft = p0;
		resRight = p1;
#else
		if (v0.y > 0)
		{
			resLeft = p1;
			resRight = p0;
		}
		else
		{
			resLeft = p0;
			resRight = p1;
		}
#endif

		Vertex::SLOPE_DEBUG.push_back(std::to_string(resLeft.x) + ", " + std::to_string(resLeft.y));
		Vertex::NV_DEBUG.push_back(std::to_string(resRight.x) + ", " + std::to_string(resRight.y));
	}

	return std::make_pair(resLeft, resRight);
}

std::pair<glm::vec2, glm::vec2> Lane::GenerateExteriorCorner(glm::vec2 a, glm::vec2 b)
{
	glm::vec2 resLeft, resRight;

	bool useHor = std::abs(a.y - b.y) < 0.01;
	bool useVer = std::abs(a.x - b.x) < 0.01;

	if (useHor)
	{
		// slope is to small

		glm::vec2 p0 = {
			b.x,
			b.y - LANE_HALF_WIDTH
		};

		glm::vec2 p1 = {
			b.x,
			b.y + LANE_HALF_WIDTH
		};

		if (a.x > b.x)
		{
			resRight = p1;
			resLeft = p0;
		}
		else
		{
			resRight = p0;
			resLeft = p1;
		}
	}
	else if (useVer)
	{
		// slope is to big

		glm::vec2 p0 = {
			b.x - LANE_HALF_WIDTH,
			b.y
		};

		glm::vec2 p1 = {
			b.x + LANE_HALF_WIDTH,
			b.y
		};

		if (a.y > b.y)
		{
			resRight = p0;
			resLeft = p1;
		}
		else
		{
			resRight = p1;
			resLeft = p0;
		}
	}
	else
	{
		// abs this for ribbon
#ifdef RIBBON_ME
		float slopeInv = std::abs((a.x - b.x) / (a.y - b.y) * -1.0f);
#else
		float slopeInv = (a.x - b.x) / (a.y - b.y) * -1.0f;
#endif

		Vertex::SLOPE_DEBUG.push_back(std::to_string(slopeInv));

		// slope is not bad

		glm::vec2 p = {
			a.x + 1.0f,
			slopeInv * ((a.x + 1.0f) - b.x) + b.y
		};

		glm::vec2 v = p - b;
		glm::vec2 nv = glm::normalize(v);
		Vertex::NV_DEBUG.push_back(std::to_string(nv.y));

		glm::vec2 p0 = b - (nv * LANE_HALF_WIDTH);
		glm::vec2 p1 = b + (nv * LANE_HALF_WIDTH);

		glm::vec2 v0 = b - a;

#ifdef RIBBON_ME
		resLeft = p1;
		resRight = p0;
#else
		if (v0.y > 0)
		{
			resLeft = p0;
			resRight = p1;
		}
		else
		{
			resLeft = p1;
			resRight = p0;
		}
#endif
	}

	return std::make_pair(resLeft, resRight);
}

void Lane::AddDebugTriangle(glm::vec2 position, glm::vec<4, float, glm::packed_mediump> color)
{
	m_Vertices.push_back(Vertex(glm::vec<3, float, glm::packed_mediump>(position + glm::vec2(0.0f, 0.0f), 0.0f), color));
	m_Vertices.push_back(Vertex(glm::vec<3, float, glm::packed_mediump>(position + glm::vec2(0.04f, 0.0f), 0.0f), color));
	m_Vertices.push_back(Vertex(glm::vec<3, float, glm::packed_mediump>(position + glm::vec2(0.04f, 0.04f), 0.0f), color));
}
