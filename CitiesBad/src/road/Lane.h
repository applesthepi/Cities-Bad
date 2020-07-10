#pragma once
#include "Vertex.h"

#include <vector>
#include <GLCore.h>
#include <GLCoreUtils.h>
#include <cassert>

#define LANE_HALF_WIDTH 0.1f

class Lane
{
public:
	Lane();

	void SetPosition(glm::vec3 p1, glm::vec3 p2);
	void SetBezier(glm::vec2 b);
	void Generate();

	float GetLength();
	std::vector<Vertex>* GetVertices();
	std::vector<uint32_t>* GetIndices();
private:
	// generates left and right of `b` not including the first and last one
	std::pair<glm::vec2, glm::vec2> GenerateExterior(glm::vec2 a, glm::vec2 b, glm::vec2 c);

	// generates left and right of `a` for the first and last one
	std::pair<glm::vec2, glm::vec2> GenerateExteriorCorner(glm::vec2 a, glm::vec2 b);

	// TODO HELP REMOVE PLEASE OH GOD NO
	void AddDebugTriangle(glm::vec2 position, glm::vec<4, float, glm::packed_mediump> color);

	glm::vec3 m_P1;
	glm::vec3 m_P2;

	glm::vec2 m_B;

	float m_Length;
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
};