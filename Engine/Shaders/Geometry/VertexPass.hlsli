// Copyright (c) Recluse Engine 2021.
#version 430
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable

#define VERTEX 1
#define DEPTH_ONLY
#define CUT_OUT
#define INSTANCED

#include "Vertex.glsl"
#include "ApplicationSettings.glsl"

layout (set = 0, binding = 0) uniform GlobalBlock {
    SceneView scene;
};

layout (set = 1, binding = 0) uniform PerMesh {
    PerMeshTransform mesh;
};

#ifndef DEPTH_ONLY

out FragIn {
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec4 texCoords;
} frag;

#endif


void main()
{
#if defined(VERTEX)

    vec4 position   = vec4(position, 1) * mesh.worldToViewClip;

#ifndef DEPTH_ONLY
    frag.normal     = normal;
    frag.texCoords  = texCoords;
    frag.tangent    = tangent;
    frag.bitangent  = bitangent;
#endif

    gl_Position = position;
#endif
}