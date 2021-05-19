// Terrain Rendering

#include "macros.h"
#include "entry.h"

#include "vec3.h"
#include "vec4.h"
#include "mat4.h"

const float PI = 3.14159265358979f;

const char *vertexShaderSource = R"(
#version 410 core

struct PerInstanceData
{
    vec2 offset;
    float scale;
};

layout (location = 0) in vec2 aPos;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

uniform InstanceData
{
    PerInstanceData instance[256];
};

void main()
{
    vec2 pos = aPos*instance[gl_InstanceID].scale + instance[gl_InstanceID].offset;
	gl_Position = proj * view * world * vec4(pos.x, 0.0, -pos.y, 1.0);
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

const int CLIPMAP_SIZE_W = 64;
const int CLIPMAP_SIZE_H = 64;
const int CLIPMAP_LEVELS = 10;
const float CLIPMAP_SCALE = 0.25f;

GLuint gProgram;
GLuint gVAO;

GLuint gUBO;

GLint gWorldLoc;
GLint gViewLoc;
GLint gProjLoc;

vec3 gEyePos;
vec3 gEyeDir;
float gEyeAngle;

// Rot
bool keyQ {};
bool keyE {};

// Move
bool keyA {};
bool keyD {};
bool keyS {};
bool keyW {};

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

    GLfloat* vertices = new GLfloat[CLIPMAP_SIZE_W*CLIPMAP_SIZE_H * 2];
    for (size_t i = 0; i < CLIPMAP_SIZE_H; i++)
    {
        for (size_t j = 0; j < CLIPMAP_SIZE_W; j++)
        {
            size_t id = i*CLIPMAP_SIZE_W + j;
            vertices[id*2 + 0] = static_cast<float>(j) - CLIPMAP_SIZE_W / 2.0f;
            vertices[id*2 + 1] = static_cast<float>(i) - CLIPMAP_SIZE_H / 2.0f;
        }
    }
    
    // 0 1 2 2 3 3 4 5 -> {0 1 2} {3 4 5}
    size_t indicesSize = (CLIPMAP_SIZE_W*2 + 2)*(CLIPMAP_SIZE_H - 1);
    GLuint* indices = new GLuint[indicesSize];
    size_t id = 0;
    for (size_t i = 0; i < CLIPMAP_SIZE_H - 1; i++)
    {
        indices[id] = i*CLIPMAP_SIZE_W;
        id++;
        for (size_t j = 0; j < CLIPMAP_SIZE_W; j++)
        {
            indices[id] = i*CLIPMAP_SIZE_W + j;
            id++;
            indices[id] = (i + 1)*CLIPMAP_SIZE_W + j;
            id++;
        }
        indices[id] = (i + 1)*CLIPMAP_SIZE_W + CLIPMAP_SIZE_W - 1;
        id++;
    }

	glGenVertexArrays(1, &gVAO);
	glBindVertexArray(gVAO);
		GLuint VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, (CLIPMAP_SIZE_W*CLIPMAP_SIZE_H * 2) * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);

		GLuint EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize * sizeof(GLuint), indices, GL_STATIC_DRAW);

    delete [] indices;
    delete [] vertices;

    glGenBuffers(1, &gUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, gUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(float)*4 * 3, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUseProgram(gProgram);
    glUniformBlockBinding(gProgram, glGetUniformBlockIndex(gProgram, "InstanceData"), 0);
	gWorldLoc = glGetUniformLocation(gProgram, "world");
	gViewLoc = glGetUniformLocation(gProgram, "view");
	gProjLoc = glGetUniformLocation(gProgram, "proj");

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, gUBO); 

    //glEnable(GL_PROGRAM_POINT_SIZE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

    gEyePos = vec3(0.0f, 30.0, 60.0f);
    gEyeDir = -gEyePos;

	size();
    
	return 0;
}

void size()
{
	//std::cout << "size " << gWidth << " " << gHeight << std::endl;
	glViewport(0, 0, gWidth, gHeight);

	glUseProgram(gProgram);

	mat4 world = mat4::identity;
	mat4 view = mat4::lookAt(gEyePos, gEyePos + gEyeDir, vec3(0.0f, 1.0f, 0.0f));
	mat4 proj = mat4::perspective(45.0f * (PI/180.0f), static_cast<float>(gWidth)/gHeight, 0.1f, 1000.0f);
    glUniformMatrix4fv(gWorldLoc, 1, false, world.m);
	glUniformMatrix4fv(gViewLoc, 1, false, view.m);
	glUniformMatrix4fv(gProjLoc, 1, false, proj.m);
}

bool gEyeInvalidate {};
auto update() -> void
{
    gEyeInvalidate = false;
    if (keyQ)
    {
        vec4 eyeDir = mat4::rotate(0.0f, 1.0f, 0.0f, 0.01f) * vec4(gEyeDir.x, gEyeDir.y, gEyeDir.z, 1.0f);
        gEyeDir.x = eyeDir.x;
        gEyeDir.y = eyeDir.y;
        gEyeDir.z = eyeDir.z;
        gEyeInvalidate = true;
    }
    if (keyE)
    {
        vec4 eyeDir = mat4::rotate(0.0f, 1.0f, 0.0f, -0.01f) * vec4(gEyeDir.x, gEyeDir.y, gEyeDir.z, 1.0f);
        gEyeDir.x = eyeDir.x;
        gEyeDir.y = eyeDir.y;
        gEyeDir.z = eyeDir.z;
        gEyeInvalidate = true;
    }

    if (keyA)
    {
        vec3 dirNor = gEyeDir.normalize();
        vec3 d = vec3::cross(dirNor, vec3(0.0f, 1.0f, 0.0f));
        gEyePos.x -= d.x*1.0f;
        gEyePos.z -= d.z*1.0f;
        gEyeInvalidate = true;
    }
    if (keyD)
    {
        vec3 dirNor = gEyeDir.normalize();
        vec3 d = vec3::cross(dirNor, vec3(0.0f, 1.0f, 0.0f));
        gEyePos.x += d.x*1.0f;
        gEyePos.z += d.z*1.0f;
        gEyeInvalidate = true;
    }
    if (keyS)
    {
        vec3 dirNor = gEyeDir.normalize();
        gEyePos.x -= dirNor.x*1.0f;
        gEyePos.z -= dirNor.z*1.0f;
        gEyeInvalidate = true;
    }
    if (keyW)
    {
        vec3 dirNor = gEyeDir.normalize();
        gEyePos.x += dirNor.x*1.0f;
        gEyePos.z += dirNor.z*1.0f;
        gEyeInvalidate = true;
    }
    if (gEyeInvalidate) 
    {
        mat4 view = mat4::lookAt(gEyePos, gEyePos + gEyeDir, vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(gViewLoc, 1, false, view.m);
    }
}

auto draw() -> void
{
	glViewport(0, 0, gWidth, gHeight);
	glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gProgram);
	glBindVertexArray(gVAO);

    glBindBuffer(GL_UNIFORM_BUFFER, gUBO);
    //float *data = static_cast<float*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(float)*4 * 3, GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_WRITE_BIT));
    float *data = static_cast<float*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY));
    data[0] = 1.0f;
    data[1] = 2.0f;
    data[2] = 3.0f;
    data[3] = 4.0f;
    //data[3] = 80.0f;
    //data[4] = 80.0f;
    //data[5] = 80.0f;
    //data[6] = 80.0f;

    //data[3] = 2.0f;
    //data[4] = 1.0f;
    //data[5] = 1.0f;
    glUnmapBuffer(GL_UNIFORM_BUFFER);

	//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    //glDrawElements(GL_TRIANGLE_STRIP, (CLIPMAP_SIZE_W*2 + 2)*(CLIPMAP_SIZE_H - 1), GL_UNSIGNED_INT,  0);
    //glDrawArrays(GL_POINTS, 0, CLIPMAP_SIZE*CLIPMAP_SIZE * 2);
    glDrawElementsInstanced(GL_TRIANGLE_STRIP, (CLIPMAP_SIZE_W*2 + 2)*(CLIPMAP_SIZE_H - 1), GL_UNSIGNED_INT,  0, 2);
}

void on_key(int key, int action)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_Q) keyQ = true;
        if (key == GLFW_KEY_E) keyE = true;

        if (key == GLFW_KEY_A) keyA = true;
        if (key == GLFW_KEY_D) keyD = true;
        if (key == GLFW_KEY_S) keyS = true;
        if (key == GLFW_KEY_W) keyW = true;
    }

    if (action == GLFW_RELEASE)
    {
        if (key == GLFW_KEY_Q) keyQ = false;
        if (key == GLFW_KEY_E) keyE = false;

        if (key == GLFW_KEY_A) keyA = false;
        if (key == GLFW_KEY_D) keyD = false;
        if (key == GLFW_KEY_S) keyS = false;
        if (key == GLFW_KEY_W) keyW = false;
    }
}

auto main() -> int
{
	return run();
}