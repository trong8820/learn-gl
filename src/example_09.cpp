// Normal Mapping

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

const vec3 lightPos = vec3(-3.0, 5.0, -1.0);
const vec3 viewPos = vec3(0.0, 3.0, 3.0);

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

out vec3 vFragPos;
out vec2 vUV;
out vec3 vTangentLightPos;
out vec3 vTangentViewPos;
out vec3 vTangentFragPos;

void main()
{
	vFragPos = vec3(world * vec4(aPos, 1.0));
	vUV = aUV;

	mat3 normalMatrix = transpose(inverse(mat3(world)));
	vec3 N = normalize(normalMatrix * aNormal);
	vec3 T = normalize(normalMatrix * aTangent);
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);

	mat3 TBN = transpose(mat3(T, B, N));
	vTangentLightPos = TBN * lightPos;
	vTangentViewPos = TBN * viewPos;
	vTangentFragPos = TBN * vFragPos;

	gl_Position = proj * view * world * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 410 core

const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const vec3 objectColor = vec3(1.0, 1.0, 1.0);

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

in vec3 vFragPos;
in vec2 vUV;
in vec3 vTangentLightPos;
in vec3 vTangentViewPos;
in vec3 vTangentFragPos;

out vec4 FragColor;

void main()
{
	vec3 color = texture(diffuseMap, vUV).rgb;
	vec3 normal = texture(normalMap, vUV).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	// ambient
	float ambientStrength = 0.6;
	vec3 ambient = ambientStrength * lightColor * color;

	// diffuse
	vec3 lightDir = normalize(vTangentLightPos - vTangentFragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * color * lightColor;

	// specular - Blinn-Phong
	float specularStrength = 0.3;
	vec3 viewDir = normalize(vTangentViewPos - vTangentFragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
	vec3 specular = specularStrength * spec * lightColor;

	vec3 result = (ambient + diffuse + specular) * objectColor;
	FragColor = vec4(result, 1.0);
}
)";

GLuint gProgram;
GLuint gVAO;

GLsizei gIndexCount;

std::tuple<std::vector<float>, std::vector<unsigned int>> sphere(unsigned int segments);
void loadTexture(const std::string& path);

auto init() -> bool
{
	auto sphereData = sphere(64);
	std::vector<float> vertices = std::get<0>(sphereData);
	std::vector<unsigned int> indices = std::get<1>(sphereData);
	gIndexCount = indices.size();

	//std::cout << "init " << gWidth << " " << gHeight << std::endl;
	auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
	GL_CHECK(glShaderSource(vertexShader, 1, &vertexShaderSource, NULL));
	GL_CHECK(glCompileShader(vertexShader));

	auto fragmentShader = GL_CHECK_RETURN(glCreateShader(GL_FRAGMENT_SHADER));
	GL_CHECK(glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL));
	GL_CHECK(glCompileShader(fragmentShader));

	gProgram = GL_CHECK_RETURN(glCreateProgram());
	GL_CHECK(glAttachShader(gProgram, vertexShader));
	GL_CHECK(glAttachShader(gProgram, fragmentShader));
	GL_CHECK(glLinkProgram(gProgram));

	GL_CHECK(glDeleteShader(vertexShader));
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
	loadTexture("data/brickwall.jpg");
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint normalTexture;
	glGenTextures(1, &normalTexture);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	loadTexture("data/brickwall_normal.jpg");
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glUseProgram(gProgram);
	glUniform1i(glGetUniformLocation(gProgram, "diffuseMap"), 0);
	glUniform1i(glGetUniformLocation(gProgram, "normalMap"), 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalTexture);

	on_size();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

	return 0;
}

void on_size()
{
	//std::cout << "size " << gWidth << " " << gHeight << std::endl;
	glViewport(0, 0, gWidth, gHeight);

	glUseProgram(gProgram);
	GLint worldLoc = glGetUniformLocation(gProgram, "world");
	GLint viewLoc = glGetUniformLocation(gProgram, "view");
	GLint projLoc = glGetUniformLocation(gProgram, "proj");

	mat4 world = mat4::identity;
	mat4 view = mat4::lookAt(vec3(0.0f, 3.0f, 3.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 proj = mat4::perspective(45.0f * (PI / 180.0f), static_cast<float>(gWidth) / gHeight, 0.1f, 100.0f);
	glUniformMatrix4fv(worldLoc, 1, false, world.m);
	glUniformMatrix4fv(viewLoc, 1, false, view.m);
	glUniformMatrix4fv(projLoc, 1, false, proj.m);
}

void on_key(int key, int action)
{

}

auto update() -> void
{

}

auto draw() -> void
{
	glViewport(0, 0, gWidth, gHeight);
	glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gProgram);
	glBindVertexArray(gVAO);

	glDrawElements(GL_TRIANGLES, gIndexCount, GL_UNSIGNED_INT, 0);
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
