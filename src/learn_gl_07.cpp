#include "entry.h"

float vertices[] = {
	// Pos
	0.5f,  0.5f,
	0.5f, -0.5f,
	-0.5f, -0.5f,
	-0.5f,  0.5f
};

unsigned int indices[] = {  // note that we start from 0!
	0, 1, 3,  // first Triangle
	1, 2, 3   // second Triangle
};

const char *renderSceneVertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec2 aPos;

uniform vec2 offsets[25];
uniform vec4 colors[25];

out vec4 vColor;

void main()
{
	vec2 offset = vec2(-0.44+(gl_InstanceID%5)*0.22, -0.44+(gl_InstanceID/5)*0.22);
	vColor = colors[gl_InstanceID];
	vec3 pos = mat3(
		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		offset.x, offset.y, 1.0
	)*mat3(
		0.2, 0.0, 0.0,
		0.0, 0.2, 0.0,
		0.0, 0.0, 1.0
	)*vec3(aPos.x, aPos.y, 1.0);
	gl_Position = vec4(pos.x, pos.y, 1.0, 1.0);
}
)";

const char *renderTextureVertexShaderSource = R"(
#version 410 core

const vec4 pos[4] = vec4[4](vec4( 1.0, -1.0, 0.0, 1.0),
							vec4(-1.0, -1.0, 0.0, 1.0),
							vec4(-1.0,  1.0, 0.0, 1.0),
							vec4( 1.0,  1.0, 0.0, 1.0));
const vec2 uvs[4] = vec2[4](vec2(1.0, 0.0),
							vec2(0.0, 0.0),
							vec2(0.0, 1.0),
							vec2(1.0, 1.0));
out vec2 vTexCoord;

void main()
{
	vTexCoord = uvs[gl_VertexID];
	gl_Position = pos[gl_VertexID];
}
)";

const char *renderSceneFragmentShaderSource = R"(
#version 410 core

in vec4 vColor;

out vec4 FragColor;

void main()
{
	FragColor = vColor;
}
)";

const char *renderLuminanceFragmentShaderSource = R"(
#version 410 core

uniform sampler2D texture0;

in vec2 vTexCoord;

out vec4 FragColor;

void main()
{
	vec4 color = texture(texture0, vTexCoord);
	float luminance = 0.2125 * color.x + 0.7154 * color.y + 0.0721 * color.z;
	if (luminance > 0.9)
	{
		FragColor = color;
	}
	else
	{
		FragColor = vec4(0.0);
	}
}
)";

const char *renderBlurHorFragmentShaderSource = R"(
#version 410 core

const float gaussianWeights[] = float[] (0.2270270270, 0.3162162162, 0.0702702703);

uniform sampler2D texture0;

in vec2 vTexCoord;

out vec4 FragColor;

void main()
{
	float blurStep = 1.6 / float((textureSize(texture0, 0)).x);

	vec4 totalColor = texture(texture0, vec2(vTexCoord.x + 1.0 * blurStep, vTexCoord.y)) * gaussianWeights[0]
					+ texture(texture0, vec2(vTexCoord.x + 2.0 * blurStep, vTexCoord.y)) * gaussianWeights[1]
					+ texture(texture0, vec2(vTexCoord.x + 3.0 * blurStep, vTexCoord.y)) * gaussianWeights[2]
					+ texture(texture0, vec2(vTexCoord.x - 1.0 * blurStep, vTexCoord.y)) * gaussianWeights[0]
					+ texture(texture0, vec2(vTexCoord.x - 2.0 * blurStep, vTexCoord.y)) * gaussianWeights[1]
					+ texture(texture0, vec2(vTexCoord.x - 3.0 * blurStep, vTexCoord.y)) * gaussianWeights[2];

	FragColor = vec4(totalColor.xyz, 1.0);
}
)";

const char *renderBlurVerFragmentShaderSource = R"(
#version 410 core

const float gaussianWeights[] = float[] (0.2270270270, 0.3162162162, 0.0702702703);

uniform sampler2D texture0;

in vec2 vTexCoord;

out vec4 FragColor;

void main()
{
	float blurStep = 1.6 / float((textureSize(texture0, 0)).y);

	vec4 totalColor = texture(texture0, vec2(vTexCoord.x, vTexCoord.y + 1.0 * blurStep)) * gaussianWeights[0]
					+ texture(texture0, vec2(vTexCoord.x, vTexCoord.y + 2.0 * blurStep)) * gaussianWeights[1]
					+ texture(texture0, vec2(vTexCoord.x, vTexCoord.y + 3.0 * blurStep)) * gaussianWeights[2]
					+ texture(texture0, vec2(vTexCoord.x, vTexCoord.y - 1.0 * blurStep)) * gaussianWeights[0]
					+ texture(texture0, vec2(vTexCoord.x, vTexCoord.y - 2.0 * blurStep)) * gaussianWeights[1]
					+ texture(texture0, vec2(vTexCoord.x, vTexCoord.y - 3.0 * blurStep)) * gaussianWeights[2];

	FragColor = vec4(totalColor.xyz, 1.0);
}
)";

const char *renderTextureFragmentShaderSource = R"(
#version 410 core

uniform sampler2D texture0;

in vec2 vTexCoord;

out vec4 FragColor;

void main()
{
	FragColor = texture(texture0, vTexCoord);
}
)";

const char *blendTextureFragmentShaderSource = R"(
#version 410 core

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;

uniform float mixFactor;

in vec2 vTexCoord;

out vec4 FragColor;

void main()
{
	vec4 color0 = texture(texture0, vTexCoord);
	vec4 color1 = texture(texture1, vTexCoord);
	vec4 color2 = texture(texture2, vTexCoord);
	FragColor = color0 + color1 + mix(color1, color2, mixFactor)*mixFactor;
}
)";

GLuint gRenderSceneProgram;
GLuint gRenderLuminanceProgram;
GLuint gRenderBlurHorProgram;
GLuint gRenderBlurVerProgram;
GLuint gRenderTextureProgram;
GLuint gBlendTextureProgram;
GLuint gVAO;

GLuint gFBO;
GLuint gFrameTexture;
GLuint gLuminanceFBO;
GLuint gLuminanceTexture;
GLuint gBlurFBO[2];
GLuint gBlurTexture[2];
GLuint gStrongerBlurFBO;
GLuint gStrongerBlurTexture;

GLint gMixFactorLoc;
float gTime = 0.0f;
float gMixFactor = 0.0f;

auto init() -> bool
{
	{
		auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &renderSceneVertexShaderSource, nullptr);
		glCompileShader(vertexShader);

		auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &renderSceneFragmentShaderSource, nullptr);
		glCompileShader(fragmentShader);

		gRenderSceneProgram = glCreateProgram();
		glAttachShader(gRenderSceneProgram, vertexShader);
		glAttachShader(gRenderSceneProgram, fragmentShader);
		glLinkProgram(gRenderSceneProgram);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	{
		auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &renderTextureVertexShaderSource, nullptr);
		glCompileShader(vertexShader);

		auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &renderLuminanceFragmentShaderSource, nullptr);
		glCompileShader(fragmentShader);

		gRenderLuminanceProgram = glCreateProgram();
		glAttachShader(gRenderLuminanceProgram, vertexShader);
		glAttachShader(gRenderLuminanceProgram, fragmentShader);
		glLinkProgram(gRenderLuminanceProgram);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	{
		auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &renderTextureVertexShaderSource, nullptr);
		glCompileShader(vertexShader);

		auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &renderBlurHorFragmentShaderSource, nullptr);
		glCompileShader(fragmentShader);

		gRenderBlurHorProgram = glCreateProgram();
		glAttachShader(gRenderBlurHorProgram, vertexShader);
		glAttachShader(gRenderBlurHorProgram, fragmentShader);
		glLinkProgram(gRenderBlurHorProgram);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	{
		auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &renderTextureVertexShaderSource, nullptr);
		glCompileShader(vertexShader);

		auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &renderBlurVerFragmentShaderSource, nullptr);
		glCompileShader(fragmentShader);

		gRenderBlurVerProgram = glCreateProgram();
		glAttachShader(gRenderBlurVerProgram, vertexShader);
		glAttachShader(gRenderBlurVerProgram, fragmentShader);
		glLinkProgram(gRenderBlurVerProgram);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	{
		auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &renderTextureVertexShaderSource, nullptr);
		glCompileShader(vertexShader);

		auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &renderTextureFragmentShaderSource, nullptr);
		glCompileShader(fragmentShader);

		gRenderTextureProgram = glCreateProgram();
		glAttachShader(gRenderTextureProgram, vertexShader);
		glAttachShader(gRenderTextureProgram, fragmentShader);
		glLinkProgram(gRenderTextureProgram);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	// Blend
	{
		auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &renderTextureVertexShaderSource, nullptr);
		glCompileShader(vertexShader);

		auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &blendTextureFragmentShaderSource, nullptr);
		glCompileShader(fragmentShader);

		gBlendTextureProgram = glCreateProgram();
		glAttachShader(gBlendTextureProgram, vertexShader);
		glAttachShader(gBlendTextureProgram, fragmentShader);
		glLinkProgram(gBlendTextureProgram);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	glGenVertexArrays(1, &gVAO);
	glBindVertexArray(gVAO);
		GLuint VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		GLuint EBO;
		glGenBuffers(1, &EBO);
		std::cout << "EBO: " << EBO << std::endl;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindVertexArray(0);

	glUseProgram(gRenderSceneProgram);
	for (size_t i = 0; i < 25; i++)
	{
		bool isWhite = false;
		isWhite = i == 0 || i == 4 || i == 6 || i == 8 || i == 12 || i == 16 || i == 18 || i == 20 || i == 24;
		float r = isWhite ? 1.0f : 0.2f;
		float g = isWhite ? 1.0f : 0.4f;
		float b = isWhite ? 1.0f : 0.8f;
		glUniform4f(glGetUniformLocation(gRenderSceneProgram, ("colors[" +std::to_string(i) + "]").c_str()), r, g, b, 1.0f);
	}
	glUseProgram(gBlendTextureProgram);
	glUniform1i(glGetUniformLocation(gBlendTextureProgram, "texture0"), 0);
	glUniform1i(glGetUniformLocation(gBlendTextureProgram, "texture1"), 1);
	glUniform1i(glGetUniformLocation(gBlendTextureProgram, "texture2"), 2);
	gMixFactorLoc = glGetUniformLocation(gBlendTextureProgram, "mixFactor");

	glad_glTexStorage2D = (PFNGLTEXSTORAGE2DPROC)glfwGetProcAddress("glTexStorage2D"); // OpenGL 4.2 or ARB_texture_storage

	// Framebuffer
	glGenFramebuffers(1, &gFBO);
	glGenTextures(1, &gFrameTexture);

	glBindTexture(GL_TEXTURE_2D, gFrameTexture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gWidth, gHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); // glTexImage2D vs glTexStorage2D
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, gWidth, gHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, gFBO);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gFrameTexture, 0); // GL_FRAMEBUFFER vs GL_DRAW_FRAMEBUFFER

	// Luminance Framebuffer
	glGenFramebuffers(1, &gLuminanceFBO);
	glGenTextures(1, &gLuminanceTexture);

	glBindTexture(GL_TEXTURE_2D, gLuminanceTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, gWidth / 2, gHeight / 2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, gLuminanceFBO);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gLuminanceTexture, 0);

	// Blur Framebuffer
	glGenFramebuffers(2, gBlurFBO);
	glGenTextures(2, gBlurTexture);

	for (size_t i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, gBlurTexture[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, gWidth / 2, gHeight / 2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, gBlurFBO[i]);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBlurTexture[i], 0);
	}

	// StrongerBlur Framebuffer
	glGenFramebuffers(1, &gStrongerBlurFBO);
	glGenTextures(1, &gStrongerBlurTexture);

	glBindTexture(GL_TEXTURE_2D, gStrongerBlurTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, gWidth / 2, gHeight / 2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, gStrongerBlurFBO);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gStrongerBlurTexture, 0);

	return true;
}

void size()
{
	//std::cout << "size " << gWidth << " " << gHeight << std::endl;
}

auto update() -> void
{
	gTime += 1.0/60.0f;
	gMixFactor = 0.5 + sinf(gTime * 3.0f) / 2.0f;
}

auto draw() -> void
{
	// Draw to framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, gFBO);

	glViewport(0, 0, gWidth, gHeight);
	glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(gRenderSceneProgram);
	glBindVertexArray(gVAO);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 25);

	// Luminance
	glBindFramebuffer(GL_FRAMEBUFFER, gLuminanceFBO);

	glViewport(0, 0, gWidth/2, gHeight/2);

	glUseProgram(gRenderLuminanceProgram);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gFrameTexture);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// Blur
	glBindFramebuffer(GL_FRAMEBUFFER, gBlurFBO[0]);
	glUseProgram(gRenderBlurHorProgram);
	glBindTexture(GL_TEXTURE_2D, gLuminanceTexture);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindFramebuffer(GL_FRAMEBUFFER, gBlurFBO[1]);
	glUseProgram(gRenderBlurVerProgram);
	glBindTexture(GL_TEXTURE_2D, gBlurTexture[0]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	int stepTotal = 4;
	for (size_t i = 0; i < stepTotal; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, gBlurFBO[0]);
		glUseProgram(gRenderBlurHorProgram);
		glBindTexture(GL_TEXTURE_2D, gBlurTexture[1]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		glBindFramebuffer(GL_FRAMEBUFFER, i == (stepTotal - 1) ? gStrongerBlurFBO : gBlurFBO[1]);
		glUseProgram(gRenderBlurVerProgram);
		glBindTexture(GL_TEXTURE_2D, gBlurTexture[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	// Draw to backbuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, gWidth, gHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(gBlendTextureProgram);
	glUniform1f(gMixFactorLoc, gMixFactor);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gFrameTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBlurTexture[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gStrongerBlurTexture);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

auto main() -> int
{
	return run();
}
