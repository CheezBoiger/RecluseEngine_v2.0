#version 430
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoords;

#include "RecluseSceneBuffer.h"

layout (set = 0, binding = 0) uniform Global {
    RecluseSceneView sceneView;
};

out VertexIn {
    vec4 posWS;
    vec3 normWS;
    vec2 texCoords;
} vertIn;

void main() {
    vertIn.posWS = vec4(position, 1.0);
    gl_Position = vertIn.posWS; 
}
