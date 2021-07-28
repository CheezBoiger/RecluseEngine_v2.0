#version 430
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout ( set = 0, binding = 0 ) uniform Test
{
    vec4 color;
    vec4 iter;
} test;


layout ( set = 0, binding = 1, rgba8 ) uniform image2D resultImg;


layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
    vec2 norm_coordinates = (gl_GlobalInvocationID.xy + vec2(0.5)) / vec2(imageSize(resultImg));
    vec2 c = (norm_coordinates - vec2(0.5)) * 2.0 - vec2(1.0, 0.0);

    vec2 z = vec2(0.0, 0.0);
    float i;
    for (i = 0.0; i < 1.0; i += test.iter.w) {
        z = vec2(
            z.x * z.x - z.y * z.y + c.x,
            z.y * z.x + z.x * z.y + c.y
        );

        if (length(z) > 4.0) {
            break;
        }
    }

    vec4 to_write = vec4(test.color.xyz * i, 1.0);
    imageStore(resultImg, ivec2(gl_GlobalInvocationID.xy), to_write);
}