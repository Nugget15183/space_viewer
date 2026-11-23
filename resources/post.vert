#version 330 core

out vec2 uv;

void main() {
    vec2 p = vec2(float((gl_VertexID << 1) & 2), float(gl_VertexID & 2));
    uv = p;
    gl_Position = vec4(p * 2.0f - 1.0f, 0.0f, 1.0f);
}