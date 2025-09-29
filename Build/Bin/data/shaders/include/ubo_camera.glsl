// this is defined globally and contains information about the camera

layout(set = 0, binding = 0) uniform ubo_camera
{
    mat4 view;
    mat4 viewInverse;
    mat4 proj;
} camera;