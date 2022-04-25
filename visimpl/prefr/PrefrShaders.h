/*
 * Copyright (c) 2015-2020 VG-Lab/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of ViSimpl <https://github.com/vg-lab/visimpl>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef PREFRSHADERS_H_
#define PREFRSHADERS_H_

namespace prefr
{

const static std::string prefrVertexShader = R"(
#version 400
//#extension GL_ARB_separate_shader_objects: enable

uniform mat4 modelViewProjM;
uniform vec3 cameraUp;
uniform vec3 cameraRight;

// Clipping planes
uniform vec4 plane[ 2 ];
out float gl_ClipDistance[ 2 ];

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in float particleSize;
layout(location = 2) in vec3 particlePosition;
layout(location = 3) in vec4 particleColor;

out vec4 color;
out vec2 uvCoord;

void main()
{
  vec4 position =  vec4(
        (vertexPosition.x * particleSize * cameraRight)
        + (vertexPosition.y * particleSize * cameraUp)
        + particlePosition, 1.0);

  gl_ClipDistance[ 0 ] = dot( position, plane[ 0 ]);
  gl_ClipDistance[ 1 ] = dot( position, plane[ 1 ]);

  gl_Position = modelViewProjM * position;

  color = particleColor;
  uvCoord = vertexPosition.rg + vec2(0.5, 0.5);
}

)";

const static std::string prefrFragmentShaderDefault = R"(
#version 400
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
}

)";

const static std::string prefrFragmentShaderSolid = R"(#version 400
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

  outputColor = vec4(color.rgb, 1.0f);
})";

const static std::string prefrVertexShaderPicking = R"(#version 400
#extension GL_ARB_separate_shader_objects: enable

uniform mat4 modelViewProjM;

uniform vec3 cameraUp;
uniform vec3 cameraRight;

uniform vec4 plane[ 2 ];
out float gl_ClipDistance[ 2 ];

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in float particleSize;
layout(location = 2) in vec3 particlePosition;
layout(location = 3) in vec4 particleColor;

out vec4 color;
out vec2 uvCoord;

out float id;

void main()
{
  vec4 position =  vec4(
        (vertexPosition.x * particleSize * cameraRight)
        + (vertexPosition.y * particleSize * cameraUp)
        + particlePosition, 1.0);

  gl_ClipDistance[ 0 ] = dot( position, plane[ 0 ]);
  gl_ClipDistance[ 1 ] = dot( position, plane[ 1 ]);

  gl_Position = modelViewProjM * position;

  color = particleColor;

  uvCoord = vertexPosition.rg + vec2(0.5, 0.5);

  id = gl_InstanceID;
})";

const static std::string prefrFragmentShaderPicking = R"(#version 400

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

const static std::string planeVertCode = R"(
#version 400

in vec3 inPos;

uniform mat4 viewProj;
uniform vec4 inColor;

out vec4 outColor;

void main( )
{
  outColor = inColor;
  //gl_Position = vec4( quadVertices[ gl_VertexID ], 0.0, 1.0 );
  //gl_Position = vec4( inPos.rg, 1.0 );
  gl_Position = viewProj * vec4( inPos, 1.0 ); 
}

)";


const static std::string planeFragCode = R"(
#version 400

in vec4 outColor;
out vec4 outputColor;

void main( )
{
  outputColor = outColor;
  //outputColor = vec4( 1.0, 1.0, 1.0, 1.0 );
  
}

)";

}

#endif /* PREFRSHADERS_H_ */
