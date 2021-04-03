#pragma once
#ifndef VIEW_H
#define VIEW_H

#include "root.h"
#include "util.h"

namespace cube
{
	GLFWwindow* initWindow()
	{
        // glfw: initialize and configure
    // ------------------------------
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
        GLFWwindow* window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "Rubiks Cube", NULL, NULL);
        /*glViewport(0, 0, SRC_WIDTH, SRC_HEIGHT);*/

        if (window == NULL)
        {
            MessageBox(NULL, TEXT("Failed to create GLFW window"), TEXT("ERROR Message"), MB_ICONERROR | MB_OK);
            glfwTerminate();
            std::exit(-1);
        }
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetKeyCallback(window, on_key_callback);

        // tell GLFW to capture our mouse
        // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // glad: load all OpenGL function pointers
        // ---------------------------------------
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            MessageBox(NULL, TEXT("Failed to load GLAD"), TEXT("ERROR Message"), MB_ICONERROR | MB_OK);
            std::exit(-1);
        }

        // configure global opengl state
        // -----------------------------
        glEnable(GL_DEPTH_TEST);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Use for debug
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        return window;
	}
}

#endif // !VIEW_H
