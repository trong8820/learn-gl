// Displacement Mapping with tessellation

#include <vector>
#include <tuple>
#include <cmath>
#include <fstream>

#include "macros.h"
#include "entry.h"

#include "vec3.h"
#include "mat4.h"

#include "nanojpeg.h"

const float PI = 3.14159265358979f;

const char* vertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

uniform vec3 eyePos;

out vec3 vPos;
out vec2 vUV;
out vec3 vNormal;

void main()
{
	vPos = aPos;
	vUV = aUV;
	vNormal = aNormal;
}
)";

// Tessellation Control
const char *tcShaderSource = R"(
#version 410 core

in vec3 vPos[];
in vec2 vUV[];
in vec3 vNormal[];

layout(vertices=3) out;
out vec3 tcPos[];
out vec2 tcUV[];
out vec3 tcNormal[];

void main()
{
	tcPos[gl_InvocationID] = vPos[gl_InvocationID];
	tcUV[gl_InvocationID] = vUV[gl_InvocationID];
	tcNormal[gl_InvocationID] = vNormal[gl_InvocationID];

	if (gl_InvocationID == 0)
	{
		gl_TessLevelOuter[0] = 9;
		gl_TessLevelOuter[1] = 9;
		gl_TessLevelOuter[2] = 9;

		gl_TessLevelInner[0] = 9;
	}
}
)";

// Tessellation Evaluation
const char *teShaderSource = R"(
#version 410 core

layout(triangles,equal_spacing) in;
in vec3 tcPos[];
in vec2 tcUV[];
in vec3 tcNormal[];

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

uniform sampler2D heightMap;

out vec3 teFragPos;
out vec2 teUV;
out vec3 teNormal;

vec2 lerp2D(vec2 v0, vec2 v1, vec2 v2)
{
	return gl_TessCoord.x * v0 + gl_TessCoord.y * v1 + gl_TessCoord.z * v2;
}

vec3 lerp3D(vec3 v0, vec3 v1, vec3 v2)
{
	return gl_TessCoord.x * v0 + gl_TessCoord.y * v1 + gl_TessCoord.z * v2;
}

void main()
{
	vec3 pos = normalize(lerp3D(tcPos[0], tcPos[1], tcPos[2]));
	teUV = lerp2D(tcUV[0], tcUV[1], tcUV[2]);
	teNormal = normalize(lerp3D(tcNormal[0], tcNormal[1], tcNormal[2]));

	float height =  texture(heightMap, teUV).r;
	pos += teNormal*(height - 0.8)*0.3;
	teFragPos = vec3(world * vec4(pos, 1.0));
	gl_Position = proj * view * world * vec4(pos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 410 core

const vec3 lightPos = vec3(-3.0, 5.0, -1.0);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const vec3 objectColor = vec3(1.0, 1.0, 1.0);

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D heightMap;

uniform vec3 eyePos;

in vec3 teFragPos;
in vec2 teUV;
in vec3 teNormal;

out vec4 FragColor;

void main()
{
	//float height =  texture(heightMap, teUV).r;
	//FragColor = vec4(height, 0.0, 0.0, 1.0);

	vec3 color = texture(diffuseMap, teUV).rgb;
	FragColor = vec4(color, 1.0);
}
)";

GLuint gProgram;
GLuint gVAO;
GLint gViewLoc;
GLint gEyePosLoc;

GLsizei gIndexCount;

double gPrevPosX;
double gPrevPosY;
float gTargetRotX;
float gTargetRotY;
float gRotX;
float gRotY;

vec3 gEyePos;
mat4 gView;

std::tuple<std::vector<float>, std::vector<unsigned int>> sphere(unsigned int segments);
void loadTexture(const std::string& path);

auto init() -> bool
{
	auto sphereData = sphere(16);
	std::vector<float> vertices = std::get<0>(sphereData);
	std::vector<unsigned int> indices = std::get<1>(sphereData);
	gIndexCount = indices.size();

	//std::cout << "init " << gWidth << " " << gHeight << std::endl;
	auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
	GL_CHECK(glShaderSource(vertexShader, 1, &vertexShaderSource, NULL));
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

	auto tcShader = glCreateShader(GL_TESS_CONTROL_SHADER);
	glShaderSource(tcShader, 1, &tcShaderSource, nullptr);
	GL_CHECK(glCompileShader(tcShader));
	{
		GLint success;
		GL_CHECK(glGetShaderiv(tcShader, GL_COMPILE_STATUS, &success));
		if(!success)
		{
			GLint infoLength = 0;
			glGetShaderiv(tcShader, GL_INFO_LOG_LENGTH, &infoLength);
			char* infoLog = new char[infoLength];
			GL_CHECK(glGetShaderInfoLog(tcShader, infoLength, &infoLength, infoLog));
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "TESS_CONTROL_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			delete [] infoLog;
			GL_CHECK(glDeleteShader(tcShader));
		}
	}

	auto teShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
	glShaderSource(teShader, 1, &teShaderSource, nullptr);
	GL_CHECK(glCompileShader(teShader));
	{
		GLint success;
		GL_CHECK(glGetShaderiv(teShader, GL_COMPILE_STATUS, &success));
		if(!success)
		{
			GLint infoLength = 0;
			glGetShaderiv(teShader, GL_INFO_LOG_LENGTH, &infoLength);
			char* infoLog = new char[infoLength];
			GL_CHECK(glGetShaderInfoLog(teShader, infoLength, &infoLength, infoLog));
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "TESS_EVALUATION_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			delete [] infoLog;
			GL_CHECK(glDeleteShader(teShader));
		}
	}

	auto fragmentShader = GL_CHECK_RETURN(glCreateShader(GL_FRAGMENT_SHADER));
	GL_CHECK(glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL));
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

	gProgram = GL_CHECK_RETURN(glCreateProgram());
	GL_CHECK(glAttachShader(gProgram, vertexShader));
	GL_CHECK(glAttachShader(gProgram, tcShader));
	GL_CHECK(glAttachShader(gProgram, teShader));
	GL_CHECK(glAttachShader(gProgram, fragmentShader));
	GL_CHECK(glLinkProgram(gProgram));

	GL_CHECK(glDeleteShader(vertexShader));
	GL_CHECK(glDeleteShader(tcShader));
	GL_CHECK(glDeleteShader(teShader));
	GL_CHECK(glDeleteShader(fragmentShader));

	glGenVertexArrays(1, &gVAO);
	glBindVertexArray(gVAO);
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));

	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	GLuint diffuseTexture;
	glGenTextures(1, &diffuseTexture);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture);
	loadTexture("data/magicmoon.jpg");
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint normalTexture;
	glGenTextures(1, &normalTexture);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	loadTexture("data/bricks2_normal.jpg");
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint heightTexture;
	glGenTextures(1, &heightTexture);
	glBindTexture(GL_TEXTURE_2D, heightTexture);
	loadTexture("data/magicmoon_heightmap.jpg");
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glUseProgram(gProgram);
	glUniform1i(glGetUniformLocation(gProgram, "diffuseMap"), 0);
	glUniform1i(glGetUniformLocation(gProgram, "normalMap"), 1);
	glUniform1i(glGetUniformLocation(gProgram, "heightMap"), 2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, heightTexture);

	gViewLoc = glGetUniformLocation(gProgram, "view");
	gEyePosLoc = glGetUniformLocation(gProgram, "eyePos");

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

	on_size();

	glfwGetCursorPos(g_pWindow, &gPrevPosX, &gPrevPosY);

	return 0;
}

void on_size()
{
	//std::cout << "size " << gWidth << " " << gHeight << std::endl;
	glViewport(0, 0, gWidth, gHeight);

	glUseProgram(gProgram);
	GLint worldLoc = glGetUniformLocation(gProgram, "world");
	//GLint viewLoc = glGetUniformLocation(gProgram, "view");
	GLint projLoc = glGetUniformLocation(gProgram, "proj");

	mat4 world = mat4::identity;
	mat4 view = mat4::lookAt(vec3(0.0f, 3.0f, 3.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 proj = mat4::perspective(45.0f * (PI / 180.0f), static_cast<float>(gWidth) / gHeight, 0.1f, 100.0f);
	glUniformMatrix4fv(worldLoc, 1, false, world.m);
	//glUniformMatrix4fv(viewLoc, 1, false, view.m);
	glUniformMatrix4fv(projLoc, 1, false, proj.m);
}

void on_key(int key, int action)
{

}

void on_mouse(double xpos, double ypos)
{
	int state = glfwGetMouseButton(g_pWindow, GLFW_MOUSE_BUTTON_LEFT);
	if (state == GLFW_PRESS)
	{
		gTargetRotX += (xpos - gPrevPosX)*0.01f;
		gTargetRotY += (ypos - gPrevPosY)*0.01f;

		if (gTargetRotY <= -PI / 2.0f) gTargetRotY = -PI / 2.0f + 0.01f;
		if (gTargetRotY >= PI / 2.0f)  gTargetRotY = PI / 2.0f - 0.01f;
	}
	gPrevPosX = xpos;
	gPrevPosY = ypos;
}

auto update() -> void
{
	gRotX += 0.05 * (gTargetRotX - gRotX);
	gRotY += 0.05 * (gTargetRotY - gRotY);

	vec4 eyePos = mat4::rotate(0.0f, 1.0f, 0.0f, -gRotX) * mat4::rotate(1.0f, 0.0f, 0.0f, -gRotY) * vec4(0.0f, 0.0f, 3.0f, 1.0f);
	gEyePos = vec3(eyePos.x, eyePos.y, eyePos.z);
	gView = mat4::lookAt(gEyePos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
}

auto draw() -> void
{
	glViewport(0, 0, gWidth, gHeight);
	glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gProgram);
	glUniformMatrix4fv(gViewLoc, 1, false, gView.m);
	glUniform3f(gEyePosLoc, gEyePos.x, gEyePos.y, gEyePos.z);
	glBindVertexArray(gVAO);

	glDrawElements(GL_PATCHES, gIndexCount, GL_UNSIGNED_INT, 0);
}

auto main() -> int
{
	return run();
}

std::tuple<std::vector<float>, std::vector<unsigned int>> sphere(unsigned int segments)
{
	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	for (unsigned int y = 0; y <= segments; y++)
	{
		for (unsigned int x = 0; x <= segments; x++)
		{
			float xSegment = (float)x / (float)segments;
			float ySegment = (float)y / (float)segments;

			float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
			float yPos = std::cos(ySegment * PI);
			float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

			vertices.push_back(xPos);
			vertices.push_back(yPos);
			vertices.push_back(zPos);

			vertices.push_back(xSegment);
			vertices.push_back(ySegment);

			vertices.push_back(xPos);
			vertices.push_back(yPos);
			vertices.push_back(zPos);

			vertices.push_back(-std::sin(xSegment * 2.0f * PI));
			vertices.push_back(0.0f);
			vertices.push_back(std::cos(xSegment * 2.0f * PI));
		}
	}

	for (unsigned int y = 0; y < segments; ++y)
	{
		for (unsigned int x = 0; x < segments; x++)
		{
			unsigned int i0 = (y + 1) * (segments + 1) + x;
			unsigned int i1 = y * (segments + 1) + x;
			unsigned int i2 = y * (segments + 1) + x + 1;
			unsigned int i3 = (y + 1) * (segments + 1) + x + 1;
			indices.push_back(i0);
			indices.push_back(i1);
			indices.push_back(i2);

			indices.push_back(i0);
			indices.push_back(i2);
			indices.push_back(i3);
		}
	}

	return std::make_tuple(vertices, indices);
}

void loadTexture(const std::string& path)
{
	njInit();
	std::ifstream ifs(path, std::ios::binary | std::ios::ate);
	std::ifstream::pos_type pos = ifs.tellg();
	size_t size = static_cast<size_t>(pos);
	ifs.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	if (ifs.read(buffer.data(), size))
	{
		if (njDecode(buffer.data(), size) == NJ_OK)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, njGetWidth(), njGetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, njGetImage());
		}
	}

	ifs.close();
	njDone();
}
