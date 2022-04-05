// Draw Indirect

#include "macros.h"
#include "entry.h"

typedef struct DrawElementsIndirectCommand
{
	unsigned int count;
	unsigned int instanceCount;
	unsigned int firstIndex;
	int  baseVertex;
	unsigned int baseInstance;
} DrawElementsIndirectCommand;

float triangleleVertices[] = {
	// Pos			// Color
	0.0f,  0.5f, 	1.0f, 0.0f, 0.0f, 1.0f,  // top right
	0.5f, -0.5f, 	0.0f, 1.0f, 0.0f, 1.0f,  // bottom right
	-0.5f, -0.5f, 	0.0f, 0.0f, 1.0f, 1.0f,  // bottom left
};

unsigned int triangleleIndices[] = {  // note that we start from 0!
	0, 1, 2
};

float rectangleVertices[] = {
	// Pos			// Color
	0.5f,  0.5f, 	1.0f, 0.0f, 0.0f, 1.0f,  // top right
	0.5f, -0.5f, 	0.0f, 1.0f, 0.0f, 1.0f,  // bottom right
	-0.5f, -0.5f, 	0.0f, 0.0f, 1.0f, 1.0f,  // bottom left
	-0.5f,  0.5f, 	1.0f, 1.0f, 0.0f, 1.0f,   // top left
};

unsigned int rectangleIndices[] = {  // note that we start from 0!
	0, 1, 3,  // first Triangle
	1, 2, 3   // second Triangle
};

const char *vertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in uint aDrawId;

out vec4 vColor;

void main()
{
	vColor = aColor;

	int drawId = int(aDrawId);
	vec2 offset = vec2(-0.44+(drawId%5)*0.22, -0.44+(drawId/5)*0.22);
	vec2 pos = aPos*0.2;
	gl_Position = vec4(pos.x + offset.x, pos.y + offset.y, 0.0, 1.0);
}
)";

const char *fragmentShaderSource = R"(
#version 410 core

in vec4 vColor;

out vec4 FragColor;

void main()
{
	FragColor = vColor;
}
)";

GLuint gProgram;
GLuint g_triangleleVAO;
GLuint g_rectangleVAO;
GLuint g_triangleleIndirectVAO;
GLuint g_combineIndirectVAO;

auto init() -> bool
{
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

		gProgram = glCreateProgram();
		glAttachShader(gProgram, vertexShader);
		glAttachShader(gProgram, fragmentShader);
		glLinkProgram(gProgram);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	glGenVertexArrays(1, &g_triangleleVAO);
	glBindVertexArray(g_triangleleVAO);
	{
		GLuint VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(triangleleVertices), triangleleVertices, GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);

				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));

		GLuint EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangleleIndices), triangleleIndices, GL_STATIC_DRAW);
	}

	glGenVertexArrays(1, &g_rectangleVAO);
	glBindVertexArray(g_rectangleVAO);
	{
		GLuint VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), rectangleVertices, GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);

				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));

		GLuint EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectangleIndices), rectangleIndices, GL_STATIC_DRAW);
	}

	glGenVertexArrays(1, &g_triangleleIndirectVAO);
	glBindVertexArray(g_triangleleIndirectVAO);
	{
		GLuint VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(triangleleVertices), triangleleVertices, GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);

				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));

		GLuint EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangleleIndices), triangleleIndices, GL_STATIC_DRAW);

		DrawElementsIndirectCommand drawCommands[1];
		drawCommands[0].count = 3;
		drawCommands[0].instanceCount = 25;
		drawCommands[0].firstIndex = 0;
		drawCommands[0].baseVertex = 0;
		drawCommands[0].baseInstance = 0;

		GLuint IBO;
		glGenBuffers(1, &IBO);
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IBO);
			glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(drawCommands), drawCommands, GL_STATIC_DRAW);
	}

	glGenVertexArrays(1, &g_combineIndirectVAO);
	glBindVertexArray(g_combineIndirectVAO);
	{
		float vertices[42];
		for (size_t i = 0; i < 18; ++i)
		{
			vertices[i] = triangleleVertices[i];
		}
		for (size_t i = 0; i < 24; ++i)
		{
			vertices[18 + i] = rectangleVertices[i];
		}

		GLuint VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);

				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));

		unsigned int indices[9];
		for (size_t i = 0; i < 3; ++i)
		{
			indices[i] = triangleleIndices[i];
		}
		for (size_t i = 0; i < 6; ++i)
		{
			indices[3 + i] = rectangleIndices[i];
		}

		GLuint EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		DrawElementsIndirectCommand drawCommands[20];
		for (size_t i = 0; i < 10; ++i)
		{
			drawCommands[2*i + 0].count = 3;
			drawCommands[2*i + 0].instanceCount = 1;
			drawCommands[2*i + 0].firstIndex = 0;
			drawCommands[2*i + 0].baseVertex = 0;
			drawCommands[2*i + 0].baseInstance = 2*i + 0;

			drawCommands[2*i + 1].count = 6;
			drawCommands[2*i + 1].instanceCount = 1;
			drawCommands[2*i + 1].firstIndex = 3;
			drawCommands[2*i + 1].baseVertex = 3;
			drawCommands[2*i + 1].baseInstance = 2*i + 1;
		}

		GLuint IBO;
		glGenBuffers(1, &IBO);
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IBO);
				glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(drawCommands), drawCommands, GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, IBO);
				glEnableVertexAttribArray(2);
					glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(DrawElementsIndirectCommand), (void*)(offsetof(DrawElementsIndirectCommand, baseInstance)));
				// https://learnopengl.com/Advanced-OpenGL/Instancing
				glVertexAttribDivisor(2, 1); // By setting this attribute to 1 we're telling OpenGL that we want to update the content of the vertex attribute when we start to render a new instance
	}

	on_size();

	return true;
}

auto on_size() -> void
{
	//std::cout << "size " << gWidth << " " << gHeight << std::endl;
	glViewport(0, 0, gWidth, gHeight);
}

auto on_key(int key, int action) -> void
{
}

auto on_mouse(double xpos, double ypos) -> void
{
}

auto update() -> void
{

}

auto draw() -> void
{
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(gProgram);

	/*
	glBindVertexArray(g_rectangleVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindVertexArray(g_triangleleVAO);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
	*/

	/*
	glBindVertexArray(g_rectangleVAO);
	glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, 1, 0, 10);
	*/

	/*
	glBindVertexArray(g_triangleleIndirectVAO);
	glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void*)0);
	*/

	glBindVertexArray(g_combineIndirectVAO);
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void*)0, 20, 0);
}

auto main() -> int
{
	return run();
}
