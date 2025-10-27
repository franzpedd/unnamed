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

// returns the correct uv orientation if billboard is active
vec2 GetCorrectedUV()
{
    vec2 frag = SquareUVs[gl_VertexIndex];
    frag = (frag * quadParams.uv_scale) + quadParams.uv_offset;

    if(quadParams.billboard == 1.0) 
    {
        if(quadParams.lockAxis.x == 1.0 && quadParams.lockAxis.y == 1.0) {
            frag = RotateUV(frag, radians(quadParams.uv_rotation));
            return frag;
        }

        else if(quadParams.lockAxis.x == 1.0) {
            frag = RotateUV(frag, radians(90.0 + quadParams.uv_rotation));
            frag.y = 1.0 - frag.y;
        }

        else if(quadParams.lockAxis.y == 1.0) {
            frag.x = 1.0 - frag.x;
            frag = RotateUV(frag, radians(quadParams.uv_rotation));
        }

        else {
            frag.x = 1.0 - frag.x;
            frag = RotateUV(frag, radians(quadParams.uv_rotation));
        }
    }

    else 
    {
        frag = RotateUV(frag, radians(quadParams.uv_rotation));
    }

    return frag;
}

// entrypoint
void main()
{
    mat4 billboard = GetBillboardMatrix(pushConstant.model, camera.view, quadParams.billboard, quadParams.lockAxis.x, quadParams.lockAxis.y);
    gl_Position = camera.proj * camera.view * billboard * vec4(SquareVertices[gl_VertexIndex].xyz, 1.0);
    outFragTexCoord = GetCorrectedUV();
}