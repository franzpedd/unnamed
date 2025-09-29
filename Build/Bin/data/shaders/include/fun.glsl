// this holds a bunch of functions to facilitate shader-coding

// rotates the uv coordinates within the center
vec2 RotateUV(vec2 uv, float angle) {
    vec2 center = vec2(0.5, 0.5);
    uv -= center;

    float xCos = cos(angle);
    float xSin = sin(angle);
    mat2 rot = mat2(xCos, -xSin, xSin, xCos);

    uv = rot * uv;
    uv += center;
    return uv;
}

// applies the rotation, scale and translation of the uv coordinates
vec2 TransformUV(vec2 uv, vec2 uvOffset, vec2 uvScale, float angle) {
    uv = RotateUV(uv, angle);
    uv = (uv + uvOffset) * uvScale;
    return uv;
}

// returns a billboard matrix for the given camera and model matrices, applys axis locking if enabled
mat4 GetBillboardMatrix(const mat4 model, const mat4 view, float lockX, float lockY) {

    // retrieve scale from model matrix
    vec3 scale = vec3(length(model[0].xyz), length(model[1].xyz), length(model[2].xyz));
    vec3 modelTranslation = model[3].xyz;

    if(lockX == 1.0 && lockY == 1.0) return model;

    // default camera vectors
    vec3 worldUp = vec3(0.0, 1.0, 0.0);
    vec3 cameraForward = normalize(vec3(view[0][2], view[1][2], view[2][2]));

    // lock X-axis: only rotate around X-axis (vertical billboard)
    if(lockX == 1.0) {
        vec3 cameraForwardXZ = normalize(vec3(0.0, cameraForward.y, cameraForward.z));
        vec3 cameraRight = vec3(1.0, 0.0, 0.0);
        vec3 cameraUp = normalize(cross(cameraRight, cameraForwardXZ));
        return mat4(vec4(cameraRight * scale.x, 0.0), vec4(cameraUp * scale.y, 0.0), vec4(cameraForwardXZ * scale.z, 0.0), vec4(modelTranslation, 1.0));
    }

    // lock Y-axis: only rotate around Y-axis (horizontal billboard)
    else if(lockY == 1.0) {
        vec3 cameraForwardYZ = normalize(vec3(cameraForward.x, 0.0, cameraForward.z));
        vec3 cameraRight = normalize(cross(worldUp, cameraForwardYZ));
        vec3 cameraUp = vec3(0.0, 1.0, 0.0); // Fixed up vector
        return mat4(vec4(cameraRight * scale.x, 0.0), vec4(cameraUp * scale.y, 0.0), vec4(cameraForwardYZ * scale.z, 0.0), vec4(modelTranslation, 1.0));
    }

    // full billboard (no axis locking)
    vec3 cameraRight = normalize(cross(worldUp, cameraForward));
    vec3 cameraUp = normalize(cross(cameraForward, cameraRight));
    return mat4(vec4(cameraRight * scale.x, 0.0), vec4(cameraUp * scale.y, 0.0), vec4(cameraForward * scale.z, 0.0), vec4(modelTranslation, 1.0));
}