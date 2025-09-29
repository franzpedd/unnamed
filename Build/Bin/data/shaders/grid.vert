#version 460
#extension GL_GOOGLE_include_directive : enable

// includes
#include "include/ubo_camera.glsl"

// output vertex attributes
layout(location = 1) out vec3 outNearPoint;
layout(location = 2) out vec3 outFarPoint;

// screen square
vec3 GridPlane[6] = vec3[]
(
    vec3(1, 1, 0),
    vec3(-1, -1, 0),
    vec3(-1, 1, 0),

    vec3(-1, -1, 0),
    vec3(1, 1, 0),
    vec3(1, -1, 0)
);

// unprojects points to infinity
vec3 UnprojectPoint(float x, float y, float z, mat4 view, mat4 projection)
{
    vec4 unprojectedPoint =  inverse(view) * inverse(projection) * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

// entrypoint
void main()
{
    vec3 point = GridPlane[gl_VertexIndex].xyz;

    // unproject the near plane to infinity
    outNearPoint = UnprojectPoint(point.x, point.y, 0.0, camera.view, camera.proj).xyz;

    // unproject the far plane to infinity
    outFarPoint = UnprojectPoint(point.x, point.y, 1.0, camera.view, camera.proj).xyz;

    // using directly the clipped coordinates
    gl_Position = vec4(point, 1.0);
}