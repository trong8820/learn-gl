#include <iostream>
#include <thread>
#include <chrono>

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

float vertices[] = {
	// Pos
	0.0f,  0.5f,
	0.5f, -0.5f,
	-0.5f, -0.5f,
	-0.5f,  0.5f,
};

unsigned int indices[] = {  // note that we start from 0!
	0, 1, 3,  // first Triangle
	1, 2, 3   // second Triangle
};

const char* vertexShaderSource = R"(
#version 410 core

layout (location = 0) in vec2 aPos;

void main()
{
	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 410 core

out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";

GLuint gProgram;
GLuint gVAO;
GLuint gVBO;
GLFWwindow* g_pWindowSlave;
std::thread gTask;

GLsync gMainFence;
GLsync gSndFence;

bool gThreadShouldRun;

auto main() -> int
{
	if (glfwInit() == GLFW_FALSE) return EXIT_FAILURE;

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
#ifdef __APPLE__
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#endif

#if defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwSetErrorCallback([](int error, const char* description)
		{
			std::cout << "GLFW Error: " << error << " - " << description << "\n";
		});
#endif

	auto pWindow = glfwCreateWindow(800, 600, "Example 01", nullptr, nullptr);
	if (pWindow == nullptr) return EXIT_FAILURE;

	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	g_pWindowSlave = glfwCreateWindow(800, 600, "", nullptr, pWindow);
	if (g_pWindowSlave == nullptr) return EXIT_FAILURE;

	glfwMakeContextCurrent(pWindow);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return EXIT_FAILURE;

	int width{ 0 };
	int height{ 0 };
	glfwGetWindowSize(pWindow, &width, &height);
	//float xscale{ 1.0f };
	//float yscale{ 1.0f };
	//glfwGetWindowContentScale(m_pWindow, &xscale, &yscale);

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

	glGenVertexArrays(1, &gVAO);
	glBindVertexArray(gVAO);
		glGenBuffers(1, &gVBO);
		glBindBuffer(GL_ARRAY_BUFFER, gVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);

			GLuint EBO;
			glGenBuffers(1, &EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	gTask = std::thread([]()
		{
			glfwMakeContextCurrent(g_pWindowSlave);
			gThreadShouldRun = true;
			while (gThreadShouldRun)
			{

				//std::this_thread::sleep_for(std::chrono::seconds(2L));
				std::this_thread::sleep_for(std::chrono::milliseconds(16L));
				//std::cout << "Chay  thread" << std::endl;


				//std::cout << "MainFence: " << gMainFence << std::endl;
				//glWaitSync(gMainFence, 0, GL_TIMEOUT_IGNORED);
				auto status = glClientWaitSync(gMainFence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
				if (status == GL_ALREADY_SIGNALED || status == GL_CONDITION_SATISFIED)
				{
					glDeleteSync(gMainFence);
					gMainFence = 0;

					vertices[0] += 0.001f;

					if (gSndFence == 0)
					{
						glBindBuffer(GL_ARRAY_BUFFER, gVBO);
						glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

						gSndFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
					}

				}
			}
		});

	glfwSwapInterval(1);
	while (glfwWindowShouldClose(pWindow) == GLFW_FALSE)
	{
		glWaitSync(gSndFence, 0, GL_TIMEOUT_IGNORED);
		glDeleteSync(gSndFence);
		gSndFence = 0;

		glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(gProgram);
		glBindVertexArray(gVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		if (gMainFence == 0)
		{
			gMainFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		}

		glfwSwapBuffers(pWindow);
		glfwPollEvents();
	}

	glfwDestroyWindow(pWindow);
	glfwDestroyWindow(g_pWindowSlave);
	glfwTerminate();

	gThreadShouldRun = false;
	gTask.join();
	return EXIT_SUCCESS;
}
