// OIT Weighted Blended

#include "macros.h"
#include "entry.h"

#include "vec3.h"
#include "mat4.h"

const float PI = 3.14159265358979f;

const GLfloat zeroFillerVec[] = {0.0f, 0.0f, 0.0f, 0.0f};
const GLfloat oneFillerVec[] = {1.0f, 1.0f, 1.0f, 1.0f};

const char *vertexShaderSource = R"(
#version 410 core

const vec4 pos[4] = vec4[4](vec4( 1.0, -1.0, 0.0, 1.0),
							vec4(-1.0, -1.0, 0.0, 1.0),
							vec4(-1.0,  1.0, 0.0, 1.0),
							vec4( 1.0,  1.0, 0.0, 1.0));
const vec2 uvs[4] = vec2[4](vec2(1.0, 0.0),
							vec2(0.0, 0.0),
							vec2(0.0, 1.0),
							vec2(1.0, 1.0));

out vec2 vUV;

void main()
{
    vUV = uvs[gl_VertexID];
	gl_Position = pos[gl_VertexID];
}
)";

const char *fragmentShaderSource = R"(
#version 410 core

uniform sampler2D screen;

in vec2 vUV;

out vec4 FragColor;

void main()
{
    FragColor = vec4(texture(screen, vUV).rgb, 1.0f);
}
)";

const char *solidVertexShaderSource = R"(
#version 410 core

const vec4 pos[4] = vec4[4](vec4( 1.0, -1.0, 0.0, 1.0),
							vec4(-1.0, -1.0, 0.0, 1.0),
							vec4(-1.0,  1.0, 0.0, 1.0),
							vec4( 1.0,  1.0, 0.0, 1.0));

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    gl_Position = proj * view * world * pos[gl_VertexID];
}
)";

const char *solidFragmentShaderSource = R"(
#version 410 core

uniform vec3 color;

layout (location = 0) out vec4 frag;

void main()
{
    frag = vec4(color, 1.0);
}
)";

const char *transparentVertexShaderSource = R"(
#version 410 core

const vec4 pos[4] = vec4[4](vec4( 1.0, -1.0, 0.0, 1.0),
							vec4(-1.0, -1.0, 0.0, 1.0),
							vec4(-1.0,  1.0, 0.0, 1.0),
							vec4( 1.0,  1.0, 0.0, 1.0));

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    gl_Position = proj * view * world * pos[gl_VertexID];
}
)";

const char *transparentFragmentShaderSource = R"(
#version 410 core

uniform vec4 color;

layout (location = 0) out vec4 accum;
layout (location = 1) out float reveal;

void main()
{
    // weight function
	float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);
	
	// store pixel color accumulation
	accum = vec4(color.rgb * color.a, color.a) * weight;
	
	// store pixel revealage threshold
	reveal = color.a;
}
)";

const char *compositeVertexShaderSource = R"(
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

const char *compositeFragmentShaderSource = R"(
#version 410 core

const float EPSILON = 0.00001;

uniform sampler2D accum;
uniform sampler2D reveal;

layout (location = 0) out vec4 frag;

// caluclate floating point numbers equality accurately
bool isApproximatelyEqual(float a, float b)
{
	return abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * EPSILON;
}

// get the max value between three values
float max3(vec3 v) 
{
	return max(max(v.x, v.y), v.z);
}

void main()
{
    // fragment coordination
	ivec2 coords = ivec2(gl_FragCoord.xy);
	
	// fragment revealage
	float revealage = texelFetch(reveal, coords, 0).r;
	
	// save the blending and color texture fetch cost if there is not a transparent fragment
	if (isApproximatelyEqual(revealage, 1.0f)) 
		discard;
 
	// fragment color
	vec4 accumulation = texelFetch(accum, coords, 0);
	
	// suppress overflow
	if (isinf(max3(abs(accumulation.rgb)))) 
		accumulation.rgb = vec3(accumulation.a);

	// prevent floating point precision bug
	vec3 average_color = accumulation.rgb / max(accumulation.a, EPSILON);

	// blend pixels
	frag = vec4(average_color, 1.0f - revealage);
}
)";

GLuint gProgram;

GLuint gSolidProgram;
GLint gSolidWorldLoc;
GLint gSolidViewLoc;
GLint gSolidColorLoc;

GLuint gTransparentProgram;
GLint gTransparentWorldLoc;
GLint gTransparentViewLoc;
GLint gTransparentColorLoc;

GLuint gCompositeProgram;

mat4 gView;

GLuint gDepthTexture;

GLuint gOpaqueFBO;
GLuint gOpaqueTexture;

GLuint gTransparentFBO;
GLuint gAccumTexture;
GLuint gRevealTexture;

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
    {
        auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
        GL_CHECK(glShaderSource(vertexShader, 1, &solidVertexShaderSource, NULL));
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
        GL_CHECK(glShaderSource(fragmentShader, 1, &solidFragmentShaderSource, NULL));
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

        gSolidProgram = GL_CHECK_RETURN(glCreateProgram());
        GL_CHECK(glAttachShader(gSolidProgram, vertexShader));
        GL_CHECK(glAttachShader(gSolidProgram, fragmentShader));
        GL_CHECK(glLinkProgram(gSolidProgram));

        GL_CHECK(glDeleteShader(vertexShader));
        GL_CHECK(glDeleteShader(fragmentShader));
    }
    {
        auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
        GL_CHECK(glShaderSource(vertexShader, 1, &transparentVertexShaderSource, NULL));
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
        GL_CHECK(glShaderSource(fragmentShader, 1, &transparentFragmentShaderSource, NULL));
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

        gTransparentProgram = GL_CHECK_RETURN(glCreateProgram());
        GL_CHECK(glAttachShader(gTransparentProgram, vertexShader));
        GL_CHECK(glAttachShader(gTransparentProgram, fragmentShader));
        GL_CHECK(glLinkProgram(gTransparentProgram));

        GL_CHECK(glDeleteShader(vertexShader));
        GL_CHECK(glDeleteShader(fragmentShader));
    }
    {
        auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
        GL_CHECK(glShaderSource(vertexShader, 1, &compositeVertexShaderSource, NULL));
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
        GL_CHECK(glShaderSource(fragmentShader, 1, &compositeFragmentShaderSource, NULL));
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

        gCompositeProgram = GL_CHECK_RETURN(glCreateProgram());
        GL_CHECK(glAttachShader(gCompositeProgram, vertexShader));
        GL_CHECK(glAttachShader(gCompositeProgram, fragmentShader));
        GL_CHECK(glLinkProgram(gCompositeProgram));

        GL_CHECK(glDeleteShader(vertexShader));
        GL_CHECK(glDeleteShader(fragmentShader));
    }

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glUseProgram(gProgram);
    glUniform1i(glGetUniformLocation(gProgram, "screen"), 0);

    glUseProgram(gSolidProgram);
    gSolidWorldLoc = glGetUniformLocation(gSolidProgram, "world");
	gSolidViewLoc = glGetUniformLocation(gSolidProgram, "view");
    gSolidColorLoc = glGetUniformLocation(gSolidProgram, "color");

    glUseProgram(gTransparentProgram);
    gTransparentWorldLoc = glGetUniformLocation(gTransparentProgram, "world");
	gTransparentViewLoc = glGetUniformLocation(gTransparentProgram, "view");
    gTransparentColorLoc = glGetUniformLocation(gTransparentProgram, "color");

    glUseProgram(gCompositeProgram);
    glUniform1i(glGetUniformLocation(gCompositeProgram, "accum"), 1);
    glUniform1i(glGetUniformLocation(gCompositeProgram, "reveal"), 2);

    glGenTextures(1, &gDepthTexture);
	glBindTexture(GL_TEXTURE_2D, gDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, gWidth, gHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &gOpaqueFBO);
    // set up attachments for opaque framebuffer
    glGenTextures(1, &gOpaqueTexture);
	glBindTexture(GL_TEXTURE_2D, gOpaqueTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gWidth, gHeight, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, gOpaqueFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gOpaqueTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepthTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Opaque framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &gTransparentFBO);
    // set up attachments for transparent framebuffer
	glGenTextures(1, &gAccumTexture);
	glBindTexture(GL_TEXTURE_2D, gAccumTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gWidth, gHeight, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &gRevealTexture);
	glBindTexture(GL_TEXTURE_2D, gRevealTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, gWidth, gHeight, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, gTransparentFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gAccumTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gRevealTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepthTexture, 0); // opaque framebuffer's depth texture

	const GLenum transparentDrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, transparentDrawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Transparent framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gOpaqueTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gAccumTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gRevealTexture);

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

    mat4 proj = mat4::perspective(45.0f * (PI/180.0f), static_cast<float>(gWidth)/gHeight, 0.1f, 100.0f);
    glUseProgram(gSolidProgram);
    glUniformMatrix4fv(glGetUniformLocation(gSolidProgram, "proj"), 1, false, proj.m);
    glUseProgram(gTransparentProgram);
    glUniformMatrix4fv(glGetUniformLocation(gTransparentProgram, "proj"), 1, false, proj.m);

    glBindTexture(GL_TEXTURE_2D, gDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, gWidth, gHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gOpaqueTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gWidth, gHeight, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gAccumTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gWidth, gHeight, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
	
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gRevealTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, gWidth, gHeight, 0, GL_RED, GL_FLOAT, NULL);
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

	vec4 eyePos = mat4::rotate(0.0f, 1.0f, 0.0f, -gRotX) * mat4::rotate(1.0f, 0.0f, 0.0f, -gRotY) * vec4(0.0f, 0.0f, 6.0f, 0.0f);
	gView = mat4::lookAt(vec3(eyePos.x, eyePos.y, eyePos.z), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
}

auto draw() -> void
{
    glBindFramebuffer(GL_FRAMEBUFFER, gOpaqueFBO);
        glViewport(0, 0, gWidth, gHeight);
        glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(gSolidProgram);
        mat4 world1 = mat4::identity;
        glUniformMatrix4fv(gSolidWorldLoc, 1, false, world1.m);
        glUniformMatrix4fv(gSolidViewLoc, 1, false, gView.m);
        glUniform3f(gSolidColorLoc, 1.0f, 0.0f, 0.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindFramebuffer(GL_FRAMEBUFFER, gTransparentFBO);
        glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunci(0, GL_ONE, GL_ONE);
		glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
		glBlendEquation(GL_FUNC_ADD);
        glClearBufferfv(GL_COLOR, 0, zeroFillerVec);
		glClearBufferfv(GL_COLOR, 1, oneFillerVec);

        glUseProgram(gTransparentProgram);
        glUniformMatrix4fv(gTransparentViewLoc, 1, false, gView.m);

        mat4 world2 = mat4::translate(0.0f, 0.0f, -1.0f);
        glUniformMatrix4fv(gTransparentWorldLoc, 1, false, world2.m);
        glUniform4f(gTransparentColorLoc, 0.0f, 1.0f, 0.0f, 0.5f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        mat4 world3 = mat4::translate(0.0f, 0.0f, 1.0f);
        glUniformMatrix4fv(gTransparentWorldLoc, 1, false, world3.m);
        glUniform4f(gTransparentColorLoc, 0.0f, 0.0f, 1.0f, 0.5f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        mat4 world4 = mat4::translate(0.0f, 0.0f, 1.5f);
        glUniformMatrix4fv(gTransparentWorldLoc, 1, false, world4.m);
        glUniform4f(gTransparentColorLoc, 0.0f, 0.0f, 1.0f, 0.5f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindFramebuffer(GL_FRAMEBUFFER, gOpaqueFBO);
        glDepthFunc(GL_ALWAYS);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(gCompositeProgram);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE); // enable depth writes so glClear won't ignore clearing the depth buffer
		glDisable(GL_BLEND);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glUseProgram(gProgram);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

auto main() -> int
{
	return run();
}