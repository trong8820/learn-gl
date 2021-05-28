// Shadow Volume

#include "macros.h"
#include "entry.h"

#include "vec3.h"
#include "mat4.h"

const float PI = 3.14159265358979f;

// Cube
float vertices[] = {
	// Pos			        // Color
	-0.5f, -0.5f, -0.5f, 	0.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, +0.5f, 	0.0f, 0.0f, 1.0f,
	-0.5f, +0.5f, -0.5f,    0.0f, 1.0f, 0.0f,
	-0.5f, +0.5f, +0.5f,    0.0f, 1.0f, 1.0f,
	+0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f,
	+0.5f, -0.5f, +0.5f,    1.0f, 0.0f, 1.0f,
	+0.5f, +0.5f, -0.5f,    1.0f, 1.0f, 0.0f,
	+0.5f, +0.5f, +0.5f,    1.0f, 1.0f, 1.0f
};

unsigned int indices[] =
{
	0, 2, 1,
	1, 2, 3,
	4, 5, 6,
	5, 7, 6,
	0, 1, 5,
	0, 5, 4,
	2, 6, 7,
	2, 7, 3,
	0, 4, 6,
	0, 6, 2,
	1, 3, 7,
	1, 7, 5
};

const char *vertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

out vec3 vColor;

void main()
{
	vColor = aColor;
	gl_Position = proj * view * world * vec4(aPos, 1.0);
}
)";

const char *fragmentShaderSource = R"(
#version 410 core

in vec3 vColor;

out vec4 FragColor;

void main()
{
	FragColor = vec4(vColor, 1.0);
}
)";

const char *nullVertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

out vec3 vColor;

void main()
{
	vColor = aColor;
	gl_Position = proj * view * world * vec4(aPos, 1.0);
}
)";

const char *nullFragmentShaderSource = R"(
#version 410 core

in vec3 vColor;

out vec4 FragColor;

void main()
{
	//FragColor = vec4(vColor, 1.0);
}
)";

const char *silhouetteVertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 world;

out vec3 vPos;

void main()
{
	vPos = (world * vec4(aPos, 1.0)).xyz;
}
)";

const char *silhouetteGeometryShaderSource = R"(
#version 410 core

layout (triangles_adjacency) in;
layout (triangle_strip, max_vertices = 18) out;

float EPSILON = 0.001;
vec3 lightPos = vec3(-3.0, 5.0, -1.0);

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;
//uniform vec3 lightPos;

in vec3 vPos[];

void EmitQuad(vec3 startVertex, vec3 endVertex)
{
    mat4 vp = proj * view;
    vec3 lightDir = normalize(startVertex - lightPos);   
    gl_Position = vp * vec4((startVertex + lightDir * EPSILON), 1.0);
    EmitVertex();

    //gl_Position = vp * vec4(lightPos, 0.0);
    gl_Position = vp * vec4((startVertex + lightDir * 10.0), 1.0);
    EmitVertex();

    lightDir = normalize(endVertex - lightPos);
    gl_Position = vp * vec4((endVertex + lightDir * EPSILON), 1.0);
    EmitVertex();

    //gl_Position = vp * vec4(lightDir, 0.0);
    gl_Position = vp * vec4((endVertex + lightDir * 10.0), 1.0);
    EmitVertex();

    EndPrimitive();
}

void main()
{
    mat4 vp = proj * view;

	vec3 e1 = vPos[2] - vPos[0];
    vec3 e2 = vPos[4] - vPos[0];
    vec3 e3 = vPos[1] - vPos[0];
    vec3 e4 = vPos[3] - vPos[2];
    vec3 e5 = vPos[4] - vPos[2];
    vec3 e6 = vPos[5] - vPos[0];

	vec3 faceNormal = cross(e1, e2);
    vec3 lightDir = lightPos - vPos[0];
	if (dot(faceNormal, lightDir) > 0.00001) 
	{
		faceNormal = cross(e3, e1);
		if (dot(faceNormal, lightDir) <= 0.0) 
		{
            vec3 startVertex = vPos[0];
            vec3 endVertex = vPos[2];
			EmitQuad(startVertex, endVertex);
		}

		faceNormal = cross(e4, e5);
		lightDir = lightPos - vPos[2];
		if (dot(faceNormal, lightDir) <= 0.0) 
		{
			vec3 startVertex = vPos[2];
            vec3 endVertex = vPos[4];
			EmitQuad(startVertex, endVertex);
		}

		faceNormal = cross(e2, e6);
		lightDir = lightPos - vPos[4];
		if (dot(faceNormal, lightDir) <= 0.0) 
		{
			vec3 startVertex = vPos[4];
            vec3 endVertex = vPos[0];
			EmitQuad(startVertex, endVertex);
		}

        // render the front cap
        lightDir = (normalize(vPos[0] - lightPos));
        gl_Position = vp * vec4((vPos[0] + lightDir * EPSILON), 1.0);
        EmitVertex();

        lightDir = (normalize(vPos[2] - lightPos));
        gl_Position = vp * vec4((vPos[2] + lightDir * EPSILON), 1.0);
        EmitVertex();

        lightDir = (normalize(vPos[4] - lightPos));
        gl_Position = vp * vec4((vPos[4] + lightDir * EPSILON), 1.0);
        EmitVertex();
        EndPrimitive();

        // render the back cap
        lightDir = (normalize(vPos[0] - lightPos));
        gl_Position = vp * vec4((vPos[0] + lightDir * 10.0), 1.0);
        EmitVertex();

		lightDir = (normalize(vPos[4] - lightPos));
        gl_Position = vp * vec4((vPos[4] + lightDir * 10.0), 1.0);
        EmitVertex();

        lightDir = (normalize(vPos[2] - lightPos));
        gl_Position = vp * vec4((vPos[2] + lightDir * 10.0), 1.0);
        EmitVertex();
        EndPrimitive();
	}
}
)";

const char *silhouetteFragmentShaderSource = R"(
#version 410 core

out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";

GLuint gProgram;
GLuint gNullProgram;
GLuint gSilhouetteProgram;

GLuint gVAO;

GLint gWorldLoc;
GLint gNullWorldLoc;
GLint gSilhouetteWorldLoc;
float gAngle = 0.0f;

auto init() -> bool
{
	unsigned int indices_adj[72];
	size_t id = 0;
	for (size_t i = 0; i < 12; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			unsigned int id0 = indices[i*3 + j];
			unsigned int id1 = indices[i*3 + (j + 1)%3];

			indices_adj[id++] = id0;
			for (size_t k = 0; k < 12; k++)
			{
				if (k == i) continue;
				unsigned int k0 = indices[k*3 + 0];
				unsigned int k1 = indices[k*3 + 1];
				unsigned int k2 = indices[k*3 + 2];
				
				if ((k0 == id0 && k1 == id1) || (k1 == id0 && k0 == id1))
				{
					indices_adj[id++] = k2;
					break;
				}

				if ((k1 == id0 && k2 == id1) || (k2 == id0 && k1 == id1))
				{
					indices_adj[id++] = k0;
					break;
				}

				if ((k2 == id0 && k0 == id1) || (k0 == id0 && k2 == id1))
				{
					indices_adj[id++] = k1;
					break;
				}
			}
		}
	}

	//std::cout << "init " << gWidth << " " << gHeight << std::endl;
	{
		auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
		GL_CHECK(glShaderSource(vertexShader, 1, &vertexShaderSource, NULL));
		GL_CHECK(glCompileShader(vertexShader));

		auto fragmentShader = GL_CHECK_RETURN(glCreateShader(GL_FRAGMENT_SHADER));
		GL_CHECK(glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL));
		GL_CHECK(glCompileShader(fragmentShader));

		gProgram = GL_CHECK_RETURN(glCreateProgram());
		GL_CHECK(glAttachShader(gProgram, vertexShader));
		GL_CHECK(glAttachShader(gProgram, fragmentShader));
		GL_CHECK(glLinkProgram(gProgram));

		GL_CHECK(glDeleteShader(vertexShader));
		GL_CHECK(glDeleteShader(fragmentShader));
	}

    {
		auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
		GL_CHECK(glShaderSource(vertexShader, 1, &nullVertexShaderSource, NULL));
		GL_CHECK(glCompileShader(vertexShader));

		auto fragmentShader = GL_CHECK_RETURN(glCreateShader(GL_FRAGMENT_SHADER));
		GL_CHECK(glShaderSource(fragmentShader, 1, &nullFragmentShaderSource, NULL));
		GL_CHECK(glCompileShader(fragmentShader));

		gNullProgram = GL_CHECK_RETURN(glCreateProgram());
		GL_CHECK(glAttachShader(gNullProgram, vertexShader));
		GL_CHECK(glAttachShader(gNullProgram, fragmentShader));
		GL_CHECK(glLinkProgram(gNullProgram));

		GL_CHECK(glDeleteShader(vertexShader));
		GL_CHECK(glDeleteShader(fragmentShader));
	}

	{
		auto vertexShader = GL_CHECK_RETURN(glCreateShader(GL_VERTEX_SHADER));
		GL_CHECK(glShaderSource(vertexShader, 1, &silhouetteVertexShaderSource, NULL));
		GL_CHECK(glCompileShader(vertexShader));

		auto geometryShader = GL_CHECK_RETURN(glCreateShader(GL_GEOMETRY_SHADER));
		GL_CHECK(glShaderSource(geometryShader, 1, &silhouetteGeometryShaderSource, NULL));
		GL_CHECK(glCompileShader(geometryShader));

		auto fragmentShader = GL_CHECK_RETURN(glCreateShader(GL_FRAGMENT_SHADER));
		GL_CHECK(glShaderSource(fragmentShader, 1, &silhouetteFragmentShaderSource, NULL));
		GL_CHECK(glCompileShader(fragmentShader));

		gSilhouetteProgram = GL_CHECK_RETURN(glCreateProgram());
		GL_CHECK(glAttachShader(gSilhouetteProgram, vertexShader));
		GL_CHECK(glAttachShader(gSilhouetteProgram, geometryShader));
		GL_CHECK(glAttachShader(gSilhouetteProgram, fragmentShader));
		GL_CHECK(glLinkProgram(gSilhouetteProgram));

		GL_CHECK(glDeleteShader(vertexShader));
		GL_CHECK(glDeleteShader(fragmentShader));
	}

	glGenVertexArrays(1, &gVAO);
	glBindVertexArray(gVAO);
		GLuint VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);

				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

		GLuint EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_adj), indices_adj, GL_STATIC_DRAW);

	glUseProgram(gProgram);
	gWorldLoc = glGetUniformLocation(gProgram, "world");

    glUseProgram(gNullProgram);
	gNullWorldLoc = glGetUniformLocation(gNullProgram, "world");

	glUseProgram(gSilhouetteProgram);
	gSilhouetteWorldLoc = glGetUniformLocation(gSilhouetteProgram, "world");

	on_size();

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
    glDepthFunc(GL_LEQUAL);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	return 0;
}

void on_size()
{
	//std::cout << "size " << gWidth << " " << gHeight << std::endl;
	glViewport(0, 0, gWidth, gHeight);

	mat4 view = mat4::lookAt(vec3(0.0f, 3.0, 3.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 proj = mat4::perspective(45.0f * (PI/180.0f), static_cast<float>(gWidth)/gHeight, 0.1f, 100.0f);

	{
		glUseProgram(gProgram);
		GLint viewLoc = glGetUniformLocation(gProgram, "view");
		GLint projLoc = glGetUniformLocation(gProgram, "proj");
		glUniformMatrix4fv(viewLoc, 1, false, view.m);
		glUniformMatrix4fv(projLoc, 1, false, proj.m);
	}

    {
		glUseProgram(gNullProgram);
		GLint viewLoc = glGetUniformLocation(gNullProgram, "view");
		GLint projLoc = glGetUniformLocation(gNullProgram, "proj");
		glUniformMatrix4fv(viewLoc, 1, false, view.m);
		glUniformMatrix4fv(projLoc, 1, false, proj.m);
	}

	{
		glUseProgram(gSilhouetteProgram);
		GLint viewLoc = glGetUniformLocation(gSilhouetteProgram, "view");
		GLint projLoc = glGetUniformLocation(gSilhouetteProgram, "proj");
		glUniformMatrix4fv(viewLoc, 1, false, view.m);
		glUniformMatrix4fv(projLoc, 1, false, proj.m);
	}
}

void on_key(int key, int action)
{

}

auto update() -> void
{
	gAngle++;
}

auto draw() -> void
{
	mat4 world1 = mat4::rotate(0.0f, 1.0f, 0.0f, gAngle * (PI/180.0f)) * mat4::translate(0.0f, 0.5f, 0.0f);
	mat4 world2 = mat4::scale(6.0f, 0.1f, 6.0f) * mat4::translate(0.0f, -0.5f, 0.0f);

	glViewport(0, 0, gWidth, gHeight);
    glDepthMask(GL_TRUE);
	glStencilMask(0xFF);
	glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0x00);

	/*
    // pass 1
    glDrawBuffer(GL_NONE);
	glUseProgram(gNullProgram);
	glBindVertexArray(gVAO);

	glUniformMatrix4fv(gNullWorldLoc, 1, false, world1.m);
	glDrawElements(GL_TRIANGLES_ADJACENCY, 72, GL_UNSIGNED_INT, 0);

    // pass 2
	//glDrawBuffer(GL_NONE);
    glDepthMask(GL_FALSE);

	glCullFace(GL_FRONT_AND_BACK); 

    //glStencilFunc(GL_ALWAYS, 0, 0xFF);
	//glStencilMask(0xFF);
    glStencilMask(0xFF);
	glStencilFunc(GL_NEVER, 1, 0xFF);
    //glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
    //glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

	glUseProgram(gSilhouetteProgram);
	glBindVertexArray(gVAO);

	glUniformMatrix4fv(gSilhouetteWorldLoc, 1, false, world1.m);
	glDrawElements(GL_TRIANGLES_ADJACENCY, 72, GL_UNSIGNED_INT, 0);

    // pass 3
    glDrawBuffer(GL_BACK);
    glStencilFunc(GL_EQUAL, 0x0, 0xFF);
	glStencilMask(0x00);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glUseProgram(gProgram);
	glBindVertexArray(gVAO);

	glUniformMatrix4fv(gWorldLoc, 1, false, world1.m);
	glDrawElements(GL_TRIANGLES_ADJACENCY, 72, GL_UNSIGNED_INT, 0);

    // pass 4
    glUseProgram(gProgram);
	glBindVertexArray(gVAO);

	glUniformMatrix4fv(gWorldLoc, 1, false, world1.m);
	glDrawElements(GL_TRIANGLES_ADJACENCY, 72, GL_UNSIGNED_INT, 0);
	*/

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_TRUE);
	glUseProgram(gNullProgram);
	glBindVertexArray(gVAO);

	glUniformMatrix4fv(gNullWorldLoc, 1, false, world1.m);
	glDrawElements(GL_TRIANGLES_ADJACENCY, 72, GL_UNSIGNED_INT, 0);
	glUniformMatrix4fv(gNullWorldLoc, 1, false, world2.m);
	glDrawElements(GL_TRIANGLES_ADJACENCY, 72, GL_UNSIGNED_INT, 0);
	
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);

	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

	glUseProgram(gSilhouetteProgram);
	glBindVertexArray(gVAO);

	glUniformMatrix4fv(gSilhouetteWorldLoc, 1, false, world1.m);
	glDrawElements(GL_TRIANGLES_ADJACENCY, 72, GL_UNSIGNED_INT, 0);
	glUniformMatrix4fv(gSilhouetteWorldLoc, 1, false, world2.m);
	glDrawElements(GL_TRIANGLES_ADJACENCY, 72, GL_UNSIGNED_INT, 0);
	glStencilMask(0x00);
	
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);

	glStencilFunc(GL_EQUAL, 0x0, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glUseProgram(gProgram);
	glBindVertexArray(gVAO);

	glUniformMatrix4fv(gWorldLoc, 1, false, world1.m);
	glDrawElements(GL_TRIANGLES_ADJACENCY, 72, GL_UNSIGNED_INT, 0);
	glUniformMatrix4fv(gWorldLoc, 1, false, world2.m);
	glDrawElements(GL_TRIANGLES_ADJACENCY, 72, GL_UNSIGNED_INT, 0);
}

auto main() -> int
{
	return run();
}