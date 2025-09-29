// this is included per mesh/drawable and contains information about the material

layout(set = 0, binding = 1) uniform ubo_sprite
{
    uint billboard;
    float uv_rotation;
    vec2 lockAxis;
    vec2 uv_offset;
    vec2 uv_scale;
} spriteParams;