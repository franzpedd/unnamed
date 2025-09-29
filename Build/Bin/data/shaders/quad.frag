#version 460
#extension GL_GOOGLE_include_directive : enable

// includes
#include "include/fun.glsl"
#include "include/ubo_camera.glsl"
#include "include/ubo_sprite.glsl"
#include "include/push_constant.glsl"

// mesh samplers
layout(set = 0, binding = 2) uniform sampler2D colorMapSampler;

// input fragment attributes
layout(location = 0) in vec2 inFragTexCoord;

// output fragment color
layout(location = 0) out vec4 outColor;

// entrypoint
void main()
{
    outColor = texture(colorMapSampler, TransformUV(inFragTexCoord, spriteParams.uv_offset, spriteParams.uv_scale, radians(spriteParams.uv_rotation)));

    // discard full transparent pixels
    if(outColor.a == 0.0) {
        discard;
    }
}