#pragma once
#ifndef UTIL_H
#define UTIL_H

#include "root.h"

namespace cube
{
    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    void on_key_callback(GLFWwindow*, int, int, int, int);
    void processInput(GLFWwindow* window);

    move_seq_t readStateFromFile();

    // process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
    // ---------------------------------------------------------------------------------------------------------
    void processInput(GLFWwindow* window)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        float cameraSpeed = 2.5 * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            camera.Position += cameraSpeed * camera.Front;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            camera.Position -= cameraSpeed * camera.Front;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            camera.Position -= glm::normalize(glm::cross(camera.Front, camera.Up)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            camera.Position += glm::normalize(glm::cross(camera.Front, camera.Up)) * cameraSpeed;

        deltaY = 0;
        deltaX = 0;
        deltaZ = 0;
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        {
            deltaX -= 0.015f;
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        {
            deltaX += 0.015f;
        }
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        {
            deltaY -= 0.015f;
        }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        {
            deltaY += 0.015f;
        }
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
        {
            deltaZ -= 0.015f;
        }
        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
        {
            deltaZ += 0.015f;
        }
        // Reset position
        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
        {
            currentModel = glm::mat4(1.0f);
        }
    }

    void on_key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
    {
        if (key == GLFW_KEY_R && action == GLFW_PRESS)
        {
            //std::lock_guard<std::mutex> lock(axismutex);
            rotate_queue.push(ROTATE_R);
        }
        if (key == GLFW_KEY_L && action == GLFW_PRESS)
        {
            //lock_guard<mutex> lock(axismutex);
            rotate_queue.push(ROTATE_L);
        }
        if (key == GLFW_KEY_U && action == GLFW_PRESS)
        {
            //lock_guard<mutex> lock(axismutex);
            rotate_queue.push(ROTATE_U);
        }
        if (key == GLFW_KEY_D && action == GLFW_PRESS)
        {
            //lock_guard<mutex> lock(axismutex);
            rotate_queue.push(ROTATE_D);
        }
        if (key == GLFW_KEY_F && action == GLFW_PRESS)
        {
            //lock_guard<mutex> lock(axismutex);
            rotate_queue.push(ROTATE_F);
        }
        if (key == GLFW_KEY_B && action == GLFW_PRESS)
        {
            //lock_guard<mutex> lock(axismutex);
            rotate_queue.push(ROTATE_B);
        }
    }


    // glfw: whenever the window size changed (by OS or user resize) this callback function executes
    // ---------------------------------------------------------------------------------------------
    void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        // make sure the viewport matches the new window dimensions; note that width and 
        // height will be significantly larger than specified on retina displays.
        glViewport(0, 0, width, height);
    }

    // glfw: whenever the mouse moves, this callback is called
    // -------------------------------------------------------
    void mouse_callback(GLFWwindow* window, double xpos, double ypos)
    {
        if (!glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
            return;

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.1f; // change this value to your liking
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        camera.Front = glm::normalize(front);
    }


    // glfw: whenever the mouse scroll wheel scrolls, this callback is called
    // ----------------------------------------------------------------------
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        camera.ProcessMouseScroll(yoffset);
    }

    // Read state from a file
    move_seq_t readStateFromFile()
    {
        // set filename
        const char* filename = "state.txt";
        // read from file
        std::ifstream myFile(filename);
        if (!myFile.is_open())
        {
            MessageBox(NULL, TEXT("Unable to open file <state.txt>"), TEXT("ERROR Message"), MB_ICONERROR | MB_OK);
            std::exit(-1);
        }
        std::string line;
        getline(myFile, line);
        myFile.close();

        // add state to rotate state
        move_seq_t move_seq;
        int i;
        for (i = 0; i < line.length(); i++)
        {
            if (line[i] == 'U')
                move_seq.push_back(ROTATE_U);
            else if (line[i] == 'D')
                move_seq.push_back(ROTATE_D);
            else if (line[i] == 'R')
                move_seq.push_back(ROTATE_R);
            else if (line[i] == 'L')
                move_seq.push_back(ROTATE_L);
            else if (line[i] == 'F')
                move_seq.push_back(ROTATE_F);
            else if (line[i] == 'B')
                move_seq.push_back(ROTATE_B);
        }

        return move_seq;
    }

    // generate a sequence of state randomly
    move_seq_t randomState(int steps = 15)
    {
        std::uniform_int_distribution<int> gen(0, 5);

        move_seq_t rs;
        for (int i = 0; i < steps; i++)
        {
            int c1 = gen(mt);
            if (c1 == 0)
                rs.push_back(State::ROTATE_U);
            else if (c1 == 1)
                rs.push_back(State::ROTATE_D);
            else if (c1 == 2)
                rs.push_back(State::ROTATE_R);
            else if (c1 == 3)
                rs.push_back(State::ROTATE_L);
            else if (c1 == 4)
                rs.push_back(State::ROTATE_F);
            else if (c1 == 5)
                rs.push_back(State::ROTATE_B);
        }
        return rs;
    }

    LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        move_seq_t rs;
        switch (message)
        {
        case WM_COMMAND:
            switch (wParam)
            {
            case 1:
                rs = randomState();
                for (auto& step : rs)
                {
                    rotate_queue.push(step);
                }
                break;
            case 2:
                rs = readStateFromFile();
                for (auto& step : rs)
                {
                    rotate_queue.push(step);
                }
                break;
            case 3:
                lightMov = !lightMov;
                break;
                // Change material
            case 11:
                material = Pearl;
                break;
            case 12:
                material = White_plastic;
                break;
            case 13:
                material = White_rubber;
                break;
            case 14:
                material = Emerald;
                break;
            case 15:
                material = Jade;
                break;
            case 16:
                material = Obsidian;
                break;
            case 17:
                material = Ruby;
                break;
            case 18:
                material = Turquoise;
                break;
            case 19:
                material = Silver;
                break;
            case 21:
                lightOn = !lightOn;
                break;
            }
            return 0;
        }
        return CallWindowProc(OldProc, hwnd, message, wParam, lParam);
    }
}

#endif // !UTIL_H
