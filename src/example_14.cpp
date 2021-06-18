// SSAO - screen space ambient occlusion

#include <vector>
#include <tuple>
#include <cmath>
#include <sstream>
#include <fstream>
#include <random>

#include "macros.h"
#include "entry.h"

#include "vec3.h"
#include "mat4.h"

#include "nanojpeg.h"

const float PI = 3.14159265358979f;

float vertices[] =
{
	// back face
	-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
	1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
	1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
	1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
	-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
	-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
	// front face
	-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
	1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
	1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
	1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
	-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
	-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
	// left face
	-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
	-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
	-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
	-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
	-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
	-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
	// right face
	1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
	1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
	1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
	1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
	1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
	1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
	// bottom face
	-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
	1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
	1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
	1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
	-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
	-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
	// top face
	-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
	1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
	1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
	1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
	-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
	-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
};

const char* vertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

out vec3 vFragPos;
out vec3 vNormal;
out vec2 vUV;

void main()
{
	vFragPos = vec3(view * world * vec4(aPos, 1.0));
	vUV = aUV;

	mat3 normalMatrix = transpose(inverse(mat3(view * world)));
	vNormal = normalize(normalMatrix * aNormal);

	gl_Position = proj * view * world * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 410 core

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vUV;

layout (location = 0) out vec3 fPosition;
layout (location = 1) out vec3 fNormal;
layout (location = 2) out vec4 fAlbedo;

void main()
{
	fPosition = vFragPos;
	fNormal = vNormal;
	fAlbedo = vec4(0.9, 0.9, 0.9, 1.0);
}
)";

const char* ssaoVertexShaderSource = R"(
#version 410 core

const vec4 pos[4] = vec4[4](vec4( 1.0, -1.0, 0.0, 1.0),
							vec4(-1.0, -1.0, 0.0, 1.0),
							vec4(-1.0,  1.0, 0.0, 1.0),
							vec4( 1.0,  1.0, 0.0, 1.0));
const vec2 uvs[4] = vec2[4](vec2(1.0, 0.0),
							vec2(0.0, 0.0),
							vec2(0.0, 1.0),
							vec2(1.0, 1.0));

out vec2 vUV;

void main()
{
	vUV = uvs[gl_VertexID];
	gl_Position = pos[gl_VertexID];
}
)";

const char* ssaoFragmentShaderSource = R"(
#version 410 core

const int kernelSize = 64;
const float radius = 0.5;
const float bias = 0.025;

uniform sampler2D positionMap;
uniform sampler2D normalMap;
uniform sampler2D noiseMap;

uniform mat4 proj;

uniform float screenWidth;
uniform float screenHeight;
uniform vec3 samples[64];

in vec2 vUV;

layout (location = 0) out float fSSAO;

void main()
{
	vec2 noiseScale = vec2(screenWidth/4.0, screenHeight/4.0);

	vec3 fragPos = texture(positionMap, vUV).xyz;
	vec3 normal = normalize(texture(normalMap, vUV).rgb);
	vec3 randomVec = normalize(texture(noiseMap, vUV * noiseScale).xyz);

	vec3 T = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 B = cross(normal, T);
	mat3 TBN = mat3(T, B, normal);

	float occlusion = 0.0;
	for (int i=0; i<kernelSize; i++)
	{
		vec3 samplePos = TBN * samples[i];
		samplePos = fragPos + samplePos * radius;

		vec4 offset = proj * vec4(samplePos, 1.0);
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5 + 0.5;

		float sampleDepth = texture(positionMap, vUV).z;

		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion = 1.0 - (occlusion / kernelSize);
	fSSAO = pow(occlusion, 8.0);
}
)";

const char* ssaoBlurVertexShaderSource = R"(
#version 410 core

const vec4 pos[4] = vec4[4](vec4( 1.0, -1.0, 0.0, 1.0),
							vec4(-1.0, -1.0, 0.0, 1.0),
							vec4(-1.0,  1.0, 0.0, 1.0),
							vec4( 1.0,  1.0, 0.0, 1.0));
const vec2 uvs[4] = vec2[4](vec2(1.0, 0.0),
							vec2(0.0, 0.0),
							vec2(0.0, 1.0),
							vec2(1.0, 1.0));

out vec2 vUV;

void main()
{
	vUV = uvs[gl_VertexID];
	gl_Position = pos[gl_VertexID];
}
)";

const char* ssaoBlurFragmentShaderSource = R"(
#version 410 core

uniform sampler2D ssaoMap;

in vec2 vUV;

layout (location = 0) out float fSSAO;

void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(ssaoMap, 0));
	float result = 0.0;
	for (int x = -2; x < 2; ++x)
	{
		for (int y = -2; y < 2; ++y)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(ssaoMap, vUV + offset).r;
		}
	}
	fSSAO = result / (4.0 * 4.0);
}
)";

const char* deferredVertexShaderSource = R"(
#version 410 core

const vec4 pos[4] = vec4[4](vec4( 1.0, -1.0, 0.0, 1.0),
							vec4(-1.0, -1.0, 0.0, 1.0),
							vec4(-1.0,  1.0, 0.0, 1.0),
							vec4( 1.0,  1.0, 0.0, 1.0));
const vec2 uvs[4] = vec2[4](vec2(1.0, 0.0),
							vec2(0.0, 0.0),
							vec2(0.0, 1.0),
							vec2(1.0, 1.0));

out vec2 vUV;

void main()
{
	vUV = uvs[gl_VertexID];
	gl_Position = pos[gl_VertexID];
}
)";

const char* deferredFragmentShaderSource = R"(
#version 410 core

//const vec3 lightPos = vec3(-3.0, 5.0, -1.0);
const vec3 lightPos = vec3(-6.0, 10.0, -2.0);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);

uniform sampler2D positionMap;
uniform sampler2D normalMap;
uniform sampler2D albedoMap;
uniform sampler2D ssaoMap;

uniform mat4 view;

uniform vec3 eyePos;

in vec2 vUV;

out vec4 FragColor;

void main()
{
	vec3 fragPos = texture(positionMap, vUV).rgb;
	vec3 normal = texture(normalMap, vUV).rgb;
	vec3 color = texture(albedoMap, vUV).rgb;
	float ssao = texture(ssaoMap, vUV).x;

	normal = normalize(normal);

	// ambient
	float ambientStrength = 0.6;
	vec3 ambient = ambientStrength * ssao * color;

	// diffuse
	vec3 lightDir = normalize(vec3(view * vec4(lightPos, 1.0)) - fragPos);
	vec3 diffuse = max(dot(normal, lightDir), 0.0) * color * lightColor;

	// specular - Blinn-Phong
	float specularStrength = 0.2;
	vec3 viewDir  = normalize(eyePos - fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
	vec3 specular = specularStrength * spec * lightColor;

	vec3 result = (ambient + diffuse + specular);

	FragColor = vec4(result, 1.0);
}
)";

GLuint gProgram;
GLuint gCubeVAO;
GLuint gBackpackVAO;
GLint gWorldLoc;
GLint gViewLoc;
GLint gEyePosLoc;

GLsizei gBackpackVerticesCount;

GLuint gSSAOProgram;
GLuint gSSAOBlurProgram;
GLuint gDeferredProgram;
GLint gDeferredViewLoc;

double gPrevPosX;
double gPrevPosY;
float gTargetRotX;
float gTargetRotY;
float gRotX;
float gRotY;

vec3 gEyePos;
mat4 gView;

GLuint gGFBO;
GLuint gSSAOFBO;
GLuint gSSAOBlurFBO;

GLuint gPositionTexture;
GLuint gNormalTexture;
GLuint gAlbedoTexture;
GLuint gSSAOTexture;
GLuint gSSAOBlurTexture;
GLuint gNoiseTexture;

GLuint gDepthRenderBuffer;

mat4 gCubeWorld;
mat4 gBackpackWorld;

std::vector<std::string> split(const std::string& str, char delim);

auto init() -> bool
{
	std::random_device rd;
	std::default_random_engine re(rd());
	std::uniform_real_distribution<float> real_dist(0.0f, 1.0f);

	// read backpack.obj
	std::ifstream ifs("data/backpack.obj");
	std::string line;
	std::vector<float> backpackPositions;
	std::vector<float> backpackUVs;
	std::vector<float> backpackNormals;
	std::vector<float> backpackVertices;
	while(std::getline(ifs, line))
	{
		if (line[0] == '#') continue;
		std::vector<std::string> strs = split(line, ' ');
		if (strs[0] == "v")
		{
			backpackPositions.push_back(std::stof(strs[1]));
			backpackPositions.push_back(std::stof(strs[2]));
			backpackPositions.push_back(std::stof(strs[3]));
		}

		if (strs[0] == "vt")
		{
			backpackUVs.push_back(std::stof(strs[1]));
			backpackUVs.push_back(std::stof(strs[2]));
		}

		if (strs[0] == "vn")
		{
			backpackNormals.push_back(std::stof(strs[1]));
			backpackNormals.push_back(std::stof(strs[2]));
			backpackNormals.push_back(std::stof(strs[3]));
		}

		if (strs[0] == "f")
		{
			std::vector<std::string> v0 = split(strs[1], '/');
			std::vector<std::string> v1 = split(strs[2], '/');
			std::vector<std::string> v2 = split(strs[3], '/');

			// v0
			int pos0Id = std::stoi(v0[0]) - 1;
			int uv0Id = std::stoi(v0[1]) - 1;
			int normal0Id = std::stoi(v0[2]) - 1;
			backpackVertices.push_back(backpackPositions[pos0Id*3 + 0]);
			backpackVertices.push_back(backpackPositions[pos0Id*3 + 1]);
			backpackVertices.push_back(backpackPositions[pos0Id*3 + 2]);

			backpackVertices.push_back(backpackNormals[normal0Id*3 + 0]);
			backpackVertices.push_back(backpackNormals[normal0Id*3 + 1]);
			backpackVertices.push_back(backpackNormals[normal0Id*3 + 2]);

			backpackVertices.push_back(backpackUVs[uv0Id*2 + 0]);
			backpackVertices.push_back(backpackUVs[uv0Id*2 + 1]);

			// v1
			int pos1Id = std::stoi(v1[0]) - 1;
			int uv1Id = std::stoi(v1[1]) - 1;
			int normal1Id = std::stoi(v1[2]) - 1;
			backpackVertices.push_back(backpackPositions[pos1Id*3 + 0]);
			backpackVertices.push_back(backpackPositions[pos1Id*3 + 1]);
			backpackVertices.push_back(backpackPositions[pos1Id*3 + 2]);

			backpackVertices.push_back(backpackNormals[normal1Id*3 + 0]);
			backpackVertices.push_back(backpackNormals[normal1Id*3 + 1]);
			backpackVertices.push_back(backpackNormals[normal1Id*3 + 2]);

			backpackVertices.push_back(backpackUVs[uv1Id*2 + 0]);
			backpackVertices.push_back(backpackUVs[uv1Id*2 + 1]);

			// v2
			int pos2Id = std::stoi(v2[0]) - 1;
			int uv2Id = std::stoi(v2[1]) - 1;
			int normal2Id = std::stoi(v2[2]) - 1;
			backpackVertices.push_back(backpackPositions[pos2Id*3 + 0]);
			backpackVertices.push_back(backpackPositions[pos2Id*3 + 1]);
			backpackVertices.push_back(backpackPositions[pos2Id*3 + 2]);

			backpackVertices.push_back(backpackNormals[normal2Id*3 + 0]);
			backpackVertices.push_back(backpackNormals[normal2Id*3 + 1]);
			backpackVertices.push_back(backpackNormals[normal2Id*3 + 2]);

			backpackVertices.push_back(backpackUVs[uv2Id*2 + 0]);
			backpackVertices.push_back(backpackUVs[uv2Id*2 + 1]);

			// count
			gBackpackVerticesCount += 3;
		}
	}
	ifs.close();

	{
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
		GL_CHECK(glAttachShader(gProgram, fragmentShader));
		GL_CHECK(glLinkProgram(gProgram));

		GL_CHECK(glDeleteShader(vertexShader));
		GL_CHECK(glDeleteShader(fragmentShader));
	}

	{
		auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
		GL_CHECK(glShaderSource(vertexShader, 1, &ssaoVertexShaderSource, NULL));
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
		GL_CHECK(glShaderSource(fragmentShader, 1, &ssaoFragmentShaderSource, NULL));
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

		gSSAOProgram = GL_CHECK_RETURN(glCreateProgram());
		GL_CHECK(glAttachShader(gSSAOProgram, vertexShader));
		GL_CHECK(glAttachShader(gSSAOProgram, fragmentShader));
		GL_CHECK(glLinkProgram(gSSAOProgram));

		GL_CHECK(glDeleteShader(vertexShader));
		GL_CHECK(glDeleteShader(fragmentShader));
	}

	{
		auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
		GL_CHECK(glShaderSource(vertexShader, 1, &ssaoBlurVertexShaderSource, NULL));
		GL_CHECK(glCompileShader(vertexShader));
		{
			GLint success;
			GL_CHECK(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success));
			if (!success)
			{
				GLint infoLength = 0;
				glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLength);
				char* infoLog = new char[infoLength];
				GL_CHECK(glGetShaderInfoLog(vertexShader, infoLength, &infoLength, infoLog));
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "VERTEX_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				delete[] infoLog;
				GL_CHECK(glDeleteShader(vertexShader));
			}
		}

		auto fragmentShader = GL_CHECK_RETURN(glCreateShader(GL_FRAGMENT_SHADER));
		GL_CHECK(glShaderSource(fragmentShader, 1, &ssaoBlurFragmentShaderSource, NULL));
		GL_CHECK(glCompileShader(fragmentShader));
		{
			GLint success;
			GL_CHECK(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success));
			if (!success)
			{
				GLint infoLength = 0;
				glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLength);
				char* infoLog = new char[infoLength];
				GL_CHECK(glGetShaderInfoLog(fragmentShader, infoLength, &infoLength, infoLog));
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "FRAGMENT_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				delete[] infoLog;
				GL_CHECK(glDeleteShader(fragmentShader));
			}
		}

		gSSAOBlurProgram = GL_CHECK_RETURN(glCreateProgram());
		GL_CHECK(glAttachShader(gSSAOBlurProgram, vertexShader));
		GL_CHECK(glAttachShader(gSSAOBlurProgram, fragmentShader));
		GL_CHECK(glLinkProgram(gSSAOBlurProgram));

		GL_CHECK(glDeleteShader(vertexShader));
		GL_CHECK(glDeleteShader(fragmentShader));
	}

	{
		auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
		GL_CHECK(glShaderSource(vertexShader, 1, &deferredVertexShaderSource, NULL));
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
		GL_CHECK(glShaderSource(fragmentShader, 1, &deferredFragmentShaderSource, NULL));
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

		gDeferredProgram = GL_CHECK_RETURN(glCreateProgram());
		GL_CHECK(glAttachShader(gDeferredProgram, vertexShader));
		GL_CHECK(glAttachShader(gDeferredProgram, fragmentShader));
		GL_CHECK(glLinkProgram(gDeferredProgram));

		GL_CHECK(glDeleteShader(vertexShader));
		GL_CHECK(glDeleteShader(fragmentShader));
	}

	{
		glGenVertexArrays(1, &gCubeVAO);
		glBindVertexArray(gCubeVAO);
		GLuint VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	}

	{
		glGenVertexArrays(1, &gBackpackVAO);
		glBindVertexArray(gBackpackVAO);
		GLuint VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, backpackVertices.size() * sizeof(float), &backpackVertices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	}

	// g-buffer
	glGenFramebuffers(1, &gGFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, gGFBO);

	glGenTextures(1, &gPositionTexture);
	glBindTexture(GL_TEXTURE_2D, gPositionTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gWidth, gHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPositionTexture, 0);

	glGenTextures(1, &gNormalTexture);
	glBindTexture(GL_TEXTURE_2D, gNormalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gWidth, gHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormalTexture, 0);

	glGenTextures(1, &gAlbedoTexture);
	glBindTexture(GL_TEXTURE_2D, gAlbedoTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gWidth, gHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoTexture, 0);

	{
		GLenum attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachments);
	}

	glGenRenderbuffers(1, &gDepthRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, gDepthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, gWidth, gHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepthRenderBuffer);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ssao buffer
	glGenFramebuffers(1, &gSSAOFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, gSSAOFBO);
	glGenTextures(1, &gSSAOTexture);
	glBindTexture(GL_TEXTURE_2D, gSSAOTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, gWidth, gHeight, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gSSAOTexture, 0);
	{
		GLenum attachments[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachments);
	}
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ssao blur buffer
	glGenFramebuffers(1, &gSSAOBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, gSSAOBlurFBO);
	glGenTextures(1, &gSSAOBlurTexture);
	glBindTexture(GL_TEXTURE_2D, gSSAOBlurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, gWidth, gHeight, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gSSAOBlurTexture, 0);
	{
		GLenum attachments[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachments);
	}
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(gProgram);
	gWorldLoc = glGetUniformLocation(gProgram, "world");
	gViewLoc = glGetUniformLocation(gProgram, "view");

	glUseProgram(gSSAOProgram);
	glUniform1i(glGetUniformLocation(gSSAOProgram, "positionMap"), 0);
	glUniform1i(glGetUniformLocation(gSSAOProgram, "normalMap"), 1);
	glUniform1i(glGetUniformLocation(gSSAOProgram, "noiseMap"), 5);

	for (size_t i = 0; i < 64; i++)
	{
		vec3 sample(real_dist(re) * 2.0 - 1.0, real_dist(re) * 2.0 - 1.0, real_dist(re));
		sample = sample.normalize();
		sample = sample * real_dist(re);
		float scale = float(i) / 64.0;

		// scale samples s.t. they're more aligned to center of kernel
		scale = 0.1f + (1.0f-0.1f)*(scale * scale);
		sample = sample * scale;

		glUniform3f(glGetUniformLocation(gSSAOProgram, ("samples[" +std::to_string(i) + "]").c_str()), sample.x, sample.y, sample.z);
	}

	std::vector<float> noise;
	for (unsigned int i = 0; i < 16; i++)
	{
		noise.push_back(real_dist(re) * 2.0 - 1.0);
		noise.push_back(real_dist(re) * 2.0 - 1.0);
		noise.push_back(0.0f);
	}

	glGenTextures(1, &gNoiseTexture);
	glBindTexture(GL_TEXTURE_2D, gNoiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &noise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, gNoiseTexture);

	glUseProgram(gSSAOBlurProgram);
	glUniform1i(glGetUniformLocation(gSSAOBlurProgram, "ssaoMap"), 3);

	glUseProgram(gDeferredProgram);
	gEyePosLoc = glGetUniformLocation(gDeferredProgram, "eyePos");
	gDeferredViewLoc = glGetUniformLocation(gDeferredProgram, "view");
	glUniform1i(glGetUniformLocation(gDeferredProgram, "positionMap"), 0);
	glUniform1i(glGetUniformLocation(gDeferredProgram, "normalMap"), 1);
	glUniform1i(glGetUniformLocation(gDeferredProgram, "albedoMap"), 2);
	glUniform1i(glGetUniformLocation(gDeferredProgram, "ssaoMap"), 4);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPositionTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormalTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gSSAOTexture);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gSSAOBlurTexture);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

	gCubeWorld = mat4::scale(0.2f, 0.2f, 0.2f) * mat4::translate(-6.0f, 10.0f, -2.0f);
	gBackpackWorld = mat4::scale(3.6f, 3.6f, 3.6f);

	on_size();

	glfwGetCursorPos(g_pWindow, &gPrevPosX, &gPrevPosY);

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

	//mat4 world = mat4::scale(50.0f, 50.0f, 50.0f);
	//mat4 view = mat4::lookAt(vec3(0.0f, 3.0f, 3.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 proj = mat4::perspective(45.0f * (PI / 180.0f), static_cast<float>(gWidth) / gHeight, 0.1f, 200.0f);
	//glUniformMatrix4fv(worldLoc, 1, false, world.m);
	//glUniformMatrix4fv(viewLoc, 1, false, view.m);
	glUniformMatrix4fv(projLoc, 1, false, proj.m);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPositionTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gWidth, gHeight, 0, GL_RGBA, GL_FLOAT, NULL);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gWidth, gHeight, 0, GL_RGBA, GL_FLOAT, NULL);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gWidth, gHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gSSAOTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, gWidth, gHeight, 0, GL_RED, GL_FLOAT, NULL);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gSSAOBlurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, gWidth, gHeight, 0, GL_RED, GL_FLOAT, NULL);

	glBindRenderbuffer(GL_RENDERBUFFER, gDepthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, gWidth, gHeight);

	glUseProgram(gSSAOProgram);
	glUniformMatrix4fv(glGetUniformLocation(gSSAOProgram, "proj"), 1, false, proj.m);
	glUniform1f(glGetUniformLocation(gSSAOProgram, "screenWidth"), gWidth);
	glUniform1f(glGetUniformLocation(gSSAOProgram, "screenHeight"), gHeight);
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

	vec4 eyePos = mat4::rotate(0.0f, 1.0f, 0.0f, -gRotX) * mat4::rotate(1.0f, 0.0f, 0.0f, -gRotY) * vec4(0.0f, 0.0f, 30.0f, 1.0f);
	gEyePos = vec3(eyePos.x, eyePos.y, eyePos.z);
	gView = mat4::lookAt(gEyePos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
}

auto draw() -> void
{
	glViewport(0, 0, gWidth, gHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glBindFramebuffer(GL_FRAMEBUFFER, gGFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(gProgram);
		glUniformMatrix4fv(gViewLoc, 1, false, gView.m);

		glUniformMatrix4fv(gWorldLoc, 1, false, gCubeWorld.m);
		glBindVertexArray(gCubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glUniformMatrix4fv(gWorldLoc, 1, false, gBackpackWorld.m);
		glBindVertexArray(gBackpackVAO);
		glDrawArrays(GL_TRIANGLES, 0, gBackpackVerticesCount);

	glBindFramebuffer(GL_FRAMEBUFFER, gSSAOFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(gSSAOProgram);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindFramebuffer(GL_FRAMEBUFFER, gSSAOBlurFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(gSSAOBlurProgram);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(gDeferredProgram);
		glUniform3f(gEyePosLoc, gEyePos.x, gEyePos.y, gEyePos.z);
		glUniformMatrix4fv(gDeferredViewLoc, 1, false, gView.m);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	/*glBindFramebuffer(GL_READ_FRAMEBUFFER, gGFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, gWidth, gHeight, 0, 0, gWidth, gHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);*/
}

auto main() -> int
{
	return run();
}

std::vector<std::string> split (const std::string& str, char delim)
{
	std::vector<std::string> result;
	std::stringstream ss (str);
	std::string item;

	while (std::getline(ss, item, delim)) {
		result.push_back (item);
	}

	return result;
}