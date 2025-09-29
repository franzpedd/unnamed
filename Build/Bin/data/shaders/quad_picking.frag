#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_gpu_shader_int64 : enable

// includes
#include "include/ubo_camera.glsl"
#include "include/push_constant.glsl"

// this is here while I don't create separate descriptors for picking render pass
layout(set = 0, binding = 2) uniform sampler2D colorMapSampler;

// fragment output color
layout(location = 0) out uint outColor;

// entrypoint
void main()
{
    outColor = pushConstant.id;
}