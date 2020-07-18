#include "GameLayer.h"
#include "road/Lane.h"

#include <stb_image/stb_image.h>
#include <GLCore/Core/KeyCodes.h>
#include <numbers>



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
#include <libavcodec/avcodec.h>

#define INBUF_SIZE 4096

static void pgm_save(unsigned char* buf, int wrap, int xsize, int ysize,
	char* filename)
{
	FILE* f;
	int i;

	f = fopen(filename, "w");
	fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
	for (i = 0; i < ysize; i++)
		fwrite(buf + i * wrap, 1, xsize, f);
	fclose(f);
}

static void decode(AVCodecContext* dec_ctx, AVFrame* frame, AVPacket* pkt,
	const char* filename)
{
	char buf[1024];
	int ret;

	ret = avcodec_send_packet(dec_ctx, pkt);
	if (ret < 0) {
		fprintf(stderr, "Error sending a packet for decoding\n");
		exit(1);
	}

	while (ret >= 0) {
		ret = avcodec_receive_frame(dec_ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0) {
			fprintf(stderr, "Error during decoding\n");
			exit(1);
		}

		printf("saving frame %3d\n", dec_ctx->frame_number);
		fflush(stdout);

		// the picture is allocated by the decoder. no need to
		//   free it 
		snprintf(buf, sizeof(buf), "%s-%d", filename, dec_ctx->frame_number);
		pgm_save(frame->data[0], frame->linesize[0],
			frame->width, frame->height, buf);
	}
}

void runDecode()
{
	const char* filename, * outfilename;
	const AVCodec* codec;
	AVCodecParserContext* parser;
	AVCodecContext* c = NULL;
	FILE* f;
	AVFrame* frame;
	uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
	uint8_t* data;
	size_t   data_size;
	int ret;
	AVPacket* pkt;

	filename = "input.avi";
	outfilename = "output.avi";

	pkt = av_packet_alloc();
	if (!pkt)
		exit(1);

	// set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) 
	memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);

	// find the MPEG-1 video decoder 
	codec = avcodec_find_decoder(AV_CODEC_ID_MPEG1VIDEO);
	if (!codec) {
		fprintf(stderr, "Codec not found\n");
		exit(1);
	}

	parser = av_parser_init(codec->id);
	if (!parser) {
		fprintf(stderr, "parser not found\n");
		exit(1);
	}

	c = avcodec_alloc_context3(codec);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

	// For some codecs, such as msmpeg4 and mpeg4, width and height
	//   MUST be initialized there because this information is not
	//   available in the bitstream. 
	//
	//    open it 
	if (avcodec_open2(c, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		exit(1);
	}

	f = fopen(filename, "rb");
	if (!f) {
		fprintf(stderr, "Could not open %s\n", filename);
		exit(1);
	}

	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}

	while (!feof(f)) {
		// read raw data from the input file 
		data_size = fread(inbuf, 1, INBUF_SIZE, f);
		if (!data_size)
			break;

		// use the parser to split the data into frames 
		data = inbuf;
		while (data_size > 0) {
			ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
				data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
			if (ret < 0) {
				fprintf(stderr, "Error while parsing\n");
				exit(1);
			}
			data += ret;
			data_size -= ret;

			if (pkt->size)
				decode(c, frame, pkt, outfilename);
		}
	}

	 flush the decoder 
	decode(c, frame, NULL, outfilename);

	fclose(f);

	av_parser_close(parser);
	avcodec_free_context(&c);
	av_frame_free(&frame);
	av_packet_free(&pkt);
}
*/

using namespace GLCore;
using namespace GLCore::Utils;

GameLayer::GameLayer()
	:m_Camera(glm::radians(70.0f), 16.0f / 9.0f, 0.1f, 10000.0f)
{
	m_Camera.SetPosition({ -2.0f, 1.0f, -2.0f });
	m_Camera.SetRotation({ 0.0f, 0.0f });
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

void GameLayer::OnAttach()
{
	//runDecode();

	// setup

	EnableGLDebugging();

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

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

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
		0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.8f, 0.8f, 0.8f, 1.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.3f, 0.3f, 0.3f, 1.0f, 0.0f, 1.0f, 0.0f,

		0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f, 1.0f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
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

	//
	//
	//

	float cube2Data[] = {
		0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.8f, 0.8f, 0.8f, 1.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.3f, 0.3f, 0.3f, 1.0f, 0.0f, 1.0f, 0.0f,

		0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f, 1.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f,
	};

	uint32_t cube2Indices[] = {
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

	glCreateVertexArrays(1, &cube2VA);
	glBindVertexArray(cube2VA);

	// VBO

	glCreateBuffers(1, &cube2VB);
	glBindBuffer(GL_ARRAY_BUFFER, cube2VB);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(Vertex), cube2Data, GL_STATIC_DRAW);

	// attribs

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Color));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexCoord));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexID));

	glCreateBuffers(1, &cube2IB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube2IB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 36, cube2Indices, GL_STATIC_DRAW);

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
static bool movForwards = false;
static bool movBackwards = false;

static bool movUp = false;
static bool movDown = false;

static bool rotXD = false;
static bool rotXU = false;
static bool rotYD = false;
static bool rotYU = false;

static float mouseMiddleX = 0.0f;
static float mouseMiddleY = 0.0f;

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
			mouseMiddleX = (e.GetX() - (1920.0f / 2.0f));
			mouseMiddleY = ((1080.0f / 2.0f) - e.GetY());

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

void GameLayer::OnUpdate(Timestep ts)
{
	using std::numbers::pi;

	//if (rotXD)
	//	m_Camera.Rotate(glm::vec3(ts * 1.0f, 0.0f, 0.0f));
	//if (rotXU)
	//	m_Camera.Rotate(glm::vec3(ts * -1.0f, 0.0f, 0.0f));
	//if (rotYD)
	//	m_Camera.Rotate(glm::vec3(0.0f, ts * -1.0f, 0.0f));
	//if (rotYU)
	//	m_Camera.Rotate(glm::vec3(0.0f, ts * 1.0f, 0.0f));
	
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
	//direction.x = cos(glm::radians(mouseMiddleX));
	//direction.y = sin(glm::radians(mouseMiddleY));
	//direction.z = sin(glm::radians(mouseMiddleX));

	//direction.x = mouseMiddleX;
	//direction.y = mouseMiddleY;

	direction.x = mouseMiddleY;
	//direction.z = mouseMiddleX;

	//m_Camera.SetRotation(direction);

	//glm::vec3 upV = glm::vec3(1.0f, 0.0f, 0.0f);
	//float somethingS = glm::dot(m_Camera.GetRotation(), upV);

	//printf("%f, %f, %f\n", glm::normalize(m_Camera.GetRotation()).x, glm::normalize(m_Camera.GetRotation()).y, glm::normalize(m_Camera.GetRotation()).z);

	float movSpeed = ts * 5.0f;

	//glm::vec3 movementChange = glm::vec3(0.0f, 0.0f, 0.0f);

	//if (movLeft)
	//	movementChange.x += 5.0f;
	//if (movRight)
	//	movementChange.x += -5.0f;
	//if (movForwards)
	//	movementChange.z += 5.0f;
	//if (movBackwards)
	//	movementChange.z += -5.0f;
	//if (movUp)
	//	movementChange.y += -5.0f;
	//if (movDown)
	//	movementChange.y += 5.0f;

	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	if (movForwards)
		m_Camera.Translate(movSpeed * m_Camera.GetForward());
	if (movBackwards)
		m_Camera.Translate(-movSpeed * m_Camera.GetForward());
	if (movLeft)
		m_Camera.Translate(-movSpeed * glm::normalize(glm::cross(m_Camera.GetForward(), cameraUp)));
	if (movRight)
		m_Camera.Translate(movSpeed * glm::normalize(glm::cross(m_Camera.GetForward(), cameraUp)));

	m_Camera.SetRotation({ mouseMiddleY / 150.0f, mouseMiddleX / 150.0f });

	//if (movUp)
	//	m_Camera.TranslateForward(movSpeed * glm::vec3(0.0f, -1.0f, 0.0f) * m_Camera.GetForward());
	//if (movDown)
	//	m_Camera.TranslateForward(movSpeed * glm::vec3(0.0f, 1.0f, 0.0f) * m_Camera.GetForward());

	//printf("%f, %f, %f\n", movementChange.x, movementChange.y, movementChange.z);
	//m_Camera.SetPosition({ 0.0f, 0.0f, 0.0f });
	//m_Camera.TranslateForward(movSpeed * glm::vec3() * m_Camera.GetForward());

	//m_Camera.SetPosition({ camPos[0], camPos[1], camPos[2] });
	//m_Camera.SetRotation({ camRot[0], camRot[1], camRot[2] });
	
	glUseProgram(m_Shader->GetRendererID());

	auto loc = glGetUniformLocation(m_Shader->GetRendererID(), "u_Textures");
	int samplers[2] = { 0, 1 };
	glUniform1iv(loc, 2, samplers);

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

	location = glGetUniformLocation(m_Shader->GetRendererID(), "u_Color");
	glUniform4fv(location, 1, glm::value_ptr(GeneralColor));

	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetRendererID(), "u_MVP"), 1, GL_FALSE, glm::value_ptr(m_Camera.ConstructMVP(
		glm::vec3(obj1Pos[0], obj1Pos[1], obj1Pos[2]),
		glm::vec3(obj1Rot[0], obj1Rot[1], obj1Rot[2]),
		glm::vec3(1.0f, 1.0f, 1.0f)
	)));
	
	glBindVertexArray(lnVA);
	glDrawElements(GL_TRIANGLES, lnIndices->size(), GL_UNSIGNED_INT, nullptr);

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
}

void GameLayer::OnImGuiRender()
{
	using std::numbers::pi;

	ImGui::Begin("Lane");

	//ImGui::SliderFloat3("CAM position", camPos, -100.0f, 100.0f);
	//ImGui::SliderFloat3("CAM rotation", camRot, -pi, pi);
	//ImGui::NewLine();

	//ImGui::SliderFloat3("OBJ1 position", obj1Pos, -100.0f, 100.0f);
	//ImGui::SliderFloat3("OBJ1 rotation", obj1Rot, -pi, pi);
	//ImGui::NewLine();
	//
	//ImGui::SliderFloat3("OBJ2 position", obj2Pos, -100.0f, 100.0f);
	//ImGui::SliderFloat3("OBJ2 rotation", obj2Rot, -pi, pi);
	//ImGui::NewLine();

	ImGui::SliderFloat2("P0", lnPos0, -10.0f, 10.0f);
	ImGui::SliderFloat2("P1", lnPos1, -10.0f, 10.0f);
	ImGui::SliderFloat2("Bez", lnBez, -10.0f, 10.0f);

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
