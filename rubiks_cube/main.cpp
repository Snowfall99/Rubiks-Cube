#include "root.h"

#include "shader_s.h"
#include "skybox.h"
#include "util.h"
#include "menu.h"
#include "viewer.h"
#include "vertices.h"
#include "light.h"
#include "material.h"
#include "rotate.h"

using namespace std;
using namespace cube;

int main()
{
    // Init window 
    GLFWwindow* window = initWindow();
    // Config menu
    setMenu();

    // build and compile our shader zprogram
    // ------------------------------------
    Shader cubeShader("cube.vs", "cube.fs");
    Shader skyboxShader("skybox.vs", "skybox.fs");
    Shader lightCubeShader("lightcube.vs", "lightcube.fs");

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
        cubeShader.setVec3("viewPos", camera.Position);

        // Light properties
        setLight(cubeShader);

        // material properties
        setMaterial(cubeShader);

        // camera/view transformation
        view = glm::lookAt(camera.Position, glm::vec3(0.f, 0.f, 0.f), camera.Up);
        cubeShader.setMat4("view", view);
        cubeShader.setMat4("projection", projection);
        
        // calculate the model matrix for each object and pass it to shader before drawing
        currentModel = glm::rotate(currentModel, glm::radians(deltaX), glm::vec3(1.0f, 0.f, 0.f));
        currentModel = glm::rotate(currentModel, glm::radians(deltaY), glm::vec3(0.0f, 1.f, 0.f));
        currentModel = glm::rotate(currentModel, glm::radians(deltaZ), glm::vec3(0.0f, 0.f, 1.f));

        if (mode == Random)
        {
            move_seq_t rs = randomQueue();
            randomState(rs, cubeShader, cubeVAO);
            mode = Default;
        }
        else if (mode == Read_File)
        { 
            move_seq_t rs = readStateFromFile();
            for (auto& step : rs)
            {
                nextState = step;
                update(cubeShader, cubeVAO);
            }
            mode = Default;
        }
        else
        {
            // update cube based on rotate state
            rotateState(cubeShader, cubeVAO);
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
        
        skyboxDraw(skyboxShader, skyboxVAO, cubemapTexture);

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
