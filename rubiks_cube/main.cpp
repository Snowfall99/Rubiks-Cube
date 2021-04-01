#include <Windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_s.h"
#include "camera.h"

#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <random>
#include <queue>

using namespace std;

std::random_device rd;
std::mt19937 mt(rd());

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void on_key_callback(GLFWwindow*, int, int, int, int);
void processInput(GLFWwindow* window);
unsigned int loadCubemap(std::vector<std::string> faces);

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
    UPDATE,
};

State state = STOP;
State nextState = STOP;
// store next state
std::queue<State> rotate_queue;

// store random state 
typedef std::vector<State> move_seq_t;
move_seq_t random_state;

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

HWND hwndGL;
WNDPROC OldProc;

LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        switch (wParam)
        {
        case 1:
            move_seq_t rs = randomState();
            for (auto& step : rs)
            {
                rotate_queue.push(step);
            }
            break;
        }
        return 0;
    }
    return CallWindowProc(OldProc, hwnd, message, wParam, lParam);
}

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
mutex axismutex;

// Model state matrix
glm::mat4 currentModel = glm::mat4(1.0f);
vector<glm::mat4> allMats(27, glm::mat4(1.0f));
glm::mat4 world = glm::mat4(1.0f);

// time
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main()
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
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
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
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Menu
    hwndGL = GetActiveWindow();
    ShowWindow(hwndGL, SW_MAXIMIZE);
    HMENU hMenu;
    hMenu = CreateMenu();

    OldProc = (WNDPROC)SetWindowLong(hwndGL, GWL_WNDPROC, (LONG)WndProc);

    AppendMenu(hMenu, MF_STRING, 1, TEXT("Random State"));
    AppendMenu(hMenu, MF_STRING, 2, TEXT("Reset State"));
    SetMenu(hwndGL, hMenu);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader cubeShader("cube.vs", "cube.fs");
    Shader skyboxShader("skybox.vs", "skybox.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float blockVertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,

    };

    float color[] = {
        // Red
        255.0f, 0.0f, 0.0f,
        255.0f, 0.0f, 0.0f,
        255.0f, 0.0f, 0.0f,
        255.0f, 0.0f, 0.0f,
        255.0f, 0.0f, 0.0f,
        255.0f, 0.0f, 0.0f,

        // Orange
        255.0f, 0.5f, 0.0f,
        255.0f, 0.5f, 0.0f,
        255.0f, 0.5f, 0.0f,
        255.0f, 0.5f, 0.0f,
        255.0f, 0.5f, 0.0f,
        255.0f, 0.5f, 0.0f,

        // Green
        0.0f, 255.0f, 0.0f,
        0.0f, 255.0f, 0.0f,
        0.0f, 255.0f, 0.0f,
        0.0f, 255.0f, 0.0f,
        0.0f, 255.0f, 0.0f,
        0.0f, 255.0f, 0.0f,

        // Blue
        0.0f, 0.0f, 255.0f,
        0.0f, 0.0f, 255.0f,
        0.0f, 0.0f, 255.0f,
        0.0f, 0.0f, 255.0f,
        0.0f, 0.0f, 255.0f,
        0.0f, 0.0f, 255.0f,

        // White
        255.0f, 255.0f, 255.0f,
        255.0f, 255.0f, 255.0f,
        255.0f, 255.0f, 255.0f,
        255.0f, 255.0f, 255.0f,
        255.0f, 255.0f, 255.0f,
        255.0f, 255.0f, 255.0f,

        // Yellow
        255.0f, 255.0f, 0.0f,
        255.0f, 255.0f, 0.0f,
        255.0f, 255.0f, 0.0f,
        255.0f, 255.0f, 0.0f,
        255.0f, 255.0f, 0.0f,
        255.0f, 255.0f, 0.0f,
    };

    // blocks' world space positions
    glm::vec3 blockPositions[] = {
        glm::vec3( 0.0f,  0.0f, -1.0f),
        glm::vec3( 1.0f,  0.0f, -1.0f),
        glm::vec3( 0.0f,  1.0f, -1.0f),
        glm::vec3(-1.0f,  0.0f, -1.0f),
        glm::vec3( 0.0f, -1.0f, -1.0f),
        glm::vec3( 1.0f,  1.0f, -1.0f),
        glm::vec3(-1.0f,  1.0f, -1.0f),
        glm::vec3(-1.0f, -1.0f, -1.0f),
        glm::vec3( 1.0f, -1.0f, -1.0f),
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 1.0f,  0.0f,  0.0f),
        glm::vec3( 0.0f,  1.0f,  0.0f),
        glm::vec3(-1.0f,  0.0f,  0.0f),
        glm::vec3( 0.0f, -1.0f,  0.0f),
        glm::vec3( 1.0f,  1.0f,  0.0f),
        glm::vec3(-1.0f,  1.0f,  0.0f),
        glm::vec3(-1.0f, -1.0f,  0.0f),
        glm::vec3( 1.0f, -1.0f,  0.0f),
        glm::vec3( 0.0f,  0.0f,  1.0f),
        glm::vec3( 1.0f,  0.0f,  1.0f),
        glm::vec3( 0.0f,  1.0f,  1.0f),
        glm::vec3(-1.0f,  0.0f,  1.0f),
        glm::vec3( 0.0f, -1.0f,  1.0f),
        glm::vec3( 1.0f,  1.0f,  1.0f),
        glm::vec3(-1.0f,  1.0f,  1.0f),
        glm::vec3(-1.0f, -1.0f,  1.0f),
        glm::vec3( 1.0f, -1.0f,  1.0f)
    };

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    std::vector<std::string> faces
    {
        "resources/textures/skybox/right.jpg",
        "resources/textures/skybox/left.jpg",
        "resources/textures/skybox/top.jpg",
        "resources/textures/skybox/bottom.jpg",
        "resources/textures/skybox/front.jpg",
        "resources/textures/skybox/back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    // cube VAO
    unsigned int cubeVBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(blockVertices), blockVertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    unsigned int colorbuffer;
    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // Shader configuration
    cubeShader.use();

    // pass projection matrix to shader (as projection matrix rarely changes there's no need to do this per frame)
    // -----------------------------------------------------------------------------------------------------------
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SRC_WIDTH / (float)SRC_HEIGHT, 0.1f, 100.0f);
    cubeShader.setMat4("projection", projection);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input 
        processInput(window);
        
        // Render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // activate shader
        cubeShader.use();
        // camera/view transformation
        glm::mat4 view = glm::lookAt(camera.Position, glm::vec3(0.f, 0.f, 0.f), camera.Up);
        cubeShader.setMat4("view", view);
        
        // calculate the model matrix for each object and pass it to shader before drawing
        currentModel = glm::rotate(currentModel, glm::radians(deltaX), glm::vec3(1.0f, 0.f, 0.f));
        currentModel = glm::rotate(currentModel, glm::radians(deltaY), glm::vec3(0.0f, 1.f, 0.f));
        currentModel = glm::rotate(currentModel, glm::radians(deltaZ), glm::vec3(0.0f, 0.f, 1.f));

        glm::mat4 newModel;

        switch (state)
        {
        case STOP:
        {
            for (int i = 0; i < 27; i++)
            {
                newModel = glm::translate(currentModel * allMats[i], blockPositions[i]);
                cubeShader.setMat4("model", newModel);
                glBindVertexArray(cubeVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }
            if (!rotate_queue.empty())
            {
                nextState = rotate_queue.front();
                rotate_queue.pop();
            }


            if (nextState == ROTATE_L)
            {
                state = ROTATE_L;
                angle = 0;
                axisVec = glm::vec3(1.0f, 0.0f, 0.0f);
            }
            else if (nextState == ROTATE_D)
            {
                state = ROTATE_D;
                angle = 0;
                axisVec = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            else if (nextState == ROTATE_B)
            {
                state = ROTATE_B;
                angle = 0;
                axisVec = glm::vec3(0.0f, 0.0f, 1.0f);
            }
            else if (nextState == ROTATE_R)
            {
                state = ROTATE_R;
                angle = 0;
                axisVec = glm::vec3(1.0f, 0.0f, 0.0f);
            }
            else if (nextState == ROTATE_U)
            {
                state = ROTATE_U;
                angle = 0;
                axisVec = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            else if (nextState == ROTATE_F)
            {
                state = ROTATE_F;
                angle = 0;
                axisVec = glm::vec3(0.0f, 0.0f, 1.0f);
            }
            break;
        }
        case ROTATE_R:
        {
            angle += deltaTime / 2 * targetangle;
            for (int i = 0; i < 27; i++)
            {
                glm::mat3 block = allMats[i];
                glm::vec3 newBlock = block * blockPositions[i];
                float& x = newBlock.x;
                if (x < 1.01f && x > 0.99f)
                {
                    glm::mat4 tempModel = glm::rotate(currentModel, glm::radians(angle), axisVec);
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                else
                {
                    glm::mat4 tempModel = currentModel;
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                cubeShader.setMat4("model", newModel);
                glBindVertexArray(cubeVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);

            }
            if (angle > targetangle)
            {
                state = UPDATE;
            }
            break;
        }
        case ROTATE_L:
        {
            angle += deltaTime / 2 * targetangle;
            for (int i = 0; i < 27; i++)
            {
                glm::mat3 block = allMats[i];
                glm::vec3 newBlock = block * blockPositions[i];
                float& x = newBlock.x;
                if (x > -1.01f && x < -0.99f)
                {
                    glm::mat4 tempModel = glm::rotate(currentModel, glm::radians(angle), axisVec);
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                else
                {
                    glm::mat4 tempModel = currentModel;
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                cubeShader.setMat4("model", newModel);
                glBindVertexArray(cubeVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);

            }
            if (angle > targetangle)
            {
                state = UPDATE;
            }
            break;
        }
        case ROTATE_U:
        {
            angle += deltaTime / 2 * targetangle;
            for (int i = 0; i < 27; i++)
            {
                glm::mat3 block = allMats[i];
                glm::vec3 newBlock = block * blockPositions[i];
                float& y = newBlock.y;
                if (y < 1.01f && y > 0.99f)
                {
                    glm::mat4 tempModel = glm::rotate(currentModel, glm::radians(angle), axisVec);
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                else
                {
                    glm::mat4 tempModel = currentModel;
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                cubeShader.setMat4("model", newModel);
                glBindVertexArray(cubeVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }
            if (angle > targetangle)
            {
                state = UPDATE;
            }
            break;
        }
        case ROTATE_D:
        {
            angle += deltaTime / 2 * targetangle;
            for (int i = 0; i < 27; i++)
            {
                glm::mat3 block = allMats[i];
                glm::vec3 newBlock = block * blockPositions[i];
                float& y = newBlock.y;
                if (y > -1.01f && y < -0.99f)
                {
                    glm::mat4 tempModel = glm::rotate(currentModel, glm::radians(angle), axisVec);
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                else
                {
                    glm::mat4 tempModel = currentModel;
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                cubeShader.setMat4("model", newModel);
                glBindVertexArray(cubeVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }
            if (angle > targetangle)
            {
                state = UPDATE;
            }
            break;
        }
        case ROTATE_F:
        {
            angle += deltaTime / 2 * targetangle;
            for (int i = 0; i < 27; i++)
            {
                glm::mat3 block = allMats[i];
                glm::vec3 newBlock = block * blockPositions[i];
                float& z = newBlock.z;
                if (z < 1.01f && z > 0.99f)
                {
                    glm::mat4 tempModel = glm::rotate(currentModel, glm::radians(angle), axisVec);
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                else
                {
                    glm::mat4 tempModel = currentModel;
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                cubeShader.setMat4("model", newModel);
                glBindVertexArray(cubeVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }
            if (angle > targetangle)
            {
                state = UPDATE;
            }
            break;
        }
        case ROTATE_B:
        {
            angle += deltaTime / 2 * targetangle;
            for (int i = 0; i < 27; i++)
            {
                glm::mat3 block = allMats[i];
                glm::vec3 newBlock = block * blockPositions[i];
                float& z = newBlock.z;
                if (z > -1.01f && z < -0.99f)
                {
                    glm::mat4 tempModel = glm::rotate(currentModel, glm::radians(angle), axisVec);
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                else
                {
                    glm::mat4 tempModel = currentModel;
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                cubeShader.setMat4("model", newModel);
                glBindVertexArray(cubeVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }
            if (angle > targetangle)
            {
                state = UPDATE;
            }
            break;
        }
        case UPDATE:
        {
            //需要对axis变量上锁。
            unique_lock<mutex> lock(axismutex);
            for (int i = 0; i < 27; i++)
            {
                glm::mat3 block = allMats[i];
                glm::vec3 newBlock = block * blockPositions[i];
                float& x = newBlock.x;
                float& y = newBlock.y;
                float& z = newBlock.z;
                if (nextState == ROTATE_L)
                {
                    if (x > -1.01f && x < -0.99f)
                    {
                        //先更新旋转
                        glm::mat4 thistime = glm::rotate(world, glm::radians(targetangle), glm::vec3(1.f, 0.f, 0.f));
                        allMats[i] = thistime * allMats[i];
                    }
                }
                else if (nextState == ROTATE_D)
                {
                    if (y > -1.01f && y < -0.99f)
                    {
                        glm::mat4 thistime = glm::rotate(world, glm::radians(targetangle), glm::vec3(0.0f, 1.0f, 0.0f));
                        allMats[i] = thistime * allMats[i];
                    }
                }
                else if (nextState == ROTATE_B)
                {
                    if (z > -1.01f && z < -0.99f)
                    {
                        glm::mat4 thistime = glm::rotate(world, glm::radians(targetangle), glm::vec3(0.0f, 0.0f, 1.0f));
                        allMats[i] = thistime * allMats[i];
                    }
                }
                else if (nextState == ROTATE_R)
                {
                    if (x < 1.01f && x > 0.99f)
                    {
                        glm::mat4 thistime = glm::rotate(world, glm::radians(targetangle), glm::vec3(1.0f, 0.0f, 0.0f));
                        allMats[i] = thistime * allMats[i];
                    }
                }
                else if (nextState == ROTATE_U)
                {
                    if (y < 1.01f && y > 0.99f)
                    {
                        glm::mat4 thistime = glm::rotate(world, glm::radians(targetangle), glm::vec3(0.0f, 1.0f, 0.0f));
                        allMats[i] = thistime * allMats[i];
                    }
                }
                else if (nextState == ROTATE_F)
                {
                    if (z < 1.01f && z > 0.99f)
                    {
                        glm::mat4 thistime = glm::rotate(world, glm::radians(targetangle), glm::vec3(0.0f, 0.0f, 1.0f));
                        allMats[i] = thistime * allMats[i];
                    }
                }
            }
            lock.unlock();

            // Re-draw all the points after updating
            for (int i = 0; i < 27; i++)
            {
                glm::mat4 tempModel = currentModel;
                tempModel *= allMats[i];
                newModel = glm::translate(tempModel, blockPositions[i]);
                cubeShader.setMat4("model", newModel);
                glBindVertexArray(cubeVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }

            state = STOP;
            nextState = STOP;
            break;
        }
        }

        // cubes
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    // glDeleteVertexArrays(1, &skyboxVAO);
    // glDeleteBuffers(1, &skyboxVBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

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
        lock_guard<mutex> lock(axismutex);
        rotate_queue.push(ROTATE_R);
    }
    if (key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        lock_guard<mutex> lock(axismutex);
        rotate_queue.push(ROTATE_L);
    }
    if (key == GLFW_KEY_U && action == GLFW_PRESS)
    {
        lock_guard<mutex> lock(axismutex);
        rotate_queue.push(ROTATE_U);
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        lock_guard<mutex> lock(axismutex);
        rotate_queue.push(ROTATE_D);
        //targetangle = 90;
    }
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        lock_guard<mutex> lock(axismutex);
        rotate_queue.push(ROTATE_F);
    }
    if (key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        lock_guard<mutex> lock(axismutex);
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

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}