// Copyright (c) Recluse Engine 2021.
#ifndef VERTEX_GLSL
#define VERTEX_GLSL


#if defined(VERTEX)
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec4 texCoords;
#endif

#if defined(SKINNED)

layout (location = 5) in vec4   weights;
layout (location = 6) in ivec4  indices;

#endif

#if defined(INSTANCED)
layout (location = 7) in mat4 transform;
#endif

#if defined(MORPH_TARGETS)
layout (location = 8) in vec3 morphPosition00;
layout (location = 9) in vec3 morphNormal00;

layout (location = 10) in vec3 morphPosition01;
layout (location = 11) in vec4 morphNormal01;
#endif

#endif // VERTEX_GLSL