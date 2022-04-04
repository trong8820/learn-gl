// Metaballs

#include "macros.h"
#include "entry.h"

#include "vec2.h"
#include "vec3.h"
#include "mat4.h"

const float PI = 3.14159265358979f;

const int table[16 * 4 * 3] =
{
	-1, -1, -1,  -1, -1, -1,  -1, -1, -1,  -1, -1, -1,   //0000
	+0, +4, +7,  -1, -1, -1,  -1, -1, -1,  -1, -1, -1,   //0001
	+4, +1, +5,  -1, -1, -1,  -1, -1, -1,  -1, -1, -1,   //0010
	+0, +1, +5,  +0, +5, +7,  -1, -1, -1,  -1, -1, -1,   //0011

	+5, +2, +6,  -1, -1, -1,  -1, -1, -1,  -1, -1, -1,   //0100
	+0, +4, +5,  +0, +5, +2,  +0, +2, +6,  +0, +6, +7,   //0101
	+4, +1, +2,  +4, +2, +6,  -1, -1, -1,  -1, -1, -1,   //0110
	+0, +1, +2,  +0, +2, +6,  +0, +6, +7,  -1, -1, -1,   //0111

	+7, +6, +3,  -1, -1, -1,  -1, -1, -1,  -1, -1, -1,   //1000
	+0, +4, +6,  +0, +6, +3,  -1, -1, -1,  -1, -1, -1,   //1001
	+7, +4, +3,  +4, +1, +5,  +4, +5, +6,  +3, +4, +6,   //1010
	+0, +1, +5,  +0, +5, +6,  +0, +6, +3,  -1, -1, -1,   //1011

	+7, +5, +2,  +7, +2, +3,  -1, -1, -1,  -1, -1, -1,   //1100
	+0, +4, +5,  +0, +5, +2,  +0, +2, +3,  -1, -1, -1,   //1101
	+4, +1, +2,  +4, +2, +3,  +7, +4, +3,  -1, -1, -1,   //1110
	+0, +1, +2,  +0, +2, +3,  -1, -1, -1,  -1, -1, -1    //1111
};

float positions[9];

const char *scalarVertexShaderSource = R"(
#version 410 core

#define EPSILON 0.000001

uniform int samples;
uniform vec3 aPos[3];
out float outScalar;

void main()
{
	int x = gl_VertexID / samples;
	int y = gl_VertexID % samples;
	vec2 pos = vec2(x, y) / (samples - 1.0);

	outScalar = 0.0;
	for (int i=0; i<3; i++)
	{
		float distance = length(distance(aPos[i].xy, pos));
		outScalar += aPos[i].z / pow(max(EPSILON, distance), 2.0);
	}
}
)";

const char *scalarFragmentShaderSource = R"(
#version 410 core

void main()
{
}
)";

const char *cellVertexShaderSource = R"(
#version 410 core

uniform int samples;
uniform sampler2D scalar;

flat out int outCell;

void main()
{
	int x = gl_VertexID / samples;
	int y = gl_VertexID % samples;

	outCell = 0;

	vec2 pos0 = vec2(x + 0, y + 0) / float(samples);
	float s0 = textureLod(scalar, pos0, 0.0).r;
	if (s0 > 12.0)
	{
		outCell |= (1 << 0);
	}

	vec2 pos1 = vec2(x + 1, y + 0) / float(samples);
	float s1 = textureLod(scalar, pos1, 0.0).r;
	if (s1 > 12.0)
	{
		outCell |= (1 << 1);
	}

	vec2 pos2 = vec2(x + 1, y + 1) / float(samples);
	float s2 = textureLod(scalar, pos2, 0.0).r;
	if (s2 > 12.0)
	{
		outCell |= (1 << 2);
	}

	vec2 pos3 = vec2(x + 0, y + 1) / float(samples);
	float s3 = textureLod(scalar, pos3, 0.0).r;
	if (s3 > 12.0)
	{
		outCell |= (1 << 3);
	}
}
)";

const char *cellFragmentShaderSource = R"(
#version 410 core

void main()
{
}
)";

const char *marchingVertexShaderSource = R"(
#version 410 core

const vec2 offsets[8] = vec2[8]
(
	vec2(0.0, 0.0),
	vec2(0.0, 1.0),
	vec2(1.0, 1.0),
	vec2(1.0, 0.0),

	vec2(0.0, 0.5),
	vec2(0.5, 1.0),
	vec2(1.0, 0.5),
	vec2(0.5, 0.0)
);

uniform int samples;
uniform isampler2D cell;
uniform isampler2D table;

void main()
{
	int encoded = gl_VertexID;
	int vertex = encoded % 12;
	encoded = encoded / 12;

	int x = encoded / samples;
	int y = encoded % samples;

	vec2 pos = vec2(x, y) / float(samples - 1.0);
	int s = textureLod(cell, pos, 0.0).r;

	// [0 .. 11] [0 .. 15]
	int edge = textureLod(table, vec2(vertex/11.0, s/15.0), 0.0).r;
	//int edge = textureLod(table, vec2(vertex/11.0, 3/15.0), 0.0).r;
	if (edge < 0)
	{
		gl_Position = vec4(0);
	} else
	{
		vec2 vPos = (vec2(x, y) + offsets[edge] - vec2(0.5, 0.5)) / float(samples);
		gl_Position = vec4(vPos - vec2(0.5, 0.5), 0.0f, 1.0);
		gl_PointSize = edge == 0 ? 4.0 : 2.0;
	}
}
)";

const char *marchingFragmentShaderSource = R"(
#version 410 core

out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)";


const int SAMPLES = 256;

GLuint gScalarProgram;
GLuint gScalarTFO;
GLuint gScalarTFBO;
GLint gPosLoc;

GLuint gCellProgram;
GLuint gCellTFO;
GLuint gCellTFBO;

GLuint gMarchingProgram;

float gTime = 0.0f;

auto init() -> bool
{
	//std::cout << "init " << gWidth << " " << gHeight << std::endl;

	glad_glTexStorage2D = (PFNGLTEXSTORAGE2DPROC)glfwGetProcAddress("glTexStorage2D"); // OpenGL 4.2 or ARB_texture_storage

	{
		auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
		GL_CHECK(glShaderSource(vertexShader, 1, &scalarVertexShaderSource, NULL));
		GL_CHECK(glCompileShader(vertexShader));
		{
			GLint success;
			GL_CHECK(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success));
			if(!success)
			{
				GLint infoLength = 0;
				glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLength);
				char* infoLog = new char[infoLength];
				GL_CHECK(glGetShaderInfoLog(vertexShader, infoLength, &infoLength, infoLog));
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "VERTEX_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				delete [] infoLog;
				GL_CHECK(glDeleteShader(vertexShader));
			}
		}

		auto fragmentShader = GL_CHECK_RETURN(glCreateShader(GL_FRAGMENT_SHADER));
		GL_CHECK(glShaderSource(fragmentShader, 1, &scalarFragmentShaderSource, NULL));
		GL_CHECK(glCompileShader(fragmentShader));
		{
			GLint success;
			GL_CHECK(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success));
			if(!success)
			{
				GLint infoLength = 0;
				glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLength);
				char* infoLog = new char[infoLength];
				GL_CHECK(glGetShaderInfoLog(fragmentShader, infoLength, &infoLength, infoLog));
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "FRAGMENT_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				delete [] infoLog;
				GL_CHECK(glDeleteShader(fragmentShader));
			}
		}

		gScalarProgram = GL_CHECK_RETURN(glCreateProgram());
		GL_CHECK(glAttachShader(gScalarProgram, vertexShader));
		GL_CHECK(glAttachShader(gScalarProgram, fragmentShader));
		const GLchar* feedbackVaryings[] = { "outScalar" };
		glTransformFeedbackVaryings(gScalarProgram, 1, feedbackVaryings, GL_SEPARATE_ATTRIBS);
		GL_CHECK(glLinkProgram(gScalarProgram));

		GL_CHECK(glDeleteShader(vertexShader));
		GL_CHECK(glDeleteShader(fragmentShader));
	}

	{
		auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
		GL_CHECK(glShaderSource(vertexShader, 1, &cellVertexShaderSource, NULL));
		GL_CHECK(glCompileShader(vertexShader));
		{
			GLint success;
			GL_CHECK(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success));
			if(!success)
			{
				GLint infoLength = 0;
				glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLength);
				char* infoLog = new char[infoLength];
				GL_CHECK(glGetShaderInfoLog(vertexShader, infoLength, &infoLength, infoLog));
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "VERTEX_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				delete [] infoLog;
				GL_CHECK(glDeleteShader(vertexShader));
			}
		}

		auto fragmentShader = GL_CHECK_RETURN(glCreateShader(GL_FRAGMENT_SHADER));
		GL_CHECK(glShaderSource(fragmentShader, 1, &cellFragmentShaderSource, NULL));
		GL_CHECK(glCompileShader(fragmentShader));
		{
			GLint success;
			GL_CHECK(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success));
			if(!success)
			{
				GLint infoLength = 0;
				glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLength);
				char* infoLog = new char[infoLength];
				GL_CHECK(glGetShaderInfoLog(fragmentShader, infoLength, &infoLength, infoLog));
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "FRAGMENT_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				delete [] infoLog;
				GL_CHECK(glDeleteShader(fragmentShader));
			}
		}

		gCellProgram = GL_CHECK_RETURN(glCreateProgram());
		GL_CHECK(glAttachShader(gCellProgram, vertexShader));
		GL_CHECK(glAttachShader(gCellProgram, fragmentShader));
		const GLchar* feedbackVaryings[] = { "outCell" };
		glTransformFeedbackVaryings(gCellProgram, 1, feedbackVaryings, GL_SEPARATE_ATTRIBS);
		GL_CHECK(glLinkProgram(gCellProgram));

		GL_CHECK(glDeleteShader(vertexShader));
		GL_CHECK(glDeleteShader(fragmentShader));
	}

	{
		auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
		GL_CHECK(glShaderSource(vertexShader, 1, &marchingVertexShaderSource, NULL));
		GL_CHECK(glCompileShader(vertexShader));
		{
			GLint success;
			GL_CHECK(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success));
			if(!success)
			{
				GLint infoLength = 0;
				glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLength);
				char* infoLog = new char[infoLength];
				GL_CHECK(glGetShaderInfoLog(vertexShader, infoLength, &infoLength, infoLog));
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "VERTEX_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				delete [] infoLog;
				GL_CHECK(glDeleteShader(vertexShader));
			}
		}

		auto fragmentShader = GL_CHECK_RETURN(glCreateShader(GL_FRAGMENT_SHADER));
		GL_CHECK(glShaderSource(fragmentShader, 1, &marchingFragmentShaderSource, NULL));
		GL_CHECK(glCompileShader(fragmentShader));
		{
			GLint success;
			GL_CHECK(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success));
			if(!success)
			{
				GLint infoLength = 0;
				glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLength);
				char* infoLog = new char[infoLength];
				GL_CHECK(glGetShaderInfoLog(fragmentShader, infoLength, &infoLength, infoLog));
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "FRAGMENT_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				delete [] infoLog;
				GL_CHECK(glDeleteShader(fragmentShader));
			}
		}

		gMarchingProgram = GL_CHECK_RETURN(glCreateProgram());
		GL_CHECK(glAttachShader(gMarchingProgram, vertexShader));
		GL_CHECK(glAttachShader(gMarchingProgram, fragmentShader));
		GL_CHECK(glLinkProgram(gMarchingProgram));

		GL_CHECK(glDeleteShader(vertexShader));
		GL_CHECK(glDeleteShader(fragmentShader));
	}

	// TODO: ? Can phai co du khong su dung
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glUseProgram(gScalarProgram);
	glUniform1i(glGetUniformLocation(gScalarProgram, "samples"), SAMPLES);
	gPosLoc = glGetUniformLocation(gScalarProgram, "aPos");

	glGenTransformFeedbacks(1, &gScalarTFO);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, gScalarTFO);

	glGenBuffers(1, &gScalarTFBO);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, gScalarTFBO);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, SAMPLES * SAMPLES * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK_BUFFER, gScalarTFBO);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, gScalarTFBO);

	GLuint scalarTexture;
	glGenTextures(1, &scalarTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scalarTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, SAMPLES, SAMPLES);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glUseProgram(gCellProgram);
	glUniform1i(glGetUniformLocation(gCellProgram, "samples"), SAMPLES - 1);
	glUniform1i(glGetUniformLocation(gCellProgram, "scalar"), 0);

	glGenTransformFeedbacks(1, &gCellTFO);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, gCellTFO);

	glGenBuffers(1, &gCellTFBO);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, gCellTFBO);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,  (SAMPLES - 1) * (SAMPLES - 1) * sizeof(GLint), NULL, GL_STATIC_DRAW);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK_BUFFER, gCellTFBO);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, gCellTFBO);

	GLuint cellTexture;
	glGenTextures(1, &cellTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, cellTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32I, SAMPLES - 1, SAMPLES - 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glUseProgram(gMarchingProgram);
	glUniform1i(glGetUniformLocation(gMarchingProgram, "samples"), SAMPLES - 1);
	glUniform1i(glGetUniformLocation(gMarchingProgram, "cell"), 1);
	glUniform1i(glGetUniformLocation(gMarchingProgram, "table"), 2);

	GLuint tableTexture;
	glGenTextures(1, &tableTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tableTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32I, 12, 16);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 12, 16, GL_RED_INTEGER, GL_INT, table);

	size();

	glEnable(GL_PROGRAM_POINT_SIZE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glEnable(GL_MULTISAMPLE);
	//glEnable(GL_DEPTH_TEST);

	return 0;
}

void size()
{
	//std::cout << "size " << gWidth << " " << gHeight << std::endl;
}

auto update() -> void
{
	gTime += 1.0/30.0f;
	vec2 pos1 = vec2(0.5f, 0.5f) + vec2(0.2f, 0.25f)*vec2(std::sinf((11.0f*gTime + 30.0f)*PI/180.0f), std::sinf((21.0f*gTime + 45.0f)*PI/180.0f));
	positions[0] = pos1.x;
	positions[1] = pos1.y;
	positions[2] = 0.1f;
	vec2 pos2 = vec2(0.5f, 0.5f) + vec2(0.25f, 0.2f)*vec2(std::sinf((22.0f*gTime + 45.0f)*PI/180.0f), std::sinf((32.0f*gTime + 90.0f)*PI/180.0f));
	positions[3] = pos2.x;
	positions[4] = pos2.y;
	positions[5] = 0.05f;
	vec2 pos3 = vec2(0.5f, 0.5f) + vec2(0.25f, 0.25f)*vec2(std::sinf((33.0f*gTime + 90.0f)*PI/180.0f), std::sinf((13.0f*gTime + 120.0f)*PI/180.0f));
	positions[6] = pos3.x;
	positions[7] = pos3.y;
	positions[8] = 0.25f;

	/*positions[0] = 0.5f;
	positions[1] = 0.5f;
	//positions[2] = 0.4f;
	positions[2] = gTime * 0.01f;

	positions[3] = -100.0f;
	positions[4] = -100.0f;
	positions[5] = 0.1f;

	positions[6] = -100.0f;
	positions[7] = -100.0f;
	positions[8] = 0.1f;*/
}

auto draw() -> void
{
	glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_RASTERIZER_DISCARD);
		glUseProgram(gScalarProgram);
		glUniform3fv(gPosLoc, 3, positions);

		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, gScalarTFO);
		glBeginTransformFeedback(GL_POINTS);
			glDrawArrays(GL_POINTS, 0, SAMPLES * SAMPLES);
		glEndTransformFeedback();
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glDisable(GL_RASTERIZER_DISCARD);

	glActiveTexture(GL_TEXTURE0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, gScalarTFBO);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SAMPLES, SAMPLES, GL_RED, GL_FLOAT, NULL);

	glEnable(GL_RASTERIZER_DISCARD);
	glUseProgram(gCellProgram);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, gCellTFO);
		glBeginTransformFeedback(GL_POINTS);
			glDrawArrays(GL_POINTS, 0, (SAMPLES - 1) * (SAMPLES - 1));
		glEndTransformFeedback();
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glDisable(GL_RASTERIZER_DISCARD);

	glActiveTexture(GL_TEXTURE1);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, gCellTFBO);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SAMPLES - 1, SAMPLES - 1, GL_RED_INTEGER, GL_INT, NULL);

	glActiveTexture(GL_TEXTURE2);
	glUseProgram(gMarchingProgram);
	glDrawArrays(GL_TRIANGLES, 0, (SAMPLES - 1) * (SAMPLES - 1) * 4 * 3); // cells * triangles per cell * vertices per triangle
	//glDrawArrays(GL_TRIANGLES, 0, 12); // cells * triangles per cell * vertices per triangle
	//glDrawArrays(GL_POINTS, 0, 3); // cells * triangles per cell * vertices per triangle
	//glDrawArrays(GL_POINTS, 0, (SAMPLES - 1) * (SAMPLES - 1) * 4 * 3); // cells * triangles per cell * vertices per triangle
}

auto main() -> int
{
	return run();
}
