// Raymarch

#include "macros.h"
#include "entry.h"

#include "vec3.h"
#include "mat4.h"

const float PI = 3.14159265358979f;

const char *vertexShaderSource = R"(
#version 410 core

const vec4 pos[4] = vec4[4](vec4( 1.0, -1.0, 0.0, 1.0),
							vec4(-1.0, -1.0, 0.0, 1.0),
							vec4(-1.0,  1.0, 0.0, 1.0),
							vec4( 1.0,  1.0, 0.0, 1.0));

void main()
{
	gl_Position = pos[gl_VertexID];
}
)";

const char *fragmentShaderSource = R"(
#version 410 core

const int MAX_MARCHING_STEPS = 255;
const float MIN_DIST = 0.0;
const float MAX_DIST = 100.0;
const float EPSILON = 0.0001;

uniform mat4 view;
uniform vec3 eyePos;
uniform vec2 resolution;

out vec4 FragColor;

float sdTorus( vec3 p, vec2 t )
{
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

float cubeSDF(vec3 p, vec3 b)
{
    vec3 d = abs(p) - b;
    float insideDistance = min(max(d.x, max(d.y, d.z)), 0.0);
    float outsideDistance = length(max(d, 0.0));
    return insideDistance + outsideDistance;
}

float sdRoundBox( vec3 p, vec3 b, float r )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - r;
}

float sceneSDF(vec3 samplePoint) 
{
    float d = cubeSDF(samplePoint + vec3(-4.2, 0.0, 0.0), vec3(1.0, 1.0, 1.0));
    d = min(d, sdTorus(samplePoint, vec2(1.5, 0.5)));
    d = min(d, sdRoundBox(samplePoint + vec3(4.2, 0.0, 0.0), vec3(1.0, 1.0, 1.0), 0.5));
    return d;
}

float shortestDistanceToSurface(vec3 marchingDirection, float start, float end) 
{
    float depth = start;
    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        float dist = sceneSDF(eyePos + depth * marchingDirection);
        if (dist < EPSILON) {
			return depth;
        }
        depth += dist;
        if (depth >= end) {
            return end;
        }
    }
    return end;
}

vec3 rayDirection(float fieldOfView, vec2 size, vec2 fragCoord)
{
    vec2 xy = fragCoord - size / 2.0;
    float z = size.y / tan(radians(fieldOfView) / 2.0);
    return normalize(vec3(xy, -z));
}

void main()
{
    vec4 viewPos = vec4(-gl_FragCoord.xy + resolution/2.0, resolution.y / 2.0 / tan(radians(45.0)), 1.0); // 45.0 FOV
    vec4 worldPos = inverse(view) * viewPos;

    vec3 dir = normalize(eyePos - worldPos.xyz);

    float dist = shortestDistanceToSurface(dir, MIN_DIST, MAX_DIST);
    if (dist > MAX_DIST - EPSILON) 
    {
        FragColor = vec4(0.0, 0.2, 0.2, 0.0);
        return;
    }

    FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}
)";

GLuint gProgram;
GLint gViewLoc;
GLint gEyePosLoc;
GLint gResolutionLoc;

mat4 gView;

vec3 gEyePos;

double gPrevPosX;
double gPrevPosY;
float gTargetRotX;
float gTargetRotY;
float gRotX;
float gRotY;

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

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glUseProgram(gProgram);
	gViewLoc = glGetUniformLocation(gProgram, "view");
    gEyePosLoc = glGetUniformLocation(gProgram, "eyePos");
    gResolutionLoc = glGetUniformLocation(gProgram, "resolution");

    on_size();

    return true;
}

auto on_size() -> void
{
    //std::cout << "size " << gWidth << " " << gHeight << std::endl;
    glViewport(0, 0, gWidth, gHeight);

    glUseProgram(gProgram);
    glUniform2f(gResolutionLoc, gWidth, gHeight);
}

auto on_key(int key, int action) -> void
{

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
    gRotX += 0.05 * (gTargetRotX - gRotX);
	gRotY += 0.05 * (gTargetRotY - gRotY);

	vec4 eyePos = mat4::rotate(0.0f, 1.0f, 0.0f, -gRotX) * mat4::rotate(1.0f, 0.0f, 0.0f, -gRotY) * vec4(0.0f, 0.0f, 7.0f, 0.0f);
    gEyePos = vec3(eyePos.x, eyePos.y, eyePos.z);
	gView = mat4::lookAt(gEyePos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
}

auto draw() -> void
{
    glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gProgram);
    glUniformMatrix4fv(gViewLoc, 1, false, gView.m);
    glUniform3f(gEyePosLoc, gEyePos.x, gEyePos.y, gEyePos.z);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

auto main() -> int
{
	return run();
}