// this holds information about primitive objects

// square vertices
vec3 SquareVertices[6] = vec3[](
    // First triangle
    vec3(-0.5, -0.5, 0.0),  // bottom-left
    vec3( 0.5, -0.5, 0.0),  // bottom-right  
    vec3( 0.5,  0.5, 0.0),  // top-right
    
    // Second triangle
    vec3( 0.5,  0.5, 0.0),  // top-right
    vec3(-0.5,  0.5, 0.0),  // top-left
    vec3(-0.5, -0.5, 0.0)   // bottom-left
);

// square uv coordinates
vec2 SquareUVs[6] = vec2[](
    // First triangle - bottom-left, bottom-right, top-right
    vec2(0.0, 0.0),  // bottom-left
    vec2(1.0, 0.0),  // bottom-right  
    vec2(1.0, 1.0),  // top-right
    
    // Second triangle - top-right, top-left, bottom-left
    vec2(1.0, 1.0),  // top-right
    vec2(0.0, 1.0),  // top-left
    vec2(0.0, 0.0)   // bottom-left
);