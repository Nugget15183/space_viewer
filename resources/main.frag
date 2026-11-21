#version 330 core

in vec3 vNormal;
in vec2 vUV;

uniform vec3 sunDir;
uniform vec3 baseColor;
uniform bool emissive;
uniform sampler2D tex;
uniform bool isRing;

out vec4 FragColor;

void main() {
    vec4 albedo = texture(tex, vUV).rgba * vec4(baseColor, 1.0);

    if(isRing) {
        if (albedo.a < 0.1f) discard;

        float lit = abs(dot(normalize(vNormal), normalize(sunDir)));

        FragColor = vec4(albedo.rgb * lit, albedo.a);
        return;
    }

    if (emissive) {
        FragColor = albedo;
        return;
    }

    float diff = max(dot(normalize(vNormal), normalize(sunDir)), 0.0);
    FragColor = vec4(albedo.rgb * diff, 1.0);
}