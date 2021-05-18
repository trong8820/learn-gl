// Terrain Rendering

#include "macros.h"
#include "entry.h"

#include "vec3.h"
#include "mat4.h"

const float PI = 3.14159265358979f;

const char *vertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec2 aPos;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	gl_Position = proj * view * world * vec4(aPos, 0.0, 1.0);
    gl_PointSize = 4.0;
}
)";

const char *fragmentShaderSource = R"(
#version 410 core

out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)";

//const int CLIPMAP_SIZE = 64;
const int CLIPMAP_SIZE = 4;
const int CLIPMAP_LEVELS = 10;
const float CLIPMAP_SCALE = 0.25f;

GLuint gProgram;
GLuint gVAO;

auto init() -> bool
{
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

    GLfloat* vertices = new GLfloat[CLIPMAP_SIZE*CLIPMAP_SIZE * 2];
    for (size_t i = 0; i < CLIPMAP_SIZE; i++)
    {
        for (size_t j = 0; j < CLIPMAP_SIZE; j++)
        {
            size_t id = i*CLIPMAP_SIZE + j;
            vertices[id*2 + 0] = static_cast<float>(j) - CLIPMAP_SIZE / 2.0f;
            vertices[id*2 + 1] = static_cast<float>(i) - CLIPMAP_SIZE / 2.0f;
        }
    }
    
    size_t indicesSize = ((CLIPMAP_SIZE + 2)*CLIPMAP_SIZE)*2;
    GLuint* indices = new GLuint[indicesSize];
    size_t id = 0;
    for (size_t i = 0; i < CLIPMAP_SIZE; i++)
    {
        indices[id] = i*CLIPMAP_SIZE;
        id++;
        for (size_t j = 0; j < CLIPMAP_SIZE; j++)
        {
            indices[id] = i*CLIPMAP_SIZE + j;
            id++;
            indices[id] = (i + 1)*CLIPMAP_SIZE + j;
            id++;
        }
        indices[id] = (i + 1)*CLIPMAP_SIZE;
        id++;
    }

	glGenVertexArrays(1, &gVAO);
	glBindVertexArray(gVAO);
		GLuint VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, (CLIPMAP_SIZE*CLIPMAP_SIZE * 2) * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);

		GLuint EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize * sizeof(GLuint), indices, GL_STATIC_DRAW);

    delete [] indices;
    delete [] vertices;

	size();

    glEnable(GL_PROGRAM_POINT_SIZE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

    int vSize = 3;
    for (size_t z = 0; z < vSize - 1; z++)
    {
        std::cout << z * vSize << std::endl;
        for (size_t x = 0; x < vSize; x++)
        {
            std::cout << z * vSize + x << std::endl;
            std::cout << (z + 1) * vSize + x << std::endl;
        }
        std::cout << (z + 1) * vSize + (vSize - 1) << std::endl;
    }
    

	return 0;
}

void size()
{
	//std::cout << "size " << gWidth << " " << gHeight << std::endl;
	glViewport(0, 0, gWidth, gHeight);

	glUseProgram(gProgram);
	GLint worldLoc = glGetUniformLocation(gProgram, "world");
	GLint viewLoc = glGetUniformLocation(gProgram, "view");
	GLint projLoc = glGetUniformLocation(gProgram, "proj");

	mat4 world = mat4::identity;
	mat4 view = mat4::lookAt(vec3(0.0f, 10.0, 10.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 proj = mat4::perspective(45.0f * (PI/180.0f), static_cast<float>(gWidth)/gHeight, 0.1f, 1000.0f);
    glUniformMatrix4fv(worldLoc, 1, false, world.m);
	glUniformMatrix4fv(viewLoc, 1, false, view.m);
	glUniformMatrix4fv(projLoc, 1, false, proj.m);
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

	//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glDrawElements(GL_TRIANGLE_STRIP, CLIPMAP_SIZE*CLIPMAP_SIZE*2, GL_UNSIGNED_INT,  0);
    //glDrawArrays(GL_POINTS, 0, CLIPMAP_SIZE*CLIPMAP_SIZE * 2);
}

auto main() -> int
{
	return run();
}