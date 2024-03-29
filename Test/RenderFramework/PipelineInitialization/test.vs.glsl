#version 430
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 position;

layout (set = 0, binding = 0) uniform Test 
{
    vec4 color;
    vec4 offset;
} test;

layout (location = 0) out vec4 triColor;

void main()
{
    triColor = test.color;
    gl_Position = vec4(position + test.offset.zw, 0.0, 1.0);
}