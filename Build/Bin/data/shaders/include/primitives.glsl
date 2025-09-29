// this holds information about primitive objects

// square vertices
vec3 SquareVertices[6] = vec3[](
    vec3( 1,  1, 0), // top-right
    vec3(-1, -1, 0), // bottom-left
    vec3(-1,  1, 0), // top-left

    vec3(-1, -1, 0), // bottom-left
    vec3( 1,  1, 0), // top-right
    vec3( 1, -1, 0)  // bottom-right
);

// square uv coordinates
vec2 SquareUVs[6] = vec2[](
    vec2(0, 0), // top-left
    vec2(1, 1), // bottom-right
    vec2(1, 0), // top-right

    vec2(1, 1), // bottom-right
    vec2(0, 0), // top-left
    vec2(0, 1)  // bottom-left
);

// cube vertices
const vec3 CubeVertices[8] = vec3[](
    vec3( 1,  1,  1), // 0: top-right-front
    vec3(-1,  1,  1), // 1: top-left-front
    vec3(-1, -1,  1), // 2: bottom-left-front
    vec3( 1, -1,  1), // 3: bottom-right-front

    vec3( 1,  1, -1), // 4: top-right-back
    vec3(-1,  1, -1), // 5: top-left-back
    vec3(-1, -1, -1), // 6: bottom-left-back
    vec3( 1, -1, -1)  // 7: bottom-right-back
);

// cube indices
const int CubeIndices[36] = int[](
    // Front face
    0, 1, 2, // Triangle 1
    0, 2, 3, // Triangle 2

    // Back face
    4, 6, 5, // Triangle 1
    4, 7, 6, // Triangle 2

    // Left face
    1, 5, 6, // Triangle 1
    1, 6, 2, // Triangle 2

    // Right face
    0, 3, 7, // Triangle 1
    0, 7, 4, // Triangle 2

    // Top face
    0, 4, 5, // Triangle 1
    0, 5, 1, // Triangle 2

    // Bottom face
    2, 6, 7, // Triangle 1
    2, 7, 3  // Triangle 2
);