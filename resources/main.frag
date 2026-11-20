#version 330 core

in vec3 vNormal;
in vec2 vUV;

uniform vec3 sunDir;
uniform vec3 baseColor;
uniform bool emissive;

out vec4 FragColor;

void main() {
    if (emissive) {
        FragColor = vec4(baseColor, 1.0);
        return;
    }

    float diff = max(dot(normalize(vNormal), normalize(sunDir)), 0.0);
    FragColor = vec4(baseColor * diff, 1.0);
}