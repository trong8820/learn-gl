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

const char *renderSceneFragmentShaderSource = R"(
#version 410 core

in vec4 vColor;

out vec4 FragColor0;
out vec4 FragColor1;

void main()
{
	FragColor0 = vColor;
    float luminance = 0.2125 * vColor.x + 0.7154 * vColor.y + 0.0721 * vColor.z;
    if (luminance > 0.9)
    {
	    FragColor1 = vColor;
    } 
    else 
    {
        FragColor1 = vec4(0.0);
    }
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

GLuint gRenderSceneProgram;
GLuint gRenderTextureProgram;
GLuint gVAO;

GLuint gFBO;
GLuint gFrameTexture;
GLuint gLuminanceTexture;

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
        glShaderSource(fragmentShader, 1, &renderTextureFragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        gRenderTextureProgram = glCreateProgram();
        glAttachShader(gRenderTextureProgram, vertexShader);
        glAttachShader(gRenderTextureProgram, fragmentShader);
        glLinkProgram(gRenderTextureProgram);

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

    glad_glTexStorage2D = (PFNGLTEXSTORAGE2DPROC)glfwGetProcAddress("glTexStorage2D"); // OpenGL 4.2 or ARB_texture_storage

    // Framebuffer
    glGenFramebuffers(1, &gFBO);
    glGenTextures(1, &gFrameTexture);
    glGenTextures(1, &gLuminanceTexture);
    
    glBindTexture(GL_TEXTURE_2D, gFrameTexture);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gWidth, gHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); // glTexImage2D vs glTexStorage2D
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, gWidth, gHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, gLuminanceTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, gWidth, gHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, gFBO);
    GLenum drawBuffers[]={GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gFrameTexture, 0); // GL_FRAMEBUFFER vs GL_DRAW_FRAMEBUFFER
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gLuminanceTexture, 0);

    return true;
}

auto update() -> void
{

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
    
    // Draw to backbuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, gWidth, gHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(gRenderTextureProgram);
    glBindTexture(GL_TEXTURE_2D, gFrameTexture);
    //glBindTexture(GL_TEXTURE_2D, gLuminanceTexture);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

auto main() -> int
{
	return run();
}
