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
#include <map>

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

// store rotate state
std::queue<State> rotate_queue;
typedef std::vector<State> move_seq_t;

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

HWND hwndGL;
WNDPROC OldProc;

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
        MessageBox(NULL, TEXT("Failed to create GLFW window"), TEXT("ERROR Message"), MB_ICONERROR | MB_OK);
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
        MessageBox(NULL, TEXT("Failed to load GLAD"), TEXT("ERROR Message"), MB_ICONERROR | MB_OK);
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Use for debug
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Menu
    hwndGL = GetActiveWindow();
    ShowWindow(hwndGL, SW_MAXIMIZE);
    HMENU hMenu = CreateMenu();
    HMENU hMenu_material = CreatePopupMenu();
    HMENU hMenu_light = CreatePopupMenu();

    OldProc = (WNDPROC)SetWindowLong(hwndGL, GWL_WNDPROC, (LONG)WndProc);

    AppendMenu(hMenu, MF_STRING, 1, TEXT("Random State"));
    AppendMenu(hMenu, MF_STRING, 2, TEXT("Read State"));
    AppendMenu(hMenu, MF_STRING, 3, TEXT("Light Move/Stop"));
    AppendMenu(hMenu, MF_POPUP, (UINT)hMenu_material, TEXT("Material"));
    AppendMenu(hMenu, MF_POPUP, (UINT)hMenu_light, TEXT("Light"));
    AppendMenu(hMenu_material, MF_STRING, 11, TEXT("Pearl"));
    AppendMenu(hMenu_material, MF_STRING, 12, TEXT("White plastic"));
    AppendMenu(hMenu_material, MF_STRING, 13, TEXT("White rubber"));
    AppendMenu(hMenu_material, MF_STRING, 14, TEXT("Emerald"));
    AppendMenu(hMenu_material, MF_STRING, 15, TEXT("Jade"));
    AppendMenu(hMenu_material, MF_STRING, 16, TEXT("Obsidian"));
    AppendMenu(hMenu_material, MF_STRING, 17, TEXT("Ruby"));
    AppendMenu(hMenu_material, MF_STRING, 18, TEXT("Turquoise"));
    AppendMenu(hMenu_material, MF_STRING, 19, TEXT("Silver"));
    AppendMenu(hMenu_light, MF_STRING, 21, TEXT("On/Off"));
    AppendMenu(hMenu_light, MF_STRING, 22, TEXT("Gradient"));

    SetMenu(hwndGL, hMenu);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader cubeShader("cube.vs", "cube.fs");
    Shader skyboxShader("skybox.vs", "skybox.fs");
    Shader lightCubeShader("lightcube.vs", "lightcube.fs");
    
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float blockVertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    float color[] = {
        // Red
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        // Orange
        1.0f, 0.5f, 0.31f,
        1.0f, 0.5f, 0.31f,
        1.0f, 0.5f, 0.31f,
        1.0f, 0.5f, 0.31f,
        1.0f, 0.5f, 0.31f,
        1.0f, 0.5f, 0.31f,

        // Green
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        // Blue
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,

        // White
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,

        // Yellow
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
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

    float lightVertices[] = {
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

    // positions of the point lights
    glm::vec3 pointLightPositions[] = {
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(2.5f,  0.0f,  0.0f),
        glm::vec3(0.0f,  0.0f,  2.5f),
        glm::vec3(0.0f,  2.5f,  0.0f)
    };

    // material list
    float materialVertices[] = {
        // Ambient                 // Diffuse                    // Specular                         // shininess
        0.0215f, 0.1745f,  0.0215f,  0.07568f, 0.61424f, 0.07568f, 0.633f,    0.727811f, 0.633f,     0.6f,          // Emerald
        0.135f,  0.2225f,  0.1575f,  0.54f,    0.89f,    0.63f,    0.316228f, 0.316228f, 0.316228f,  0.1f,          // Jade
        0.25f,   0.20725f, 0.20725f, 1.0f,     0.829f,   0.829f,   0.296648f, 0.296648f, 0.296648f,  0.088f,        // Pearl
        0.0f,    0.0f,     0.0f,     0.55f,    0.55f,    0.55f,    0.70f,     0.70f,     0.70f,      0.25f,         // White plastic
        0.05f,   0.05f,    0.05f,    0.5f,     0.5f,     0.5f,     0.7f,      0.7f,      0.7f,       0.78125f,      // White rubber
        0.05375f,0.05f,    0.06625f, 0.18275f, 0.17f,    0.22525f, 0.332741f, 0.328634f, 0.346435f,  0.3f,          // Obsidian
        0.1745f, 0.01175f, 0.01175f, 0.61424f, 0.04136f, 0.04136f, 0.727811f, 0.626959f, 0.626959f,  0.6f,          // Ruby
        0.1f,    0.18725f, 0.1745f,  0.396f,   0.74151f, 0.69102f, 0.297254f, 0.30829f,  0.306678f,  0.1f,          // Turquoise
        0.19225f,0.19225f, 0.19225f, 0.50754f, 0.50754f, 0.50754f, 0.508273f, 0.508273f, 0.508273f,  0.4f           // Silver
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
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // config lightCubeVBO
    unsigned int lightCubeVBO;
    glGenBuffers(1, &lightCubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices, GL_STATIC_DRAW);
    // configure the lightCube VAO
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    // we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
    glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // cube VAO
    unsigned int cubeVBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(blockVertices), blockVertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // color attribute
    unsigned int colorbuffer;
    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

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
        /* Todo */
        /*std::map<float, glm::vec3> sorted;
        for (int i = 0; i < sizeof(blockPositions) / sizeof(glm::vec3); i++)
        {
            float distance = glm::length(camera.Position - blockPositions[i]);
            sorted[distance] = blockPositions[i];
        }*/
        
        // Render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (lightMov == true) 
        {
            pointLightPositions[0].x = sin(glfwGetTime()) * cos(glfwGetTime() / 2.0f) * 5.2f;
            pointLightPositions[0].y = cos(glfwGetTime()) * sin(glfwGetTime() / 2.0f) * 5.2f;
            pointLightPositions[0].z = cos(glfwGetTime()) * 5.2f;
        }

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SRC_WIDTH / (float)SRC_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        // activate shader
        cubeShader.use();
        cubeShader.setVec3("viewPos", camera.Position);
        
        /*
           Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
           the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
           by defining light types as classes and set their values in there, or by using a more efficient uniform approach
           by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
        */
        // directional light
        cubeShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        cubeShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        cubeShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        cubeShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        // point light 1
        cubeShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        cubeShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        cubeShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        cubeShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        cubeShader.setFloat("pointLights[0].constant", 1.0f);
        cubeShader.setFloat("pointLights[0].linear", 0.09);
        cubeShader.setFloat("pointLights[0].quadratic", 0.032);
        // point light 2
        cubeShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        cubeShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        cubeShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        cubeShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        cubeShader.setFloat("pointLights[1].constant", 1.0f);
        cubeShader.setFloat("pointLights[1].linear", 0.09);
        cubeShader.setFloat("pointLights[1].quadratic", 0.032);
        // point light 3
        cubeShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        cubeShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        cubeShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
        cubeShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        cubeShader.setFloat("pointLights[2].constant", 1.0f);
        cubeShader.setFloat("pointLights[2].linear", 0.09);
        cubeShader.setFloat("pointLights[2].quadratic", 0.032);
        // point light 4
        cubeShader.setVec3("pointLights[3].position", pointLightPositions[3]);
        cubeShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        cubeShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
        cubeShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
        cubeShader.setFloat("pointLights[3].constant", 1.0f);
        cubeShader.setFloat("pointLights[3].linear", 0.09);
        cubeShader.setFloat("pointLights[3].quadratic", 0.032);
        // spotLight
        cubeShader.setVec3("spotLight.position", camera.Position);
        cubeShader.setVec3("spotLight.direction", camera.Front);
        cubeShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        cubeShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        cubeShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        cubeShader.setFloat("spotLight.constant", 1.0f);
        cubeShader.setFloat("spotLight.linear", 0.09);
        cubeShader.setFloat("spotLight.quadratic", 0.032);
        cubeShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        cubeShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        // material properties
        if (material == Pearl)
        {
            cubeShader.setVec3("material.ambient", materialVertices[20], materialVertices[21], materialVertices[22]);
            cubeShader.setVec3("material.diffuse", materialVertices[23], materialVertices[24], materialVertices[25]);
            cubeShader.setVec3("material.specular", materialVertices[26], materialVertices[27], materialVertices[28]); // specular lighting doesn't have full effect on this object's material
            cubeShader.setFloat("material.shininess", 128.0f * materialVertices[29]);
        }
        else if (material == White_plastic)
        {
            cubeShader.setVec3("material.ambient", materialVertices[30], materialVertices[31], materialVertices[32]);
            cubeShader.setVec3("material.diffuse", materialVertices[33], materialVertices[34], materialVertices[35]);
            cubeShader.setVec3("material.specular", materialVertices[36], materialVertices[37], materialVertices[38]); // specular lighting doesn't have full effect on this object's material
            cubeShader.setFloat("material.shininess", 128.0f * materialVertices[39]);
        }
        else if (material == White_rubber)
        {
            cubeShader.setVec3("material.ambient", materialVertices[40], materialVertices[41], materialVertices[42]);
            cubeShader.setVec3("material.diffuse", materialVertices[43], materialVertices[44], materialVertices[45]);
            cubeShader.setVec3("material.specular", materialVertices[46], materialVertices[47], materialVertices[48]); // specular lighting doesn't have full effect on this object's material
            cubeShader.setFloat("material.shininess", 128.0f * materialVertices[49]);
        }
        else if (material == Emerald)
        {
            cubeShader.setVec3("material.ambient", materialVertices[0], materialVertices[1], materialVertices[2]);
            cubeShader.setVec3("material.diffuse", materialVertices[3], materialVertices[4], materialVertices[5]);
            cubeShader.setVec3("material.specular", materialVertices[6], materialVertices[7], materialVertices[8]); // specular lighting doesn't have full effect on this object's material
            cubeShader.setFloat("material.shininess", 128.0f * materialVertices[9]);
        }
        else if (material == Jade)
        {
            cubeShader.setVec3("material.ambient", materialVertices[10], materialVertices[11], materialVertices[12]);
            cubeShader.setVec3("material.diffuse", materialVertices[13], materialVertices[14], materialVertices[15]);
            cubeShader.setVec3("material.specular", materialVertices[16], materialVertices[17], materialVertices[18]); // specular lighting doesn't have full effect on this object's material
            cubeShader.setFloat("material.shininess", 128.0f * materialVertices[19]);
        }
        else if (material == Obsidian)
        {
            cubeShader.setVec3("material.ambient", materialVertices[50], materialVertices[51], materialVertices[52]);
            cubeShader.setVec3("material.diffuse", materialVertices[53], materialVertices[54], materialVertices[55]);
            cubeShader.setVec3("material.specular", materialVertices[56], materialVertices[57], materialVertices[58]); // specular lighting doesn't have full effect on this object's material
            cubeShader.setFloat("material.shininess", 128.0f * materialVertices[59]);
        }
        else if (material == Ruby)
        {
            cubeShader.setVec3("material.ambient", materialVertices[60], materialVertices[61], materialVertices[62]);
            cubeShader.setVec3("material.diffuse", materialVertices[63], materialVertices[64], materialVertices[65]);
            cubeShader.setVec3("material.specular", materialVertices[66], materialVertices[67], materialVertices[68]); // specular lighting doesn't have full effect on this object's material
            cubeShader.setFloat("material.shininess", 128.0f * materialVertices[69]);
        }
        else if (material == Turquoise)
        {
            cubeShader.setVec3("material.ambient", materialVertices[70], materialVertices[71], materialVertices[72]);
            cubeShader.setVec3("material.diffuse", materialVertices[73], materialVertices[74], materialVertices[75]);
            cubeShader.setVec3("material.specular", materialVertices[76], materialVertices[77], materialVertices[78]); // specular lighting doesn't have full effect on this object's material
            cubeShader.setFloat("material.shininess", 128.0f * materialVertices[79]);
        }
        else if (material == Silver)
        {
            cubeShader.setVec3("material.ambient", materialVertices[80], materialVertices[81], materialVertices[82]);
            cubeShader.setVec3("material.diffuse", materialVertices[83], materialVertices[84], materialVertices[85]);
            cubeShader.setVec3("material.specular", materialVertices[86], materialVertices[87], materialVertices[88]); // specular lighting doesn't have full effect on this object's material
            cubeShader.setFloat("material.shininess", 128.0f * materialVertices[89]);
        }

        // camera/view transformation
        view = glm::lookAt(camera.Position, glm::vec3(0.f, 0.f, 0.f), camera.Up);
        cubeShader.setMat4("view", view);
        cubeShader.setMat4("projection", projection);
        
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
                axisVec = glm::vec3(-1.0f, 0.0f, 0.0f);
            }
            else if (nextState == ROTATE_U)
            {
                state = ROTATE_U;
                angle = 0;
                axisVec = glm::vec3(0.0f, -1.0f, 0.0f);
            }
            else if (nextState == ROTATE_F)
            {
                state = ROTATE_F;
                angle = 0;
                axisVec = glm::vec3(0.0f, 0.0f, -1.0f);
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
            // lock axis
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
                        // update before rotate
                        glm::mat4 newMat = glm::rotate(world, glm::radians(targetangle), glm::vec3(1.f, 0.f, 0.f));
                        allMats[i] = newMat * allMats[i];
                    }
                }
                else if (nextState == ROTATE_D)
                {
                    if (y > -1.01f && y < -0.99f)
                    {
                        glm::mat4 newMat = glm::rotate(world, glm::radians(targetangle), glm::vec3(0.0f, 1.0f, 0.0f));
                        allMats[i] = newMat * allMats[i];
                    }
                }
                else if (nextState == ROTATE_B)
                {
                    if (z > -1.01f && z < -0.99f)
                    {
                        glm::mat4 newMat = glm::rotate(world, glm::radians(targetangle), glm::vec3(0.0f, 0.0f, 1.0f));
                        allMats[i] = newMat * allMats[i];
                    }
                }
                else if (nextState == ROTATE_R)
                {
                    if (x < 1.01f && x > 0.99f)
                    {
                        glm::mat4 newMat = glm::rotate(world, glm::radians(targetangle), glm::vec3(-1.0f, 0.0f, 0.0f));
                        allMats[i] = newMat * allMats[i];
                    }
                }
                else if (nextState == ROTATE_U)
                {
                    if (y < 1.01f && y > 0.99f)
                    {
                        glm::mat4 newMat = glm::rotate(world, glm::radians(targetangle), glm::vec3(0.0f, -1.0f, 0.0f));
                        allMats[i] = newMat * allMats[i];
                    }
                }
                else if (nextState == ROTATE_F)
                {
                    if (z < 1.01f && z > 0.99f)
                    {
                        glm::mat4 newMat = glm::rotate(world, glm::radians(targetangle), glm::vec3(0.0f, 0.0f, -1.0f));
                        allMats[i] = newMat * allMats[i];
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

        /* Todo */
        //// transparent effect
        //for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        //{
        //    model = glm::mat4(1.0f);
        //    model = glm::translate(model, it->second);
        //    cubeShader.setMat4("model", model);
        //    glBindVertexArray(cubeVAO);
        //    glDrawArrays(GL_TRIANGLES, 0, 36);
        //}

        // draw the lamp object
        lightCubeShader.use();
        /*if (lightOn == true)
            lightCubeShader.setVec4("lightColor", glm::vec4(1.0f));
        else
            lightCubeShader.setVec4("lightColor", glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));*/
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        glBindVertexArray(lightCubeVAO);
        for (unsigned int i = 0; i < 4; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
            lightCubeShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
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
    glDeleteBuffers(1, &colorbuffer);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &lightCubeVBO);

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
            MessageBox(NULL, TEXT("Cubemap texture failed to load"), TEXT("ERROR Message"), MB_ICONERROR | MB_OK);
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

void initState()
{
    return;
}