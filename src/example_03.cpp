// Metaballs

#include "macros.h"
#include "entry.h"

#include "vec2.h"
#include "vec3.h"
#include "mat4.h"

const float PI = 3.14159265358979f;

float positions[9];

const char *scalarVertexShaderSource = R"(
#version 410 core

#define EPSILON 0.000001

uniform int samples;
uniform vec3 aPos[3];
out float outScalar;

void main()
{
    int x = gl_VertexID / samples;
    int y = gl_VertexID % samples;
    vec2 pos = vec2(x, y) / (samples - 1.0);
    
    outScalar = 0.0;
    for (int i=0; i<3; i++)
    {
        float distance = length(distance(aPos[i].xy, pos));
        outScalar += aPos[i].z / pow(max(EPSILON, distance), 2.0);
    }
}
)";

const char *scalarFragmentShaderSource = R"(
#version 410 core

void main()
{
}
)";

const char *cellVertexShaderSource = R"(
#version 410 core

uniform sampler2D scalar;

void main()
{
    int x = gl_VertexID / 64;
    int y = gl_VertexID % 64;
    vec2 pos = vec2(x, y) / (64 - 1.0);
    float s = textureLod(scalar, pos, 0.0).r;
    gl_Position = vec4(pos - vec2(0.5, 0.5), 0.0f, 1.0);
    gl_PointSize = s > 12.0 ? 2.0 : 1.0;
}
)";

const char *cellFragmentShaderSource = R"(
#version 410 core

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)";


const int SAMPLES = 64;

GLuint gScalarProgram;
GLuint gScalarTFO;
GLuint gScalarTFBO;
GLint gPosLoc;

GLuint gCellProgram;

float gTime = 0.0f;

auto init() -> bool
{
	//std::cout << "init " << gWidth << " " << gHeight << std::endl;

    glad_glTexStorage2D = (PFNGLTEXSTORAGE2DPROC)glfwGetProcAddress("glTexStorage2D"); // OpenGL 4.2 or ARB_texture_storage

    {
        auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
        GL_CHECK(glShaderSource(vertexShader, 1, &scalarVertexShaderSource, NULL));
        GL_CHECK(glCompileShader(vertexShader));
        {
            GLint success;
            GLchar infoLog[1024];
            GL_CHECK(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success));
            if(!success)
            {
                GLint infoLength = 0;
                glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLength);
                char* infoLog = new char[infoLength];
                GL_CHECK(glGetShaderInfoLog(vertexShader, infoLength, &infoLength, infoLog));
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "VERTEX_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                delete [] infoLog;
                GL_CHECK(glDeleteShader(vertexShader));
            }
        }

        auto fragmentShader = GL_CHECK_RETURN(glCreateShader(GL_FRAGMENT_SHADER));
        GL_CHECK(glShaderSource(fragmentShader, 1, &scalarFragmentShaderSource, NULL));
        GL_CHECK(glCompileShader(fragmentShader));
        {
            GLint success;
            GLchar infoLog[1024];
            GL_CHECK(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success));
            if(!success)
            {
                GLint infoLength = 0;
                glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLength);
                char* infoLog = new char[infoLength];
                GL_CHECK(glGetShaderInfoLog(fragmentShader, infoLength, &infoLength, infoLog));
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "FRAGMENT_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                delete [] infoLog;
                GL_CHECK(glDeleteShader(fragmentShader));
            }
        }

        gScalarProgram = GL_CHECK_RETURN(glCreateProgram());
        GL_CHECK(glAttachShader(gScalarProgram, vertexShader));
        GL_CHECK(glAttachShader(gScalarProgram, fragmentShader));
        const GLchar* feedbackVaryings[] = { "outScalar" };
        glTransformFeedbackVaryings(gScalarProgram, 1, feedbackVaryings, GL_SEPARATE_ATTRIBS);
        GL_CHECK(glLinkProgram(gScalarProgram));

        GL_CHECK(glDeleteShader(vertexShader));
        GL_CHECK(glDeleteShader(fragmentShader));
    }

    {
        auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
        GL_CHECK(glShaderSource(vertexShader, 1, &cellVertexShaderSource, NULL));
        GL_CHECK(glCompileShader(vertexShader));
        {
            GLint success;
            GLchar infoLog[1024];
            GL_CHECK(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success));
            if(!success)
            {
                GLint infoLength = 0;
                glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLength);
                char* infoLog = new char[infoLength];
                GL_CHECK(glGetShaderInfoLog(vertexShader, infoLength, &infoLength, infoLog));
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "VERTEX_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                delete [] infoLog;
                GL_CHECK(glDeleteShader(vertexShader));
            }
        }

        auto fragmentShader = GL_CHECK_RETURN(glCreateShader(GL_FRAGMENT_SHADER));
        GL_CHECK(glShaderSource(fragmentShader, 1, &cellFragmentShaderSource, NULL));
        GL_CHECK(glCompileShader(fragmentShader));
        {
            GLint success;
            GLchar infoLog[1024];
            GL_CHECK(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success));
            if(!success)
            {
                GLint infoLength = 0;
                glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLength);
                char* infoLog = new char[infoLength];
                GL_CHECK(glGetShaderInfoLog(fragmentShader, infoLength, &infoLength, infoLog));
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "FRAGMENT_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                delete [] infoLog;
                GL_CHECK(glDeleteShader(fragmentShader));
            }
        }

        gCellProgram = GL_CHECK_RETURN(glCreateProgram());
        GL_CHECK(glAttachShader(gCellProgram, vertexShader));
        GL_CHECK(glAttachShader(gCellProgram, fragmentShader));
        //const GLchar* feedbackVaryings[] = { "outCell" };
        //glTransformFeedbackVaryings(gCellProgram, 1, feedbackVaryings, GL_SEPARATE_ATTRIBS);
        GL_CHECK(glLinkProgram(gCellProgram));

        GL_CHECK(glDeleteShader(vertexShader));
        GL_CHECK(glDeleteShader(fragmentShader));
    }

    // TODO: ? Can phai co du khong su dung
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

    glUseProgram(gScalarProgram);
    glUniform1i(glGetUniformLocation(gScalarProgram, "samples"), SAMPLES);
    gPosLoc = glGetUniformLocation(gScalarProgram, "aPos");

    glGenTransformFeedbacks(1, &gScalarTFO);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, gScalarTFO);

    glGenBuffers(1, &gScalarTFBO);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, gScalarTFBO);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, SAMPLES * SAMPLES * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK_BUFFER, gScalarTFBO);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, gScalarTFBO);

    GLuint scalarTexture;
    glGenTextures(1, &scalarTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scalarTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, SAMPLES, SAMPLES);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	size();

    glEnable(GL_PROGRAM_POINT_SIZE);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //glEnable(GL_MULTISAMPLE);
	//glEnable(GL_DEPTH_TEST);

	return 0;
}

void size()
{
	//std::cout << "size " << gWidth << " " << gHeight << std::endl;
}

auto update() -> void
{
    gTime += 1.0/30.0f;
    vec2 pos1 = vec2(0.5f, 0.5f) + vec2(0.2f, 0.25f)*vec2(std::sinf((11.0f*gTime + 30.0f)*PI/180.0f), std::sinf((21.0f*gTime + 45.0f)*PI/180.0f));
    positions[0] = pos1.x;
    positions[1] = pos1.y;
    positions[2] = 0.1f;
    vec2 pos2 = vec2(0.5f, 0.5f) + vec2(0.25f, 0.2f)*vec2(std::sinf((22.0f*gTime + 45.0f)*PI/180.0f), std::sinf((32.0f*gTime + 90.0f)*PI/180.0f));
    positions[3] = pos2.x;
    positions[4] = pos2.y;
    positions[5] = 0.05f;
    vec2 pos3 = vec2(0.5f, 0.5f) + vec2(0.25f, 0.25f)*vec2(std::sinf((33.0f*gTime + 90.0f)*PI/180.0f), std::sinf((13.0f*gTime + 120.0f)*PI/180.0f));
    positions[6] = pos3.x;
    positions[7] = pos3.y;
    positions[8] = 0.25f;
}

auto draw() -> void
{
	glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_RASTERIZER_DISCARD);
        glUseProgram(gScalarProgram);
        glUniform3fv(gPosLoc, 3, positions);

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, gScalarTFO);
        glBeginTransformFeedback(GL_POINTS);
	        glDrawArrays(GL_POINTS, 0, SAMPLES * SAMPLES);
        glEndTransformFeedback();
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    glDisable(GL_RASTERIZER_DISCARD);

    glActiveTexture(GL_TEXTURE0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, gScalarTFBO);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SAMPLES, SAMPLES, GL_RED, GL_FLOAT, NULL);

    glUseProgram(gCellProgram);
    glDrawArrays(GL_POINTS, 0, SAMPLES * SAMPLES);
}

auto main() -> int
{
	return run();
}