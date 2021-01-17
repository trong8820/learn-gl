#include "entry.h"

auto run() -> int
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

	glfwMakeContextCurrent(pWindow);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return EXIT_FAILURE;

	int width{ 0 };
	int height{ 0 };
	glfwGetWindowSize(pWindow, &width, &height);
	//float xscale{ 1.0f };
	//float yscale{ 1.0f };
	//glfwGetWindowContentScale(m_pWindow, &xscale, &yscale);

	init();

	glfwSwapInterval(1);
	while (glfwWindowShouldClose(pWindow) == GLFW_FALSE)
	{
		update();
		draw();

		glfwSwapBuffers(pWindow);
		glfwPollEvents();
	}

	glfwDestroyWindow(pWindow);
	glfwTerminate();
	return EXIT_SUCCESS;
}
