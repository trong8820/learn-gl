#include "entry.h"

// Tessellation
// https://github.com/Erkaman/tess-opt

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

out vec4 tcPos;
out vec4 tcColor;
out vec2 tcTexCoord;

void main()
{
    tcPos = vec4(aPos.x, aPos.y, 0.0, 1.0);
	tcColor = aColor;
	tcTexCoord = aTexCoord;
}
)";

// Tessellation Control
const char *tcShaderSource = R"(
#version 410 core

in vec4 tcPos[];
in vec4 tcColor[];
in vec2 tcTexCoord[];

layout(vertices=3) out;
out vec4 tePos[];
out vec4 teColor[];
out vec2 teTexCoord[];

void main()
{
    tePos[gl_InvocationID] = tcPos[gl_InvocationID];
    teColor[gl_InvocationID] = tcColor[gl_InvocationID];
    teTexCoord[gl_InvocationID] = tcTexCoord[gl_InvocationID];

    gl_TessLevelOuter[0] = 3;
    gl_TessLevelOuter[1] = 3;
    gl_TessLevelOuter[2] = 3;

    gl_TessLevelInner[0] = 3;
}
)";

// Tessellation Evaluation
const char *teShaderSource = R"(
#version 410 core

layout(triangles,equal_spacing) in;
in vec4 tePos[];
in vec4 teColor[];
in vec2 teTexCoord[];

out vec4 vColor;
out vec2 vTexCoord;

vec3 lerp3D(vec3 v0, vec3 v1, vec3 v2)
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

void main()
{
    vColor = teColor[0];
    vTexCoord = teTexCoord[0];
    vec3 pos = lerp3D(tePos[0].xyz, tePos[1].xyz, tePos[2].xyz);
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
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
	//FragColor = vColor * texture(texture0, vTexCoord);
    //FragColor = vColor;
    FragColor = vec4(0.0, 1.0, 0.0, 1.0);
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

    auto tcShader = glCreateShader(GL_TESS_CONTROL_SHADER);
	glShaderSource(tcShader, 1, &tcShaderSource, nullptr);
	glCompileShader(tcShader);

    auto teShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
	glShaderSource(teShader, 1, &teShaderSource, nullptr);
	glCompileShader(teShader);

	auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
	glCompileShader(fragmentShader);

	gProgram = glCreateProgram();
	glAttachShader(gProgram, vertexShader);
    glAttachShader(gProgram, tcShader);
    glAttachShader(gProgram, teShader);
	glAttachShader(gProgram, fragmentShader);
	glLinkProgram(gProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(tcShader);
	glDeleteShader(teShader);
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	return true;
}

auto update() -> void
{

}

auto draw() -> void
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(gProgram);
	glBindVertexArray(gVAO);
	glDrawElements(GL_PATCHES, 3, GL_UNSIGNED_INT, 0);
}

auto main() -> int
{
	return run();
}
