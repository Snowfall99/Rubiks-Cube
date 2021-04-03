#pragma once
#ifndef ROOT_H
#define ROOT_H

#include <Windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"

#include <vector>
#include <queue>
#include <mutex>
#include <random>
#include <string>
#include <map>
#include <stack>
#include <iostream>

std::random_device rd;
std::mt19937 mt(rd());

namespace cube 
{
    // Using State machine to present cube state
    enum State
    {
        STOP,
        ROTATE_R,
        ROTATE_L,
        ROTATE_U,
        ROTATE_D,
        ROTATE_F,
        ROTATE_B,
    };

    State state = STOP;
    State nextState = STOP;

    // store rotate state
    std::queue<State> rotate_queue;
    std::stack<State> used_rotate_stack;
    typedef std::vector<State> move_seq_t;

    // three modes
    enum Mode
    {
        Random,
        Read_File,
        Default,
    };
    Mode mode = Default;

    // Init sign
    bool Init = false;

    // settings
    const unsigned int SRC_WIDTH = GetSystemMetrics(SM_CXSCREEN);
    const unsigned int SRC_HEIGHT = GetSystemMetrics(SM_CYSCREEN);

    // camera
    Camera camera(glm::vec3(5.0f, 5.0f, 5.0f));
    float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
    float pitch = 0.0f;
    float fov = 45.0f;
    float lastX = (float)SRC_WIDTH / 2.0;
    float lastY = (float)SRC_HEIGHT / 2.0;
    bool firstMouse = true;

    // rotate angle
    float deltaX = 0.0f;
    float deltaY = 0.0f;
    float deltaZ = 0.0f;

    float angle = 0;
    float targetangle = 90;
    glm::vec3 axisVec;
    std::mutex axismutex;

    // Model state matrix
    glm::mat4 currentModel = glm::mat4(1.0f);
    std::vector<glm::mat4> allMats(27, glm::mat4(1.0f));
    glm::mat4 world = glm::mat4(1.0f);

    // time
    float deltaTime = 0.0f;	// time between current frame and last frame
    float lastFrame = 0.0f;

    // lighting
    glm::vec3 lightPos(2.4f, 2.4f, 2.4f);
    bool lightMov = false;
    bool lightOn = true;

    // material
    enum Material
    {
        Emerald,
        Jade,
        Pearl,
        White_plastic,
        White_rubber,
        Obsidian,
        Ruby,
        Turquoise,
        Silver
    };
    Material material = Pearl;

    // Window Handler
    HWND hwndGL;
    WNDPROC OldProc;
}

#endif // !ROOT_H
