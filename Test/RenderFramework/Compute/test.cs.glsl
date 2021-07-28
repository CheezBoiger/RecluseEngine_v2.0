#version 430
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout ( set = 0, binding = 0 ) uniform Test
{
    vec4 color;
    vec4 offset;
} test;


layout ( set = 0, binding = 1, rgba8 ) uniform image2D resultColor;


layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
    ivec2 iPixCoord = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
    vec4 color = vec4(0.0);
    imageStore(resultColor, iPixCoord, color);
}