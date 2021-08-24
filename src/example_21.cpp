// Cloth

#include "macros.h"
#include "entry.h"

#include <vector>

#define PRIM_RESTART 0xffffff

/*
float vertices[] = {
	// Pos
	0.5f,  0.5f, 0.0f, 0.0f,		  // top right
	0.5f, -0.5f, 0.0f, 0.0f,		  // bottom right
	-0.5f, -0.5f, 0.0f, 0.0f,		  // bottom left
	-0.5f,  0.5f, 0.0f, 0.0f		   // top left
};

unsigned int indices[] = {  // note that we start from 0!
	0, 1, 3,  // first Triangle
	1, 2, 3   // second Triangle
};
*/

const char* computeShaderSource = R"(
#version 460

layout(local_size_x = 10, local_size_y = 10) in;

uniform vec3 Gravity = vec3(0,-10,0);
uniform float ParticleMass = 0.2;
uniform float ParticleInvMass = 1.0 / 0.2;
uniform float SpringK = 2000.0;
uniform float RestLengthHoriz = 4.0 / 39.0;
uniform float RestLengthVert = 3.0 / 39.0;
uniform float RestLengthDiag = sqrt((4.0 / 39.0)*(4.0 / 39.0) + (3.0 / 39.0)*(3.0 / 39.0));
uniform float DeltaT = 0.000005;
uniform float DampingConst = 0.1;


layout( std140, binding = 0 ) buffer PositionIn
{
	vec4 PosIn[];
};

layout( std140, binding = 1 ) buffer PositionOut
{
	vec4 PosOut[];
};

layout( std140, binding = 2 ) buffer VelocityIn
{
	vec4 VelIn[];
};

layout( std140, binding = 3 ) buffer VelocityOut
{
	vec4 VelOut[];
};

void main()
{
	uvec3 nParticles = gl_NumWorkGroups * gl_WorkGroupSize;
	uint idx = gl_GlobalInvocationID.y * nParticles.x + gl_GlobalInvocationID.x;
	vec3 p = vec3(PosIn[idx]);
	vec3 v = vec3(VelIn[idx]);
	vec3 r;

	vec3 force = Gravity * ParticleMass;

	// Particle above
	if( gl_GlobalInvocationID.y < nParticles.y - 1 )
	{
		r = PosIn[idx + nParticles.x].xyz - p;
		force += normalize(r) * SpringK * (length(r) - RestLengthVert);
	}
	// Below
	if( gl_GlobalInvocationID.y > 0 )
	{
		r = PosIn[idx - nParticles.x].xyz - p;
		force += normalize(r) * SpringK * (length(r) - RestLengthVert);
	}
	// Left
	if( gl_GlobalInvocationID.x > 0 )
	{
		r = PosIn[idx-1].xyz - p;
		force += normalize(r) * SpringK * (length(r) - RestLengthHoriz);
	}
	// Right
	if( gl_GlobalInvocationID.x < nParticles.x - 1 )
	{
		r = PosIn[idx + 1].xyz - p;
		force += normalize(r) * SpringK * (length(r) - RestLengthHoriz);
	}

	// Diagonals
	// Upper-left
	if( gl_GlobalInvocationID.x > 0 && gl_GlobalInvocationID.y < nParticles.y - 1 )
	{
		r = PosIn[idx + nParticles.x - 1].xyz - p;
		force += normalize(r) * SpringK * (length(r) - RestLengthDiag);
	}
	// Upper-right
	if( gl_GlobalInvocationID.x < nParticles.x - 1 && gl_GlobalInvocationID.y < nParticles.y - 1)
	{
		r = PosIn[idx + nParticles.x + 1].xyz - p;
		force += normalize(r) * SpringK * (length(r) - RestLengthDiag);
	 }
	// lower -left
	if( gl_GlobalInvocationID.x > 0 && gl_GlobalInvocationID.y > 0 )
	{
		r = PosIn[idx - nParticles.x - 1].xyz - p;
		force += normalize(r) * SpringK * (length(r) - RestLengthDiag);
	}
	// lower-right
	if( gl_GlobalInvocationID.x < nParticles.x - 1 && gl_GlobalInvocationID.y > 0 )
	{
		r = PosIn[idx - nParticles.x + 1].xyz - p;
		force += normalize(r) * SpringK * (length(r) - RestLengthDiag);
	}

	force += -DampingConst * v;

	// Apply simple Euler integrator
	vec3 a = force * ParticleInvMass;

	PosOut[idx] = vec4(p + v * DeltaT + 0.5 * a * DeltaT * DeltaT, 1.0);
	VelOut[idx] = vec4(v + a * DeltaT, 0.0);
	//PosOut[idx] = vec4(p.x, p.y - 0.001, 0.0, 1.0);

	if( gl_GlobalInvocationID.y == nParticles.y - 1 &&
		  (gl_GlobalInvocationID.x == 0 ||
		   gl_GlobalInvocationID.x == nParticles.x / 4 ||
		   gl_GlobalInvocationID.x == nParticles.x * 2 / 4 ||
		   gl_GlobalInvocationID.x == nParticles.x * 3 / 4 ||
		   gl_GlobalInvocationID.x == nParticles.x - 1))
	{
		PosOut[idx] = vec4(p.x, p.y, p.z + 0.000001, 1.0);
		VelOut[idx] = vec4(0.0, 0.0, 0.0, 0.0);
	}
}
)";

const char* vertexShaderSource = R"(
#version 460

layout (location = 0) in vec4 aPos;

out vec4 vColor;
out vec2 vTexCoord;

void main()
{
	gl_PointSize = 4.0;
	gl_Position = vec4(-0.8 + aPos.x*0.4, -0.4 + aPos.y*0.4, 0.0, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 460

out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";

GLuint gComputeProgram;
GLuint gPosBufs[2];
GLuint gVelBufs[2];

GLuint gProgram;
GLuint gVAO;

GLsizei gIndiceCount;
int gReadBuferId = 0;


bool init()
{
	// Init data
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> velocities(40*40*4, 0.0f);
	float dx = 4.0f / (40 - 1);
	float dy = 3.0f / (40 - 1);
	for (int i = 0; i < 40; i++)
	{
		for (int j = 0; j < 40; j++)
		{
			float x = dx * j;
			float y = dy * i;

			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(0.0f);
			vertices.push_back(1.0f);
		}
	}

	std::vector<GLuint> indices;
	for (int row = 0; row < 40 - 1; row++)
	{
		for (int col = 0; col < 40; col++)
		{
			indices.push_back((row + 1) * 40 + (col));
			indices.push_back((row)*40 + (col));
		}
		indices.push_back(PRIM_RESTART);
	}
	gIndiceCount = static_cast<GLsizei>(indices.size());

	//std::cout << "init " << gWidth << " " << gHeight << std::endl;
	{
		auto computeShader = GL_CHECK_RETURN(glCreateShader(GL_COMPUTE_SHADER));
		GL_CHECK(glShaderSource(computeShader, 1, &computeShaderSource, NULL));
		GL_CHECK(glCompileShader(computeShader));
		{
			GLint success;
			GL_CHECK(glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success));
			if (!success)
			{
				GLint infoLength = 0;
				glGetShaderiv(computeShader, GL_INFO_LOG_LENGTH, &infoLength);
				char* infoLog = new char[infoLength];
				GL_CHECK(glGetShaderInfoLog(computeShader, infoLength, &infoLength, infoLog));
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "COMPUTE_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				delete[] infoLog;
				GL_CHECK(glDeleteShader(computeShader));
			}
		}

		gComputeProgram = GL_CHECK_RETURN(glCreateProgram());
		GL_CHECK(glAttachShader(gComputeProgram, computeShader));
		GL_CHECK(glLinkProgram(gComputeProgram));

		GL_CHECK(glDeleteShader(computeShader));
	}

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
	}

	glUseProgram(gComputeProgram);

	glGenBuffers(2, gPosBufs);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gPosBufs[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, gPosBufs[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vertices.size() * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(2, gVelBufs);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, gVelBufs[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, velocities.size() * sizeof(GLfloat), velocities.data(), GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, gVelBufs[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, velocities.size() * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);

	glGenVertexArrays(1, &gVAO);
	glBindVertexArray(gVAO);
	glBindBuffer(GL_ARRAY_BUFFER, gPosBufs[0]);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);

	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);

	glEnable(GL_PROGRAM_POINT_SIZE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glEnable(GL_MULTISAMPLE);
	//glEnable(GL_DEPTH_TEST);

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(PRIM_RESTART);

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

void update()
{
}

void draw()
{
	glUseProgram(gComputeProgram);
	for (int i = 0; i < 1000; i++)
	{
		glDispatchCompute(4, 4, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// Swap buffers
		gReadBuferId = 1 - gReadBuferId;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gPosBufs[gReadBuferId]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, gPosBufs[1 - gReadBuferId]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, gVelBufs[gReadBuferId]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, gVelBufs[1 - gReadBuferId]);
	}

	glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(gProgram);
	glBindVertexArray(gVAO);
	glDrawElements(GL_TRIANGLE_STRIP, gIndiceCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

int main()
{
	return run();
}