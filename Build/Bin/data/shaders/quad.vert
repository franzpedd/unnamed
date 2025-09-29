#version 460
#extension GL_GOOGLE_include_directive : enable

// includes
#include "include/fun.glsl"
#include "include/primitives.glsl"
#include "include/ubo_camera.glsl"
#include "include/ubo_sprite.glsl"
#include "include/push_constant.glsl"

// output vertex attributes
layout(location = 0) out vec2 outFragTexCoord;

// returns the correct uv orientation if billboard is active
vec2 GetCorrectedUV()
{
    vec2 frag = SquareUVs[gl_VertexIndex];

    // we must invert the sprite to correctly face the camera if it is a billboard
    if(spriteParams.billboard == 1) 
    {
        // both locked, don't flip uv
        if(spriteParams.lockAxis.x == 1.0 && spriteParams.lockAxis.y == 1.0){
            return frag;
        }

        // x axis is locked, flip X, Y and rotate
        else if(spriteParams.lockAxis.x == 1.0) {
            vec2 uv = vec2(1.0 - SquareUVs[gl_VertexIndex].x, 1.0 - SquareUVs[gl_VertexIndex].y);
            frag = RotateUV(uv, radians(90.0));
        }

        // y axis is locked, flip X
        else if(spriteParams.lockAxis.y == 1.0) {
            frag = vec2(1.0 - SquareUVs[gl_VertexIndex].x, SquareUVs[gl_VertexIndex].y);
        }

        // not locked, flip X
        else {
            frag = vec2(1.0 - SquareUVs[gl_VertexIndex].x, SquareUVs[gl_VertexIndex].y);
        }
    }

    return frag;
}

// entrypoint
void main()
{
    mat4 billboard = GetBillboardMatrix(pushConstant.model, camera.view, spriteParams.lockAxis.x, spriteParams.lockAxis.y);
    gl_Position = camera.proj * camera.view * billboard * vec4(SquareVertices[gl_VertexIndex].xyz, 1.0);
    outFragTexCoord = GetCorrectedUV();
}