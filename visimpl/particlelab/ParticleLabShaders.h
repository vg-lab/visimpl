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

#include <string>

namespace visimpl
{

  const static std::string PARTICLE_VERTEX_SHADER = R"(#version 330
//#extension GL_ARB_separate_shader_objects: enable

uniform mat4 viewProjectionMatrix;
uniform vec3 cameraUp;
uniform vec3 cameraRight;

uniform float time;

uniform int gradientSize;
uniform float gradientTimes[256];
uniform vec4 gradientColors[256];

uniform int sizeSize;
uniform float sizeTimes[16];
uniform float sizeValues[16];

// Clipping planes
uniform vec4 plane[2];
out float gl_ClipDistance[2];

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 particlePosition;
layout(location = 2) in float timestamp;

out vec4 color;
out vec2 uvCoord;

float sizeGradient (float t) {
    if (sizeSize == 0) return 8.0f;
    int first = sizeSize - 1;
    for (int i = 0; i < sizeSize; i++) {
        if (sizeTimes[i] > t) {
            first = i - 1;
            break;
        }
    }

    // Particles have the last value before they reach the first value.
    if (first == -1 || first == sizeSize - 1)
    return sizeValues[sizeSize - 1];

    float start = sizeTimes[first];
    float end = sizeTimes[first + 1];
    float normalizedT = (t - start) / (end - start);

    return mix(sizeValues[first], sizeValues[first + 1], normalizedT);
}

vec4 gradient (float t) {
    if (gradientSize == 0) return vec4(1.0f, 0.0f, 1.0f, 1.0f);
    int first = gradientSize - 1;
    for (int i = 0; i < gradientSize; i++) {
        if (gradientTimes[i] > t) {
            first = i - 1;
            break;
        }
    }

    // Particles have the last value before they reach the first value.
    if (first == -1 || first == gradientSize - 1)
      return gradientColors[gradientSize - 1];

    float start = gradientTimes[first];
    float end = gradientTimes[first + 1];
    float normalizedT = (t - start) / (end - start);

    return mix(gradientColors[first], gradientColors[first + 1], normalizedT);
}

void main()
{
    float particleSize = sizeGradient(time - timestamp);
    vec4 position =  vec4(
    (vertexPosition.x * particleSize * cameraRight)
    + (vertexPosition.y * particleSize * cameraUp)
    + particlePosition, 1.0);

    gl_ClipDistance[0] = dot(position, plane[0]);
    gl_ClipDistance[1] = dot(position, plane[1]);

    gl_Position = viewProjectionMatrix * position;

    color = gradient(time - timestamp);
    uvCoord = vertexPosition.rg + vec2(0.5, 0.5);
}

)";

  const static std::string PARTICLE_DEFAULT_FRAGMENT_SHADER = R"(#version 330
in vec4 color;
in vec2 uvCoord;

layout (location = 0) out vec4 accumulation;
layout (location = 1) out float reveal;

void main()
{
    vec2 p = -1.0 + 2.0 * uvCoord;
    float l = sqrt(dot(p, p));
    l = 1.0 - clamp(l, 0.0, 1.0);
    l *= color.a;

    vec4 c = vec4(color.rgb, l);

    float weight = clamp(pow(min(1.0, c.a * 10.0) + 0.01, 3.0) * 1e8
    * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);

    accumulation = vec4 (c.rgb * c.a, c.a) * weight;
    reveal = c.a;
}

)";

  const static std::string PARTICLE_ACC_DEFAULT_FRAGMENT_SHADER = R"(#version 330
in vec4 color;
in vec2 uvCoord;

out vec4 fragColor;

void main()
{
    vec2 p = -1.0 + 2.0 * uvCoord;
    float l = sqrt(dot(p, p));
    l = 1.0 - clamp(l, 0.0, 1.0);
    l *= color.a;

    fragColor = vec4(color.rgb, l);
}

)";

  const static std::string PARTICLE_SOLID_FRAGMENT_SHADER = R"(#version 330
in vec4 color;
in vec2 uvCoord;

layout (location = 0) out vec4 accumulation;
layout (location = 1) out float reveal;

void main()
{
  vec2 p = -1.0 + 2.0 * uvCoord;
  float l = sqrt(dot(p,p));
  l = clamp(l, 0.0, 1.0);
  
  if( l > 0.8 )
    discard;

  vec4 c = vec4(color.rgb, 1.0f);

  float weight = clamp(pow(min(1.0, c.a * 10.0) + 0.01, 3.0) * 1e8
  * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);

  accumulation = vec4 (c.rgb * c.a, c.a) * weight;
  reveal = c.a;
})";

  const static std::string PARTICLE_ACC_SOLID_FRAGMENT_SHADER = R"(#version 330
in vec4 color;
in vec2 uvCoord;

out vec4 fragColor;

void main()
{
    vec2 p = -1.0 + 2.0 * uvCoord;
    float l = sqrt(dot(p,p));
    l = clamp(l, 0.0, 1.0);

    if( l > 0.8 )
      discard;

    fragColor = vec4(color.rgb, 1.0f);
}

)";


  const static std::string PICK_VERTEX_SHADER = R"(#version 330
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

  const static std::string PICK_FRAGMENT_SHADER = R"(#version 330

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

  const static std::string SHADER_PLANE_VERTEX = R"(#version 330

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


  const static std::string SHADER_PLANE_FRAGMENT = R"(#version 330

in vec4 outColor;
out vec4 outputColor;

void main( )
{
  outputColor = outColor;
  //outputColor = vec4( 1.0, 1.0, 1.0, 1.0 );
  
}

)";


  const static std::string SHADER_SCREEN_VERTEX = R"(#version 330

// shader inputs
layout (location = 0) in vec3 position;

void main()
{
	gl_Position = vec4(position, 1.0f);
}
)";

  const static std::string SHADER_SCREEN_FRAGMENT = R"(#version 330

// shader outputs
layout (location = 0) out vec4 frag;

// color accumulation buffer
uniform sampler2D accumulation;

// revealage threshold buffer
uniform sampler2D reveal;

// epsilon number
const float EPSILON = 0.00001f;

// caluclate floating point numbers equality accurately
bool isApproximatelyEqual(float a, float b)
{
	return abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * EPSILON;
}

// get the max value between three values
float max3(vec3 v)
{
	return max(max(v.x, v.y), v.z);
}

void main()
{
	// fragment coordination
	ivec2 coords = ivec2(gl_FragCoord.xy);

	// fragment revealage
	float revealage = texelFetch(reveal, coords, 0).r;

	// save the blending and color texture fetch cost if there is not a transparent fragment
	if (isApproximatelyEqual(revealage, 1.0f))
		discard;

	// fragment color
	vec4 accumulation = texelFetch(accumulation, coords, 0);

	// suppress overflow
	if (isinf(max3(abs(accumulation.rgb))))
		accumulation.rgb = vec3(accumulation.a);

	// prevent floating point precision bug
	vec3 average_color = accumulation.rgb / max(accumulation.a, EPSILON);

	// blend pixels
	frag = vec4(average_color, 1.0f - revealage);
}
)";

}

#endif /* PREFRSHADERS_H_ */
