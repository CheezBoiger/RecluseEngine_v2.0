#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_enhanced_layouts : enable

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec4 inColor;


void main()
{
    outColor = inColor;
}