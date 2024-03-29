#include "entry.h"

// Compute shader. OpenGL >= 4.3

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

const char *computeShaderSource = R"(
#version 430 core

uniform writeonly image2D destTex;

layout (local_size_x = 16, local_size_y = 16) in;

void main()
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	//float localCoef = length(vec2(ivec2(gl_LocalInvocationID.xy)-8)/8.0);
	//float globalCoef = sin(float(gl_WorkGroupID.x+gl_WorkGroupID.y)*0.1)*0.5;
	//imageStore(destTex, storePos, vec4(1.0-globalCoef*localCoef, 0.0, 0.0, 0.0));
	ivec2 id = ivec2(gl_LocalInvocationID.xy);
	float r = storePos.x & storePos.y;
	imageStore(destTex, storePos, vec4(r, 0.0, 0.0, 0.0));
}
)";

const char *vertexShaderSource = R"(
#version 430 core

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
#version 430 core

uniform sampler2D texture0;

in vec4 vColor;
in vec2 vTexCoord;

out vec4 FragColor;

void main()
{
	float red = texture(texture0, vTexCoord).r;
	//FragColor = vColor * texture(texture0, vTexCoord);
	FragColor = vec4(red, 0.0, 0.0, 1.0);
}
)";

GLuint gComputeProgram;
GLuint gProgram;
GLuint gVAO;
GLuint gTexture;

auto init() -> bool
{
	auto computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShader, 1, &computeShaderSource, nullptr);
	glCompileShader(computeShader);

	int rvalue;
	glGetShaderiv(computeShader, GL_COMPILE_STATUS, &rvalue);
	if (!rvalue) {
		fprintf(stderr, "Error in compiling the compute shader\n");
		GLchar log[10240];
		GLsizei length;
		glGetShaderInfoLog(computeShader, 10239, &length, log);
		fprintf(stderr, "Compiler log:\n%s\n", log);
		exit(40);
	}

	gComputeProgram = glCreateProgram();
	glAttachShader(gComputeProgram, computeShader);
	glLinkProgram(gComputeProgram);

	glDeleteShader(gComputeProgram);

	// ----
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

	// Pass Matrix see: https://learnopengl.com/Advanced-OpenGL/Instancing
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 512, 512, 0, GL_RED, GL_FLOAT, NULL);

	glBindImageTexture(0, gTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

	glUseProgram(gComputeProgram);
	glUniform1i(glGetUniformLocation(gComputeProgram, "destTex"), 0);

	glDispatchCompute(512/16, 512/16, 1);

	on_size();

	return true;
}

void on_size()
{
	//std::cout << "size " << gWidth << " " << gHeight << std::endl;
	glViewport(0, 0, gWidth, gHeight);
}

void on_key(int key, int action)
{
}

void on_mouse(double xpos, double ypos)
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
	glBindVertexArray(gVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

auto main() -> int
{
	return run();
}
