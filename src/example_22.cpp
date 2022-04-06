// Compute shader. OpenGL >= 4.3

#include "macros.h"
#include "entry.h"

#include "vec3.h"
#include "mat4.h"

const float PI = 3.14159265358979f;

float triangleleVertices[] = {
	// Pos
	0.0f,  0.5f,
	0.5f, -0.5f,
	-0.5f, -0.5f
};

unsigned int triangleleIndices[] = {  // note that we start from 0!
	0, 1, 2
};

float ssboData[256*2]; //vec2

const char *computeShaderSource = R"(
#version 430 core

layout(std430, binding = 0) buffer SSBO
{
	vec2 data[];
};

layout (local_size_x = 128) in;

void main()
{
	uint ident = gl_GlobalInvocationID.x;
	data[ident] = vec2(1.1*int(ident%5), 1.1*int(ident/5));
}
)";

const char *vertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aOffset;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	gl_Position = proj * view * world * vec4(aPos.x + aOffset.x, aPos.y + aOffset.y, 0.0, 1.0);
}
)";

const char *fragmentShaderSource = R"(
#version 410 core

out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";

GLuint g_computeProgram;

GLuint g_SSBO;

GLuint g_program;
GLint g_worldLoc;
GLint g_viewLoc;

GLuint g_triangleleVAO;

mat4 g_view;

double g_prevPosX;
double g_prevPosY;
float g_targetRotX;
float g_targetRotY;
float g_rotX;
float g_rotY;

// Move
bool keyS {};
bool keyW {};
float g_eyePosZ = 8.0f;

auto init() -> bool
{
	std::memset(ssboData, 0, sizeof(ssboData));

	{
		auto computeShader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(computeShader, 1, &computeShaderSource, nullptr);
		GL_CHECK(glCompileShader(computeShader))
		{
			GLint success;
			GL_CHECK(glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success));
			if (!success) {
				GLint infoLength = 0;
				glGetShaderiv(computeShader, GL_INFO_LOG_LENGTH, &infoLength);
				char* infoLog = new char[infoLength];
				GL_CHECK(glGetShaderInfoLog(computeShader, infoLength, &infoLength, infoLog));
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "COMPUTE_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				delete[] infoLog;
				GL_CHECK(glDeleteShader(computeShader));
			}

			g_computeProgram = glCreateProgram();
			glAttachShader(g_computeProgram, computeShader);
			glLinkProgram(g_computeProgram);

			GL_CHECK(glDeleteShader(computeShader));
		}
	}

	{
		auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
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

		auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
		glCompileShader(fragmentShader);

		g_program = glCreateProgram();
		glAttachShader(g_program, vertexShader);
		glAttachShader(g_program, fragmentShader);
		glLinkProgram(g_program);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	glGenBuffers(1, &g_SSBO);
	glGenVertexArrays(1, &g_triangleleVAO);
	glBindVertexArray(g_triangleleVAO);
	{
		GLuint VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(triangleleVertices), triangleleVertices, GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
		glBindBuffer(GL_ARRAY_BUFFER, g_SSBO);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
				glVertexAttribDivisor(1, 1);

		GLuint EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangleleIndices), triangleleIndices, GL_STATIC_DRAW);
	}

	glUseProgram(g_program);
	g_worldLoc = glGetUniformLocation(g_program, "world");
	g_viewLoc = glGetUniformLocation(g_program, "view");

	GLint maxComputeWorkGroupInvocations;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxComputeWorkGroupInvocations);
	std::cout << "GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS: " << maxComputeWorkGroupInvocations << std::endl;

	//glGenBuffers(1, &g_SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_SSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ssboData), ssboData, GL_STATIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_SSBO);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	on_size();

	return true;
}

void on_size()
{
	//std::cout << "size " << gWidth << " " << gHeight << std::endl;
	glViewport(0, 0, gWidth, gHeight);

	glUseProgram(g_program);
	GLint projLoc = glGetUniformLocation(g_program, "proj");

	mat4 proj = mat4::perspective(45.0f * (PI/180.0f), static_cast<float>(gWidth)/gHeight, 0.1f, 100.0f);
	glUniformMatrix4fv(projLoc, 1, false, proj.m);
}

void on_key(int key, int action)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_S) keyS = true;
		if (key == GLFW_KEY_W) keyW = true;
	}

	if (action == GLFW_RELEASE)
	{
		if (key == GLFW_KEY_S) keyS = false;
		if (key == GLFW_KEY_W) keyW = false;
	}
}

void on_mouse(double xpos, double ypos)
{
	int state = glfwGetMouseButton(g_pWindow, GLFW_MOUSE_BUTTON_LEFT);
	if (state == GLFW_PRESS)
	{
		g_targetRotX += (xpos - g_prevPosX)*0.01f;
		g_targetRotY += (ypos - g_prevPosY)*0.01f;

		if (g_targetRotY <= -PI / 2.0f) g_targetRotY = -PI / 2.0f + 0.01f;
		if (g_targetRotY >= PI / 2.0f)  g_targetRotY = PI / 2.0f - 0.01f;
	}
	g_prevPosX = xpos;
	g_prevPosY = ypos;
}

void update()
{
	if(keyW)
	{
		g_eyePosZ -= 0.02f;
	}
	if (keyS)
	{
		g_eyePosZ += 0.02f;
	}

	g_rotX += 0.05 * (g_targetRotX - g_rotX);
	g_rotY += 0.05 * (g_targetRotY - g_rotY);

	vec4 eyePos = mat4::rotate(0.0f, 1.0f, 0.0f, -g_rotX) * mat4::rotate(1.0f, 0.0f, 0.0f, -g_rotY) * vec4(0.0f, 0.0f, g_eyePosZ, 0.0f);
	g_view = mat4::lookAt(vec3(eyePos.x, eyePos.y, eyePos.z), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
}

void draw()
{
	glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(g_computeProgram);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_SSBO);
	glDispatchCompute(2, 1, 1);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUseProgram(g_program);
	glUniformMatrix4fv(g_viewLoc, 1, false, g_view.m);

	glBindVertexArray(g_triangleleVAO);
	mat4 world = mat4::identity;
	glUniformMatrix4fv(g_worldLoc, 1, false, world.m);
	//glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
	glDrawElementsInstanced(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)0, 25);
}

int main()
{
	return run();
}
