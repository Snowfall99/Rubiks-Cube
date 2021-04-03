#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

#include "root.h"
#include "shader_s.h"
#include "vertices.h"

namespace cube
{
	void setMaterial(Shader cubeShader)
	{
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
	}
}
#endif // !MATERIAL_H
