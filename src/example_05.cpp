// Skybox - Cubemaps

#include <fstream>
#include <vector>

#include "macros.h"
#include "entry.h"

#include "vec3.h"
#include "mat4.h"

#include "nanojpeg.h"

const float PI = 3.14159265358979f;

// Cube
float vertices[] = {
	// Pos			        // Color
	-0.5f, -0.5f, -0.5f, 	0.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, +0.5f, 	0.0f, 0.0f, 1.0f,
	-0.5f, +0.5f, -0.5f,    0.0f, 1.0f, 0.0f,
	-0.5f, +0.5f, +0.5f,    0.0f, 1.0f, 1.0f,
	+0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f,
	+0.5f, -0.5f, +0.5f,    1.0f, 0.0f, 1.0f,
	+0.5f, +0.5f, -0.5f,    1.0f, 1.0f, 0.0f,
	+0.5f, +0.5f, +0.5f,    1.0f, 1.0f, 1.0f
};

unsigned int indices[] =
{
	0, 2, 1,
	1, 2, 3,
	4, 5, 6,
	5, 7, 6,
	0, 1, 5,
	0, 5, 4,
	2, 6, 7,
	2, 7, 3,
	0, 4, 6,
	0, 6, 2,
	1, 3, 7,
	1, 7, 5
};

const char* skyboxVertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

out vec3 vTexCoords;
out vec3 vColor;

void main()
{
	vTexCoords = aPos;
	vec4 pos = proj * view * vec4(aPos, 1.0);
	gl_Position = pos.xyww;
}
)";

const char* skyboxFragmentShaderSource = R"(
#version 410 core

uniform samplerCube skybox;

in vec3 vTexCoords;

out vec4 FragColor;

void main()
{
	FragColor = texture(skybox, vTexCoords);
}
)";

const char *vertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

out vec3 vColor;

void main()
{
	vColor = aColor;
	gl_Position = proj * view * world * vec4(aPos, 1.0);
}
)";

const char *fragmentShaderSource = R"(
#version 410 core

in vec3 vColor;

out vec4 FragColor;

void main()
{
	FragColor = vec4(vColor, 1.0);
}
)";

GLuint gSkyboxProgram;

GLuint gProgram;
GLuint gVAO;

GLuint gTexture;

GLint gWorldLoc;
GLuint gViewLoc;
GLuint gSkyboxViewLoc;

float gAngle = 0.0f;

void loadFace(GLenum target, const std::string& faceName)
{
	njInit();
		std::ifstream ifs(faceName, std::ios::binary | std::ios::ate);
		std::streamsize size = ifs.tellg();
		ifs.seekg(0, std::ios::beg);

		std::vector<char> buffer(size);
		if (ifs.read(buffer.data(), size))
		{
			if (njDecode(buffer.data(), size) == NJ_OK)
			{
				glTexImage2D(target, 0, GL_RGB, njGetWidth(), njGetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, njGetImage());
			}
		}

		ifs.close();
    njDone();
}

auto init() -> bool
{
	//std::cout << "init " << gWidth << " " << gHeight << std::endl;
	{
		auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
		GL_CHECK(glShaderSource(vertexShader, 1, &skyboxVertexShaderSource, NULL));
		GL_CHECK(glCompileShader(vertexShader));

		auto fragmentShader = GL_CHECK_RETURN(glCreateShader(GL_FRAGMENT_SHADER));
		GL_CHECK(glShaderSource(fragmentShader, 1, &skyboxFragmentShaderSource, NULL));
		GL_CHECK(glCompileShader(fragmentShader));

		gSkyboxProgram = GL_CHECK_RETURN(glCreateProgram());
		GL_CHECK(glAttachShader(gSkyboxProgram, vertexShader));
		GL_CHECK(glAttachShader(gSkyboxProgram, fragmentShader));
		GL_CHECK(glLinkProgram(gSkyboxProgram));

		GL_CHECK(glDeleteShader(vertexShader));
		GL_CHECK(glDeleteShader(fragmentShader));
	}

	{
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
	}

	glGenVertexArrays(1, &gVAO);
	glBindVertexArray(gVAO);
		GLuint VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);

				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

		GLuint EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glGenTextures(1, &gTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, gTexture);

	loadFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X, "data/right.jpg");
	loadFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "data/left.jpg");
	loadFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "data/top.jpg");
	loadFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "data/bottom.jpg");
	loadFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "data/front.jpg");
	loadFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "data/back.jpg");

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glUseProgram(gProgram);
	gWorldLoc = glGetUniformLocation(gProgram, "world");
	gViewLoc = glGetUniformLocation(gProgram, "view");

	glUseProgram(gSkyboxProgram);		
	gSkyboxViewLoc = glGetUniformLocation(gSkyboxProgram, "view");

	on_size();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

	return 0;
}

void on_size()
{
	//std::cout << "size " << gWidth << " " << gHeight << std::endl;
	
	glUseProgram(gProgram);
	GLint viewLoc = glGetUniformLocation(gProgram, "view");
	GLint projLoc = glGetUniformLocation(gProgram, "proj");

	mat4 world = mat4::identity;
	mat4 proj = mat4::perspective(45.0f * (PI/180.0f), static_cast<float>(gWidth)/gHeight, 0.1f, 100.0f);
	glUniformMatrix4fv(projLoc, 1, false, proj.m);

	glUseProgram(gSkyboxProgram);
	glUniformMatrix4fv(glGetUniformLocation(gSkyboxProgram, "world"), 1, false, world.m);
	glUniformMatrix4fv(glGetUniformLocation(gSkyboxProgram, "proj"), 1, false, proj.m);
}

void on_key(int key, int action)
{

}

auto update() -> void
{
	gAngle++;
}

auto draw() -> void
{
	mat4 world1 = mat4::rotate(0.0f, 1.0f, 0.0f, PI / 4.0f);
	mat4 world2 = mat4::scale(6.0f, 0.1f, 6.0f) * mat4::translate(0.0f, -0.5f, 0.0f);

	vec4 eyePos = mat4::rotate(0.0f, 1.0f, 0.0f, gAngle * (PI/180.0f)) * vec4(0.0f, 3.0, 9.0f, 1.0f);
	mat4 view = mat4::lookAt(vec3(eyePos.x, eyePos.y, eyePos.z), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

	glViewport(0, 0, gWidth, gHeight);
	glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gProgram);
	glBindVertexArray(gVAO);

	glUniformMatrix4fv(gViewLoc, 1, false, view.m);

	glUniformMatrix4fv(gWorldLoc, 1, false, world1.m);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glUniformMatrix4fv(gWorldLoc, 1, false, world2.m);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glDepthFunc(GL_LEQUAL);
	glUseProgram(gSkyboxProgram);
	float m[16] = 
	{
		view.m[0], view.m[1], view.m[2], 0.0f, 
		view.m[4], view.m[5], view.m[6], 0.0f, 
		view.m[8], view.m[9], view.m[10], 0.0f, 
		0.0f, 0.0f, 0.0f, 1.0f
	};

	glUniformMatrix4fv(gSkyboxViewLoc, 1, false, m);
	glBindVertexArray(gVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, gTexture);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glDepthFunc(GL_LESS);
}

auto main() -> int
{
	return run();
}