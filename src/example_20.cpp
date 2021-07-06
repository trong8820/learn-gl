// Billboards
// http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/

#include "macros.h"
#include "entry.h"

#include "vec3.h"
#include "mat4.h"

const float PI = 3.14159265358979f;

float cubeVertices[] =
{
	// back face
	-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
	1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
	1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
	1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
	-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
	-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
	// front face
	-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
	1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
	1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
	1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
	-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
	-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
	// left face
	-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
	-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
	-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
	-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
	-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
	-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
	// right face
	1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
	1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
	1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
	1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
	1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
	1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
	// bottom face
	-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
	1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
	1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
	1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
	-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
	-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
	// top face
	-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
	1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
	1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
	1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
	-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
	-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
};

const char *vertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    gl_Position = proj * view * world * vec4(aPos, 1.0);
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

const char *billboardVertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    //vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    //vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]);

    //vec3 vertexPos = vec3(0.0, 1.8, 0.0) + cameraRight*aPos.x + cameraUp*aPos.y;

    //gl_Position = proj * view * vec4(vertexPos, 1.0);

    vec3 vertexPos = vec3(0.0, 1.8, 0.0);
    gl_Position = proj * view * vec4(vertexPos, 1.0);
    gl_Position /= gl_Position.w;
    gl_Position.xy += aPos.xy * vec2(0.2, 0.05);
}
)";

const char *billboardFragmentShaderSource = R"(
#version 410 core

out vec4 FragColor;

void main()
{
    FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}
)";

GLuint gProgram;
GLint gWorldLoc;
GLint gViewLoc;

GLuint gBillboardProgram;
GLint gBillboardWorldLoc;
GLint gBillboardViewLoc;

GLuint gCubeVAO;

mat4 gView;

double gPrevPosX;
double gPrevPosY;
float gTargetRotX;
float gTargetRotY;
float gRotX;
float gRotY;

// Move
bool keyS {};
bool keyW {};
float gEyePosZ = 8.0f;

auto init() -> bool
{
    //std::cout << "init " << gWidth << " " << gHeight << std::endl;
    {
        auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
        GL_CHECK(glShaderSource(vertexShader, 1, &vertexShaderSource, NULL));
        GL_CHECK(glCompileShader(vertexShader));
        {
			GLint success;
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
        GL_CHECK(glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL));
        GL_CHECK(glCompileShader(fragmentShader));
        {
			GLint success;
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

        gProgram = GL_CHECK_RETURN(glCreateProgram());
        GL_CHECK(glAttachShader(gProgram, vertexShader));
        GL_CHECK(glAttachShader(gProgram, fragmentShader));
        GL_CHECK(glLinkProgram(gProgram));

        GL_CHECK(glDeleteShader(vertexShader));
        GL_CHECK(glDeleteShader(fragmentShader));
    }
    {
        auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
        GL_CHECK(glShaderSource(vertexShader, 1, &billboardVertexShaderSource, NULL));
        GL_CHECK(glCompileShader(vertexShader));
        {
			GLint success;
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
        GL_CHECK(glShaderSource(fragmentShader, 1, &billboardFragmentShaderSource, NULL));
        GL_CHECK(glCompileShader(fragmentShader));
        {
			GLint success;
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

        gBillboardProgram = GL_CHECK_RETURN(glCreateProgram());
        GL_CHECK(glAttachShader(gBillboardProgram, vertexShader));
        GL_CHECK(glAttachShader(gBillboardProgram, fragmentShader));
        GL_CHECK(glLinkProgram(gBillboardProgram));

        GL_CHECK(glDeleteShader(vertexShader));
        GL_CHECK(glDeleteShader(fragmentShader));
    }

    glGenVertexArrays(1, &gCubeVAO);
		glBindVertexArray(gCubeVAO);
		GLuint VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	

    glUseProgram(gProgram);
    gWorldLoc = glGetUniformLocation(gProgram, "world"); 
	gViewLoc = glGetUniformLocation(gProgram, "view");

    glUseProgram(gBillboardProgram);
    gBillboardWorldLoc = glGetUniformLocation(gBillboardProgram, "world"); 
	gBillboardViewLoc = glGetUniformLocation(gBillboardProgram, "view");

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

    on_size();

    return true;
}

auto on_size() -> void
{
    //std::cout << "size " << gWidth << " " << gHeight << std::endl;
	glViewport(0, 0, gWidth, gHeight);

    glUseProgram(gProgram);
	//GLint worldLoc = glGetUniformLocation(gProgram, "world");
	//GLint viewLoc = glGetUniformLocation(gProgram, "view");
	GLint projLoc = glGetUniformLocation(gProgram, "proj");

	//mat4 world = mat4::identity;
	//mat4 view = mat4::lookAt(vec3(0.0f, 3.0f, 3.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 proj = mat4::perspective(45.0f * (PI/180.0f), static_cast<float>(gWidth)/gHeight, 0.1f, 100.0f);
    //glUniformMatrix4fv(worldLoc, 1, false, world.m);
	//glUniformMatrix4fv(viewLoc, 1, false, view.m);
	glUniformMatrix4fv(projLoc, 1, false, proj.m);

    glUseProgram(gBillboardProgram);
    glUniformMatrix4fv(glGetUniformLocation(gBillboardProgram, "proj"), 1, false, proj.m);
}

auto on_key(int key, int action) -> void
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

auto on_mouse(double xpos, double ypos) -> void
{
    int state = glfwGetMouseButton(g_pWindow, GLFW_MOUSE_BUTTON_LEFT);
	if (state == GLFW_PRESS)
	{
		gTargetRotX += (xpos - gPrevPosX)*0.01f;
		gTargetRotY += (ypos - gPrevPosY)*0.01f;

		if (gTargetRotY <= -PI / 2.0f) gTargetRotY = -PI / 2.0f + 0.01f;
		if (gTargetRotY >= PI / 2.0f)  gTargetRotY = PI / 2.0f - 0.01f;
	}
	gPrevPosX = xpos;
	gPrevPosY = ypos;
}

auto update() -> void
{
    if(keyW)
    {
        gEyePosZ -= 0.02f;
    }
    if (keyS)
    {
        gEyePosZ += 0.02f;
    }

    gRotX += 0.05 * (gTargetRotX - gRotX);
	gRotY += 0.05 * (gTargetRotY - gRotY);

	vec4 eyePos = mat4::rotate(0.0f, 1.0f, 0.0f, -gRotX) * mat4::rotate(1.0f, 0.0f, 0.0f, -gRotY) * vec4(0.0f, 0.0f, gEyePosZ, 0.0f);
	gView = mat4::lookAt(vec3(eyePos.x, eyePos.y, eyePos.z), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
}

auto draw() -> void
{
    glViewport(0, 0, gWidth, gHeight);
    glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gProgram);
    glUniformMatrix4fv(gViewLoc, 1, false, gView.m);
    glBindVertexArray(gCubeVAO);
    mat4 world1 = mat4::identity;
    glUniformMatrix4fv(gWorldLoc, 1, false, world1.m);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glUseProgram(gBillboardProgram);
    glUniformMatrix4fv(gBillboardViewLoc, 1, false, gView.m);
    glBindVertexArray(gCubeVAO);
    mat4 world2 = mat4::scale(1.0f, 0.2f, 1.0f)*mat4::translate(0.0f, 1.8f, 0.0f);
    glUniformMatrix4fv(gBillboardWorldLoc, 1, false, world2.m);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

auto main() -> int
{
	return run();
}