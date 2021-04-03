#pragma once
#ifndef ROTATE_H
#define ROTATE_H

#include "root.h"
#include "vertices.h"
#include "shader_s.h"

namespace cube
{
	void rotateState(Shader, unsigned int);
	void update(Shader, unsigned int);

	// rotate cube based on rotate queue
	void rotateState(Shader shader, unsigned int VAO)
	{
        glm::mat4 newModel;

        switch (state)
        {
        case STOP:
        {
            // keep original state
            for (int i = 0; i < 27; i++)
            {
                newModel = glm::translate(currentModel * allMats[i], blockPositions[i]);
                shader.setMat4("model", newModel);
                glBindVertexArray(VAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }
            // read and update state 
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
                // rotate related face 
                if (x < 1.01f && x > 0.99f)
                {
                    glm::mat4 tempModel = glm::rotate(currentModel, glm::radians(angle), axisVec);
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                // keep other face still
                else
                {
                    glm::mat4 tempModel = currentModel;
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                // refresh model
                shader.setMat4("model", newModel);
                glBindVertexArray(VAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }
            // update state
            if (angle > targetangle)
                update(shader, VAO);
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
                // rotate related face
                if (x > -1.01f && x < -0.99f)
                {
                    glm::mat4 tempModel = glm::rotate(currentModel, glm::radians(angle), axisVec);
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                // keep the other face still
                else
                {
                    glm::mat4 tempModel = currentModel;
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                // refresh model
                shader.setMat4("model", newModel);
                glBindVertexArray(VAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);

            }
            // update state
            if (angle > targetangle)
                update(shader, VAO);
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
                // rotate related face
                if (y < 1.01f && y > 0.99f)
                {
                    glm::mat4 tempModel = glm::rotate(currentModel, glm::radians(angle), axisVec);
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                // keep the other face still
                else
                {
                    glm::mat4 tempModel = currentModel;
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                // refresh model
                shader.setMat4("model", newModel);
                glBindVertexArray(VAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }
            // update state
            if (angle > targetangle)
                update(shader, VAO);
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
                // rotate related face
                if (y > -1.01f && y < -0.99f)
                {
                    glm::mat4 tempModel = glm::rotate(currentModel, glm::radians(angle), axisVec);
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                // keep the other face still
                else
                {
                    glm::mat4 tempModel = currentModel;
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                // refresh model
                shader.setMat4("model", newModel);
                glBindVertexArray(VAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }
            // update state
            if (angle > targetangle)
                update(shader, VAO);
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
                // rotate related face
                if (z < 1.01f && z > 0.99f)
                {
                    glm::mat4 tempModel = glm::rotate(currentModel, glm::radians(angle), axisVec);
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                // keep the other face still
                else
                {
                    glm::mat4 tempModel = currentModel;
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                // refresh model
                shader.setMat4("model", newModel);
                glBindVertexArray(VAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }
            // update state
            if (angle > targetangle)
                update(shader, VAO);
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
                // rotate related face 
                if (z > -1.01f && z < -0.99f)
                {
                    glm::mat4 tempModel = glm::rotate(currentModel, glm::radians(angle), axisVec);
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                // keep the other face still
                else
                {
                    glm::mat4 tempModel = currentModel;
                    tempModel *= allMats[i];
                    newModel = glm::translate(tempModel, blockPositions[i]);
                }
                // refresh model
                shader.setMat4("model", newModel);
                glBindVertexArray(VAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }
            // update state
            if (angle > targetangle)
                update(shader, VAO);
            break;
        }
        }
	}

    // update cube state matrix 
    void update(Shader shader, unsigned int VAO)
    {
        // lock axis
        std::unique_lock<std::mutex> lock(axismutex);
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

        glm::mat4 newModel;

        // Re-draw all the points after updating
        for (int i = 0; i < 27; i++)
        {
            glm::mat4 tempModel = currentModel;
            tempModel *= allMats[i];
            newModel = glm::translate(tempModel, blockPositions[i]);
            shader.setMat4("model", newModel);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        }

        state = STOP;
        nextState = STOP;
    }
}

#endif // !ROTATE_H
