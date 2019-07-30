#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec2 FragCoords;

vec2 positions[6] = vec2[](
    vec2(-1.0f, 1.0f),
    vec2(-1.0f,-1.0f),
    vec2( 1.0f, 1.0f),
    vec2( 1.0f, 1.0f),
    vec2(-1.0f,-1.0f),
    vec2( 1.0f,-1.0f)
);

void main() {
    FragCoords = positions[gl_VertexIndex];
    gl_Position = vec4(FragCoords, 0.0, 1.0);
}
