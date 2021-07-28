#include "Application.h"
#include <GLFW/glfw3.h>
#include <glew.h>


void Application::run()
{
    initSystem();
    mainLoop();
    terminateSystem();
}

void Application::initSystem()
{
    if (!glfwInit())
        return;

    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Fluid Simulation", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);

    if (!glewInit())
        return;

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}

void Application::mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);



        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Application::terminateSystem()
{
    glfwTerminate();
}

void Application::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

