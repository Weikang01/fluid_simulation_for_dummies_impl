#pragma once
class GLFWwindow;

class Application
{
public:
	void run();
private:
	void initSystem();
	void mainLoop();
	void terminateSystem();
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	const int SCREEN_WIDTH = 128;
	const int SCREEN_HEIGHT = 128;
	GLFWwindow* window;
};

