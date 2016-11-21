/*
 * @file	PrefrShaders.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es> 
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *					Do not distribute without further notice.
 */

#ifndef PREFRSHADERS_H_
#define PREFRSHADERS_H_

namespace prefr
{

const static std::string prefrVertexShader = "#version 330\n\
#extension GL_ARB_separate_shader_objects: enable\n\
uniform mat4 modelViewProjM;\n\
uniform vec3 cameraUp;\n\
uniform vec3 cameraRight;\n\
layout(location = 0) in vec3 vertexPosition;\n\
layout(location = 1) in vec4 particlePosition;\n\
layout(location = 2) in vec4 particleColor;\n\
out vec4 color;\n\
out vec2 uvCoord;\n\
void main()\n\
{\n\
  gl_Position = modelViewProjM\n\
        * vec4(\n\
        (vertexPosition.x * particlePosition.a * cameraRight)\n\
        + (vertexPosition.y * particlePosition.a * cameraUp)\n\
        + particlePosition.rgb, 1.0);\n\
  color = particleColor;\n\
  uvCoord = vertexPosition.rg + vec2(0.5, 0.5);\n\
}";

const static std::string prefrFragmentShader = "#version 330\n\
in vec4 color; \n\
in vec2 uvCoord;\n\
out vec4 outputColor;\n\
void main()\n\
{\n \
  vec2 p = -1.0 + 2.0 * uvCoord;\n\
  float l = sqrt(dot(p,p));\n \
  l = 1.0 - clamp(l, 0.0, 1.0);\n\
  l *= color.a;\n\
  outputColor = vec4(color.rgb, l);\n\
}";

}

#endif /* PREFRSHADERS_H_ */
