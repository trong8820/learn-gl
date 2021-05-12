// Shadow mapping

#include "macros.h"
#include "entry.h"

#include "vec3.h"
#include "mat4.h"

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
	gl_Position = proj * view * world * vec4(aPos.xyz, 1.0);
}
)";

const char *fragmentShaderSource = R"(
#version 410 core

in vec3 vColor;

out vec4 FragColor;

void main()
{
	FragColor = vec4(vColor.rgb, 1.0);
}
)";

GLuint gProgram;
GLuint gVAO;

GLint gWorldLoc;
float gAngle = 0.0f;

auto init() -> bool
{
	//std::cout << "init " << gWidth << " " << gHeight << std::endl;
    auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
	GL_CHECK(glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr));
	GL_CHECK(glCompileShader(vertexShader));

	auto fragmentShader = GL_CHECK_RETURN(glCreateShader(GL_FRAGMENT_SHADER));
	GL_CHECK(glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr));
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
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);

				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

        GLuint EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	size();

    return 0;
}

void size()
{
	//std::cout << "size " << gWidth << " " << gHeight << std::endl;
	glViewport(0, 0, gWidth, gHeight);

	glUseProgram(gProgram);
	gWorldLoc = glGetUniformLocation(gProgram, "world");
	GLint viewLoc = glGetUniformLocation(gProgram, "view");
	GLint projLoc = glGetUniformLocation(gProgram, "proj");

	mat4 world = mat4::identity;
	mat4 view = mat4::lookAt(vec3(0.0f, 3.0, 3.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 proj = mat4::perspective(45.0f * (PI/180.0f), static_cast<float>(gWidth)/gHeight, 0.1f, 100.0f);
	glUniformMatrix4fv(gWorldLoc, 1, false, world.m);
	glUniformMatrix4fv(viewLoc, 1, false, view.m);
	glUniformMatrix4fv(projLoc, 1, false, proj.m);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
}

auto update() -> void
{
	gAngle++;
}

auto draw() -> void
{
    glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gProgram);
	glBindVertexArray(gVAO);

	mat4 world1 = mat4::rotate(0.0f, 1.0f, 0.0f, gAngle * (PI/180.0f));
	glUniformMatrix4fv(gWorldLoc, 1, false, world1.m);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	mat4 world2 = mat4::translate(0.0f, -1.0f, 0.0f) * mat4::scale(10.0f, 0.1f, 10.0f);
	glUniformMatrix4fv(gWorldLoc, 1, false, world2.m);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

auto main() -> int
{
	return run();
}