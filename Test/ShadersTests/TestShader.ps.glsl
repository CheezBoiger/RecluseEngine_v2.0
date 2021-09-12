#version 430
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec4 color;

#include "RecluseSceneBuffer.h"

in VertexIn {
    vec4 posWS;
    vec3 normWS;
    vec2 texCoords;
} pixOut;

void main() {
    color = vec4(1.0);
}