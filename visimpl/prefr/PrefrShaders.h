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

const static std::string prefrVertexShader = R"(#version 330
#extension GL_ARB_separate_shader_objects: enable
uniform mat4 modelViewProjM;
uniform vec3 cameraUp;
uniform vec3 cameraRight;
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec4 particlePosition;
layout(location = 2) in vec4 particleColor;
out vec4 color;
out vec2 uvCoord;
void main()
{
  gl_Position = modelViewProjM
        * vec4(
        (vertexPosition.x * particlePosition.a * cameraRight)
        + (vertexPosition.y * particlePosition.a * cameraUp)
        + particlePosition.rgb, 1.0);
  color = particleColor;
  uvCoord = vertexPosition.rg + vec2(0.5, 0.5);
})";

const static std::string prefrFragmentShaderDefault = R"(#version 330
in vec4 color; 
in vec2 uvCoord;
out vec4 outputColor;
void main()
{
  vec2 p = -1.0 + 2.0 * uvCoord;
  float l = sqrt(dot(p,p));
  l = 1.0 - clamp(l, 0.0, 1.0);
  l *= color.a;
  outputColor = vec4(color.rgb, l);
})";

const static std::string prefrFragmentShaderSolid = R"(#version 330
uniform float radiusThreshold;
in vec4 color; 
in vec2 uvCoord;
out vec4 outputColor;
void main()
{
  vec2 p = -1.0 + 2.0 * uvCoord;
  float l = sqrt(dot(p,p));
  l = clamp(l, 0.0, 1.0);
  
  if( l > radiusThreshold )
    discard;

  outputColor = color;
})";

const static std::string prefrVertexShaderPicking = R"(#version 330
#extension GL_ARB_separate_shader_objects: enable

uniform mat4 modelViewProjM;

uniform vec3 cameraUp;
uniform vec3 cameraRight;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec4 particlePosition;
layout(location = 2) in vec4 particleColor;


out vec4 color;
out vec2 uvCoord;

out float id;

void main()
{
  //gl_Position = vec4((vec4(vertexPosition * particlePosition.a, 1.0) + (modelViewProjM * vec4(particlePosition.rgb, 1.0))).rgb, 1.0);

  gl_Position = modelViewProjM 
        * vec4(
        (vertexPosition.x * particlePosition.a * cameraRight)
        + (vertexPosition.y * particlePosition.a * cameraUp)
        + particlePosition.rgb, 1.0);

  color = particleColor;

  uvCoord = vertexPosition.rg + vec2(0.5, 0.5);

  id = gl_InstanceID;
})";

const static std::string prefrFragmentShaderPicking = R"(#version 330

uniform float radiusThreshold;

in vec4 color;
in vec2 uvCoord;
in float id;

out vec4 outputColor;

vec3 unpackColor( float f )
{
  vec3 color = fract(vec3(1.0/255.0, 1.0/(255.0*255.0), 
    1.0/(255.0*255.0*255.0)) * f);
  color -= color.xxy * vec3(0.0, 1.0/255.0, 1.0/255.0);

  return color;
}

void main( )
{ 

  vec2 p = -1.0 + 2.0 * uvCoord;
  float l = sqrt(dot(p,p));
  l = clamp(l, 0.0, 1.0);

  if( l > radiusThreshold )
    discard;

  vec3 cc = unpackColor(id);
  outputColor = vec4(cc, 1.0);
})";


}

#endif /* PREFRSHADERS_H_ */
