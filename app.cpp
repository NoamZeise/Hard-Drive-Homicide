#include "app.h"


App::App(int windowWidth, int windowHeight)
{
	//set member variables
	mWindowWidth = windowWidth;
	mWindowHeight = windowHeight;
	//init glfw window
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //using vulkan not openGL
	mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, "Vulkan App", nullptr, nullptr);
	glfwSetWindowUserPointer(mWindow, this);
	glfwSetFramebufferSizeCallback(mWindow, framebuffer_size_callback);
	glfwSetCursorPosCallback(mWindow, mouse_callback);
	glfwSetScrollCallback(mWindow, scroll_callback);
	glfwSetKeyCallback(mWindow, key_callback);
	glfwSetMouseButtonCallback(mWindow, mouse_button_callback);
	mRender = new Render(mWindow);
	loadAssets();
}


App::~App()
{
	delete mRender;
	delete roboto;
	delete robotbold;
	//cleanup glfw
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

void App::loadAssets()
{
	//TODO load assets
	roboto = mRender->loadFont("Roboto-Regular.ttf");
	robotbold = mRender->loadFont("Roboto-Bold.ttf");
	mRender->endTextureLoad();
}

void App::run()
{
	while (!glfwWindowShouldClose(mWindow))
	{
		update();
		draw();
	}
}

void App::resize(int windowWidth, int windowHeight)
{
	mWindowWidth = windowWidth;
	mWindowHeight = windowHeight;
	mRender->framebufferResized = true;
}

void App::update()
{
	glfwPollEvents();
}

void App::draw()
{
	mRender->startDraw();

	//todo draw app
	mRender->DrawString(roboto, "testing text rendering", glm::vec2(100, 100), 50, 0, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
	mRender->DrawString(robotbold, "testing bold rendering", glm::vec2(100, 300), 50, 0, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

	mRender->endDraw();
}

	
#pragma region GLFW_CALLBACKS


void App::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	app->resize(width, height);
}

void App::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	app->input.X = xpos;
	app->input.Y = ypos;
}
void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	app->input.offset = yoffset;
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	if (key == GLFW_KEY_F && action == GLFW_RELEASE)
	{
		if (glfwGetWindowMonitor(window) == nullptr)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		else
		{
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(window, NULL, 100, 100, app->mWindowWidth, app->mWindowHeight, mode->refreshRate);
		}
	}
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			app->input.Keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			app->input.Keys[key] = false;
		}
	}
}

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));

	if (button >= 0 && button < 8)
	{
		if (action == GLFW_PRESS)
		{
			app->input.Buttons[button] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			app->input.Buttons[button] = true;
		}
	}
}
#pragma endregion