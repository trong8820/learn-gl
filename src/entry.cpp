#include "entry.h"

void window_size_callback(GLFWwindow* window, int width, int height)
{
	gWidth = width;
	gHeight = height;
	on_size();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	on_key(key, action);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	on_mouse(xpos, ypos);
}

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

	glfwWindowHint(GLFW_SAMPLES, 4);

	g_pWindow = glfwCreateWindow(800, 600, "Learn GL", nullptr, nullptr);
	if (g_pWindow == nullptr) return EXIT_FAILURE;

	glfwMakeContextCurrent(g_pWindow);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return EXIT_FAILURE;

	glfwGetWindowSize(g_pWindow, &gWidth, &gHeight);
	//float xscale{ 1.0f };
	//float yscale{ 1.0f };
	//glfwGetWindowContentScale(m_pWindow, &xscale, &yscale);

	// Version
	const GLubyte *name     =  glGetString(GL_VENDOR);
	const GLubyte *renderer =  glGetString(GL_RENDERER);
	const GLubyte *version  =  glGetString(GL_VERSION);
	std::cout << "Vendor: " << name << std::endl;
	std::cout << "Renderer: " << renderer << std::endl;
	std::cout << "Version: " << version << std::endl;

	// Check exts
	GLint numExts;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExts);
	for (GLint i = 0; i < numExts; i++)
	{
		const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
		std::cout << "Ext: " << extension << std::endl;
	}

	init();
	auto prevWindowSizeCallback = glfwSetWindowSizeCallback(g_pWindow, window_size_callback);
	auto prevKeyCallback = glfwSetKeyCallback(g_pWindow, key_callback);
	auto prevMouseCallback = glfwSetCursorPosCallback(g_pWindow, mouse_callback);

	glfwSwapInterval(1);
	while (glfwWindowShouldClose(g_pWindow) == GLFW_FALSE)
	{
		update();
		draw();

		glfwSwapBuffers(g_pWindow);
		glfwPollEvents();
	}

	glfwSetKeyCallback(g_pWindow, prevKeyCallback);
	glfwSetWindowSizeCallback(g_pWindow, prevWindowSizeCallback);
	glfwSetCursorPosCallback(g_pWindow, prevMouseCallback);

	glfwDestroyWindow(g_pWindow);
	glfwTerminate();
	return EXIT_SUCCESS;
}
