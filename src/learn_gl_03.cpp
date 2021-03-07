#include "entry.h"

float vertices[] = {
	// Pos			// Color
	0.5f,  0.5f, 	1.0f, 0.0f, 0.0f, 1.0f,  // top right
	0.5f, -0.5f, 	0.0f, 1.0f, 0.0f, 1.0f,  // bottom right
	-0.5f, -0.5f, 	0.0f, 0.0f, 1.0f, 1.0f,  // bottom left
	-0.5f,  0.5f, 	1.0f, 1.0f, 0.0f, 1.0f,   // top left
};

float uvs[] = {
	1.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,
	0.0f, 0.0f,
};

unsigned int indices[] = {  // note that we start from 0!
	0, 1, 3,  // first Triangle
	1, 2, 3   // second Triangle
};

unsigned char pixels[] = {
	255, 0, 0, 255,      255, 255, 0, 255,
	0, 255, 0, 255,      0, 255, 255, 255,
};

const char *vertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec4 vColor;
out vec2 vTexCoord;

void main()
{
	vColor = aColor;
	vTexCoord = aTexCoord;
	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}
)";

const char *fragmentShaderSource = R"(
#version 410 core

uniform sampler2D texture0;

in vec4 vColor;
in vec2 vTexCoord;

out vec4 FragColor;

void main()
{
	FragColor = vColor * texture(texture0, vTexCoord);
}
)";

GLuint gProgram;
GLuint gVAO;
GLuint gTexture;

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

	glGenVertexArrays(1, &gVAO);
	std::cout << "VAO: " << gVAO << std::endl;
	glBindVertexArray(gVAO);
		GLuint VBO1;
		glGenBuffers(1, &VBO1);
		std::cout << "VBO1: " << VBO1 << std::endl;
		glBindBuffer(GL_ARRAY_BUFFER, VBO1);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);

				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));

		GLuint VBO2;
		glGenBuffers(1, &VBO2);
		std::cout << "VBO2: " << VBO2 << std::endl;
		glBindBuffer(GL_ARRAY_BUFFER, VBO2);
			glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);

		GLuint EBO;
		glGenBuffers(1, &EBO);
		std::cout << "EBO: " << EBO << std::endl;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Get vbo
	glBindVertexArray(gVAO);
	GLint maxVertexAttribs;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
	std::cout << "Max vertex Attribs: " << maxVertexAttribs << std::endl;
	for (GLuint i = 0; i < maxVertexAttribs; i++)
	{
		GLuint vertexAttribEnabled;
		glGetVertexAttribIuiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &vertexAttribEnabled);
		if (vertexAttribEnabled > 0)
		{
			std::cout << "Vertex attrib enabled: " << i << std::endl;

			GLuint vbo;
			glGetVertexAttribIuiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vbo);
			std::cout << "Retreve VBO: " << vbo << std::endl;
		}
	}

	int ebo;
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &ebo);
	std::cout << "Retreve EBO: " << ebo << std::endl;

	// Texture check
	// GL 4.3 or ARB_internalformat_query2
	// glad_glGetInternalformativ = (PFNGLGETINTERNALFORMATIVPROC)glfwGetProcAddress("glGetInternalformativ");

	/*GLint ptformat, tformat, ttype;
	glGetInternalformativ(GL_TEXTURE_2D, GL_RGBA8, GL_INTERNALFORMAT_PREFERRED, 1, &ptformat);
	glGetInternalformativ(GL_TEXTURE_2D, GL_RGBA8, GL_TEXTURE_IMAGE_FORMAT, 1, &tformat);
	glGetInternalformativ(GL_TEXTURE_2D, GL_RGBA8, GL_TEXTURE_IMAGE_TYPE, 1, &ttype);
	std::cout << "Texture: " << (GLuint)ptformat << " " << (GLuint)tformat << " " << (GLuint)ttype << std::endl;*/

	// Texture
	glGenTextures(1, &gTexture);
	glBindTexture(GL_TEXTURE_2D, gTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	return true;
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
	glBindVertexArray(gVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

auto main() -> int
{
	return run();
}
