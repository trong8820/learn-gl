#include "entry.h"

const char *vertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec2 aPos;

void main()
{
	gl_Position = vec4(aPos.x, aPos.y, 1.0, 1.0);
}
)";

const char *fragmentShaderSource = R"(
#version 410 core

out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
)";

GLuint gProgram;
GLuint gVAO[3];
GLuint gVBO[3];

float gVertices[200];

GLsync gFences[3];
int frameNum = 0;

// https://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf

auto init() -> bool
{
	auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
	glCompileShader(vertexShader);

	auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
	glCompileShader(fragmentShader);

	gProgram = glCreateProgram();
	glAttachShader(gProgram, vertexShader);
	glAttachShader(gProgram, fragmentShader);
	glLinkProgram(gProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glGenVertexArrays(3, gVAO);
	glGenBuffers(3, gVBO);
	for(size_t i=0; i<3; i++)
	{
		glBindVertexArray(gVAO[i]);
			glBindBuffer(GL_ARRAY_BUFFER, gVBO[i]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(gVertices), NULL, GL_DYNAMIC_DRAW);
				glEnableVertexAttribArray(0);
					glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	for (size_t i = 0; i < 3; i++)
	{
		gFences[i] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}

	return true;
}

float scale = 1.0f;

auto update() -> void
{
	scale += 0.01f;
	if (scale > 20.0f) scale = 1.0f;

	for (size_t i = 0; i < 100; i++)
	{
		float x = (static_cast<int>(i) - 50)*0.01f;
		gVertices[2*i + 0] = x;
		gVertices[2*i + 1] = sin(x*scale);
	}
}

auto draw() -> void
{
	int bufferNum = frameNum++ % 3;

	glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(gProgram);

	GLenum result = glClientWaitSync(gFences[bufferNum], 0, GL_TIMEOUT_IGNORED);
	if (result == GL_TIMEOUT_EXPIRED || result == GL_WAIT_FAILED)
	{
		std::cout << "Client wait sync failed!" << std::endl;
		glDeleteSync(gFences[bufferNum]);
		gFences[bufferNum] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		return;
	}
	glDeleteSync(gFences[bufferNum]);

	glBindVertexArray(gVAO[bufferNum]);
		glBindBuffer(GL_ARRAY_BUFFER, gVBO[bufferNum]);
			void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0*sizeof(float), 200*sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
			std::memcpy(ptr, gVertices, 200*sizeof(float));
			glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDrawArrays(GL_LINE_STRIP, 0, 100);

	gFences[bufferNum] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

auto main() -> int
{
	return run();
}
