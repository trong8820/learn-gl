// PBR Lighting

#include <vector>
#include <tuple>
#include <cmath>
#include <fstream>
#include <random>

#include "macros.h"
#include "entry.h"

#include "vec3.h"
#include "mat4.h"

#include "nanojpeg.h"

const float PI = 3.14159265358979f;

const char *vertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

out vec3 vFragPos;
out vec2 vUV;
out vec3 vNormal;

void main()
{
	vFragPos = vec3(world * vec4(aPos, 1.0));
	vUV = aUV;
	vNormal = mat3(world) * aNormal;
	gl_Position = proj * view * vec4(vFragPos, 1.0);
}
)";

const char *fragmentShaderSource = R"(
#version 410 core

const float PI = 3.14159265359;

const vec3 lightPoss[4] = vec3[4]
(
	vec3(-3.0, 5.0, -2.0),
	vec3(3.0, 5.0, -2.0),
	vec3(-3.0, 5.0, 2.0),
	vec3(3.0, 5.0, 2.0)
);

//const vec3 albedo = vec3(0.5, 0.0, 0.0);
//const float metallic = 0.1;
//const float roughness = 0.3;
//const float ao = 1.0;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

uniform vec3 eyePos;

in vec3 vFragPos;
in vec2 vUV;
in vec3 vNormal;

out vec4 FragColor;

vec3 getNormalFromMap()
{
	vec3 tangentNormal = texture(normalMap, vUV).xyz * 2.0 - 1.0;

	vec3 Q1  = dFdx(vFragPos);
	vec3 Q2  = dFdy(vFragPos);
	vec2 st1 = dFdx(vUV);
	vec2 st2 = dFdy(vUV);

	vec3 N   = normalize(vNormal);
	vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
	vec3 B  = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness*roughness;
	float a2 = a*a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;

	float nom   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / max(denom, 0.0000001); // prevent divide by zero for roughness=0.0 and NdotH=1.0
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float nom   = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

void main()
{
	vec3 albedo     = pow(texture(albedoMap, vUV).rgb, vec3(2.2));
	float metallic  = texture(metallicMap, vUV).r;
	float roughness = texture(roughnessMap, vUV).r;
	float ao        = texture(aoMap, vUV).r;

	vec3 N = getNormalFromMap();
	vec3 V = normalize(eyePos - vFragPos);
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	vec3 Lo = vec3(0.0);
	for(int i = 0; i < 4; ++i)
	{
		// calculate per-light radiance
		vec3 lightPos = lightPoss[i];
		vec3 L = normalize(lightPos - vFragPos);
		vec3 H = normalize(V + L);
		float distance = length(lightPos - vFragPos);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = vec3(20.0, 20.0, 20.0) * attenuation;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, roughness);
		float G   = GeometrySmith(N, V, L, roughness);
		vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

		vec3 numerator    = NDF * G * F;
		float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
		vec3 specular = numerator / max(denominator, 0.001);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;
		float NdotL = max(dot(N, L), 0.0);
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	vec3 ambient = vec3(0.03) * albedo * ao;
	vec3 color = ambient + Lo;
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2));
	FragColor = vec4(color, 1.0);
}
)";

GLuint gProgram;
GLuint gVAO;
GLint gWorldLoc;
GLint gViewLoc;
GLint gEyePosLoc;

GLsizei gIndexCount;

double gPrevPosX;
double gPrevPosY;
float gTargetRotX;
float gTargetRotY;
float gRotX;
float gRotY;

mat4 gWorld;

vec3 gEyePos;
mat4 gView;

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
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);

				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));

		GLuint EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);


	GLuint albedoTexture;
	glGenTextures(1, &albedoTexture);
	glBindTexture(GL_TEXTURE_2D, albedoTexture);
	loadTexture("data/rusted/albedo.jpg");
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint normalTexture;
	glGenTextures(1, &normalTexture);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	loadTexture("data/rusted/normal.jpg");
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint metallicTexture;
	glGenTextures(1, &metallicTexture);
	glBindTexture(GL_TEXTURE_2D, metallicTexture);
	loadTexture("data/rusted/metallic.jpg");
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint roughnessTexture;
	glGenTextures(1, &roughnessTexture);
	glBindTexture(GL_TEXTURE_2D, roughnessTexture);
	loadTexture("data/rusted/roughness.jpg");
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint aoTexture;
	glGenTextures(1, &aoTexture);
	glBindTexture(GL_TEXTURE_2D, aoTexture);
	loadTexture("data/rusted/ao.jpg");
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glUseProgram(gProgram);
	gWorldLoc = glGetUniformLocation(gProgram, "world");
	gViewLoc = glGetUniformLocation(gProgram, "view");
	gEyePosLoc = glGetUniformLocation(gProgram, "eyePos");

	glUniform1i(glGetUniformLocation(gProgram, "albedoMap"), 0);
	glUniform1i(glGetUniformLocation(gProgram, "normalMap"), 1);
	glUniform1i(glGetUniformLocation(gProgram, "metallicMap"), 2);
	glUniform1i(glGetUniformLocation(gProgram, "roughnessMap"), 3);
	glUniform1i(glGetUniformLocation(gProgram, "aoMap"), 4);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, albedoTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, metallicTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, roughnessTexture);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, aoTexture);

	gWorld = mat4::identity;

	on_size();

	return 0;
}

void on_size()
{
	//std::cout << "size " << gWidth << " " << gHeight << std::endl;
	glViewport(0, 0, gWidth, gHeight);

	glUseProgram(gProgram);
	//GLint worldLoc = glGetUniformLocation(gProgram, "world");
	//GLint viewLoc = glGetUniformLocation(gProgram, "view");
	GLint projLoc = glGetUniformLocation(gProgram, "proj");

	mat4 world = mat4::identity;
	mat4 view = mat4::lookAt(vec3(0.0f, 3.0f, 3.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 proj = mat4::perspective(45.0f * (PI/180.0f), static_cast<float>(gWidth)/gHeight, 0.1f, 100.0f);
	//glUniformMatrix4fv(worldLoc, 1, false, world.m);
	//glUniformMatrix4fv(viewLoc, 1, false, view.m);
	glUniformMatrix4fv(projLoc, 1, false, proj.m);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
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

	vec4 eyePos = mat4::rotate(0.0f, 1.0f, 0.0f, -gRotX) * mat4::rotate(1.0f, 0.0f, 0.0f, -gRotY) * vec4(0.0f, 0.0f, 5.0f, 0.0f);
	gEyePos = vec3(eyePos.x, eyePos.y, eyePos.z);
	gView = mat4::lookAt(gEyePos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
}

auto draw() -> void
{
	glViewport(0, 0, gWidth, gHeight);
	glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gProgram);
	glUniformMatrix4fv(gWorldLoc, 1, false, gWorld.m);
	glUniformMatrix4fv(gViewLoc, 1, false, gView.m);
	glUniform3f(gEyePosLoc, gEyePos.x, gEyePos.y, gEyePos.z);

	glBindVertexArray(gVAO);
	glDrawElements(GL_TRIANGLE_STRIP, gIndexCount, GL_UNSIGNED_INT, 0);
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
		}
	}

	bool oddRow = false;
	for (unsigned int y = 0; y < segments; ++y)
	{
		if (!oddRow) // even rows: y == 0, y == 2; and so on
		{
			for (unsigned int x = 0; x <= segments; ++x)
			{
				indices.push_back(y       * (segments + 1) + x);
				indices.push_back((y + 1) * (segments + 1) + x);
			}
		}
		else
		{
			for (int x = segments; x >= 0; --x)
			{
				indices.push_back((y + 1) * (segments + 1) + x);
				indices.push_back(y       * (segments + 1) + x);
			}
		}
		oddRow = !oddRow;
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
