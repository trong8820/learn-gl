// Deferred Shading

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

const char* vertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

out vec3 vFragPos;
out vec2 vUV;
out vec3 vNormal;
out mat3 vTBN;

void main()
{
	vFragPos = vec3(world * vec4(aPos, 1.0));
	vUV = aUV;

	mat3 normalMatrix = transpose(inverse(mat3(world)));
    vNormal = normalize(normalMatrix * aNormal);
    vec3 T = normalize(normalMatrix * aTangent);
	T = normalize(T - dot(T, vNormal) * vNormal);
    vec3 B = cross(vNormal, T);
    vTBN = mat3(T, B, vNormal);

	gl_Position = proj * view * world * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 410 core

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

in vec3 vFragPos;
in vec2 vUV;
in vec3 vNormal;
in mat3 vTBN;

layout (location = 0) out vec3 fPosition;
layout (location = 1) out vec3 fNormal;
layout (location = 2) out vec4 fAlbedo;

out vec4 FragColor;

void main()
{   
    vec3 color = texture(diffuseMap, vUV).rgb;
	vec3 normal = texture(normalMap, vUV).rgb;
	normal = vTBN * normalize(normal * 2.0 - 1.0);

    fPosition = vFragPos;
    fNormal = vNormal + normal;
    //fAlbedo = vec4(color, 1.0);
    fAlbedo = vec4(1.0, 1.0, 1.0, 1.0);
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

const vec3 objectColor = vec3(0.5, 0.5, 0.5);

uniform sampler2D positionMap;
uniform sampler2D normalMap;
uniform sampler2D albedoMap;

uniform vec3 lightPositions[256];
uniform vec3 lightColors[256];

uniform vec3 eyePos;

in vec2 vUV;

out vec4 FragColor;

void main()
{   
    vec3 fragPos = texture(positionMap, vUV).rgb;
    vec3 normal = texture(normalMap, vUV).rgb;
    vec3 color = texture(albedoMap, vUV).rgb * objectColor;

    normal = normalize(normal);
    vec3 viewDir  = normalize(eyePos - fragPos);

    vec3 result = 0.2 * color;

    // ambient
	float ambientStrength = 0.2;
	vec3 ambient = ambientStrength * color;
    result += ambient;
    
    float radius = 10.0;
    for (int i=0; i<64; i++)
    {
        vec3 lightPos = lightPositions[i];
        float distance = length(lightPos - fragPos);
        if (distance < radius)
        {
            vec3 lightColor = lightColors[i];

            // diffuse
            vec3 lightDir = normalize(lightPos - fragPos);
            vec3 diffuse = max(dot(normal, lightDir), 0.0) * color * lightColor;

            // specular - Blinn-Phong
            float specularStrength = 0.8;
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
            vec3 specular = specularStrength * spec * lightColor;

            // attenuation
            float attenuation = (distance - radius)*(distance - radius)/(radius*radius);
            result += (diffuse + specular) * attenuation * 1.6;
        }
    }

	FragColor = vec4(result, 1.0);
}
)";

GLuint gProgram;
GLuint gVAO;
GLint gWorldLoc;
GLint gViewLoc;
GLint gEyePosLoc;

GLsizei gIndexCount;

GLuint gDeferredProgram;

double gPrevPosX;
double gPrevPosY;
float gTargetRotX;
float gTargetRotY;
float gRotX;
float gRotY;

vec3 gEyePos;
mat4 gView;

std::vector<mat4> gWorls;

GLuint gFrameBuffer;
GLuint gPositionTexture;
GLuint gNormalTexture;
GLuint gAlbedoTexture;

GLuint gDepthRenderBuffer;

std::tuple<std::vector<float>, std::vector<unsigned int>> sphere(unsigned int segments);
void loadTexture(const std::string& path);

auto init() -> bool
{
	auto sphereData = sphere(16);
	std::vector<float> vertices = std::get<0>(sphereData);
	std::vector<unsigned int> indices = std::get<1>(sphereData);
	gIndexCount = indices.size();

    std::random_device rd;
    std::default_random_engine re(rd());
    std::uniform_real_distribution<float> ball_pos_real_dist(-10.0f, 10.0f);
    std::uniform_real_distribution<float> light_pos_real_dist(-16.0f, 16.0f);
    std::uniform_real_distribution<float> color_real_dist(0.5f, 1.0f);

    for (size_t i = 0; i < 100; i++)
    {
        gWorls.push_back(mat4::translate(ball_pos_real_dist(re), ball_pos_real_dist(re), ball_pos_real_dist(re)));
    }
    
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
	loadTexture("data/bricks2.jpg");
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
    
    // g-buffer
    glGenFramebuffers(1, &gFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gFrameBuffer);

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

    GLenum attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    glGenRenderbuffers(1, &gDepthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, gDepthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, gWidth, gHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepthRenderBuffer);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {   
        std::cout << "Framebuffer not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glUseProgram(gProgram);
    gWorldLoc = glGetUniformLocation(gProgram, "world");
    gViewLoc = glGetUniformLocation(gProgram, "view");
	glUniform1i(glGetUniformLocation(gProgram, "diffuseMap"), 0);
	glUniform1i(glGetUniformLocation(gProgram, "normalMap"), 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalTexture);

    glUseProgram(gDeferredProgram);
    gEyePosLoc = glGetUniformLocation(gDeferredProgram, "eyePos");
    glUniform1i(glGetUniformLocation(gDeferredProgram, "positionMap"), 2);
	glUniform1i(glGetUniformLocation(gDeferredProgram, "normalMap"), 3);
    glUniform1i(glGetUniformLocation(gDeferredProgram, "albedoMap"), 4);
    glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gPositionTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gNormalTexture);
    glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gAlbedoTexture);

    for (size_t i = 0; i < 100; i++)
    {
        glUniform3f(glGetUniformLocation(gDeferredProgram, ("lightPositions[" +std::to_string(i) + "]").c_str()), light_pos_real_dist(re), light_pos_real_dist(re), light_pos_real_dist(re));
        glUniform3f(glGetUniformLocation(gDeferredProgram, ("lightColors[" +std::to_string(i) + "]").c_str()), color_real_dist(re), color_real_dist(re), color_real_dist(re));
    }

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
	//GLint worldLoc = glGetUniformLocation(gProgram, "world");
	//GLint viewLoc = glGetUniformLocation(gProgram, "view");
	GLint projLoc = glGetUniformLocation(gProgram, "proj");

	//mat4 world = mat4::identity;
	mat4 view = mat4::lookAt(vec3(0.0f, 3.0f, 3.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 proj = mat4::perspective(45.0f * (PI / 180.0f), static_cast<float>(gWidth) / gHeight, 0.1f, 100.0f);
	//glUniformMatrix4fv(worldLoc, 1, false, world.m);
	//glUniformMatrix4fv(viewLoc, 1, false, view.m);
	glUniformMatrix4fv(projLoc, 1, false, proj.m);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gPositionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gWidth, gHeight, 0, GL_RGBA, GL_FLOAT, NULL);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gNormalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gWidth, gHeight, 0, GL_RGBA, GL_FLOAT, NULL);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, gAlbedoTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gWidth, gHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glBindRenderbuffer(GL_RENDERBUFFER, gDepthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, gWidth, gHeight);
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
    glBindFramebuffer(GL_FRAMEBUFFER, gFrameBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(gProgram);
        glUniformMatrix4fv(gViewLoc, 1, false, gView.m);
        glBindVertexArray(gVAO);

        for (size_t i = 0; i < gWorls.size(); i++)
        {
            glUniformMatrix4fv(gWorldLoc, 1, false, gWorls[i].m);
            glDrawElements(GL_TRIANGLES, gIndexCount, GL_UNSIGNED_INT, 0);
        }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gDeferredProgram);
    glUniform3f(gEyePosLoc, gEyePos.x, gEyePos.y, gEyePos.z);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, gFrameBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, gWidth, gHeight, 0, 0, gWidth, gHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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