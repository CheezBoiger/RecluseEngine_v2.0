#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec4 outColor;

in FragIn {
    vec4 color;
} fragIn;


void main()
{
    outColor = fragIn.color;
}