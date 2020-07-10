#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

class Vertex
{
public:
	Vertex()
		:Position{ 0.0f, 0.0f, 0.0f }, Color{ 1.0f, 1.0f, 1.0f, 1.0f }, TexCoord{ 0.0f, 0.0f }, TexID(0.0f) {}

	Vertex(glm::vec<3, float, glm::packed_mediump> position, glm::vec<4, float, glm::packed_mediump> color)
		:Position(position), Color(color), TexCoord{ 0.0f, 0.0f }, TexID(0.0f) {}

	Vertex(glm::vec<3, float, glm::packed_mediump> position, glm::vec<4, float, glm::packed_mediump> color, glm::vec<2, float, glm::packed_mediump> texCoord)
		:Position(position), Color(color), TexCoord(texCoord), TexID(1.0f) {}

	Vertex(glm::vec<3, float, glm::packed_mediump> position, glm::vec<2, float, glm::packed_mediump> texCoord)
		:Position(position), Color{ 1.0f, 1.0f, 1.0f, 1.0f }, TexCoord(texCoord), TexID(1.0f) {}

	glm::vec<3, float, glm::packed_mediump> Position;
	glm::vec<4, float, glm::packed_mediump> Color;
	glm::vec<2, float, glm::packed_mediump> TexCoord;
	float TexID;


	static std::vector<std::string> SLOPE_DEBUG;
	static std::vector<std::string> NV_DEBUG;
};

static const size_t VertexFloatCount = sizeof(Vertex) / sizeof(float);