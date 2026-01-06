#version 460
#extension GL_GOOGLE_include_directive : enable

// includes
#include "include/fun.glsl"
#include "include/primitives.glsl"
#include "include/ubo_camera.glsl"
#include "include/ubo_quad.glsl"
#include "include/push_constant.glsl"

// output vertex attributes
layout(location = 0) out vec2 outFragTexCoord;

// entrypoint
void main()
{
    mat4 billboard = GetBillboardMatrix(pushConstant.model, camera.view, quadParams.billboard, quadParams.lockAxis.x, quadParams.lockAxis.y);
    gl_Position = camera.proj * camera.view * billboard * vec4(SquareVertices[gl_VertexIndex].xyz, 1.0);
}