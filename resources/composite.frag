#version 330 core
in vec2 uv;

uniform sampler2D scene;

uniform vec2 resolution;

uniform float time;
uniform float warpPx;
uniform float cellPx;
uniform float dispersionPx;
uniform float exposure;
uniform float vignette;
uniform float focusSepPx;

uniform bool fieldStop;

out vec4 FragColor;

float hash(vec2 p) {
    return fract(
            sin(
                    dot(
                            p,
                            vec2(127.1, 311.7)
                    )
            ) * 43758.5453
    );
}

float vnoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);

    vec2 u = f * f * (3.0f - 2.0f * f);

    return mix(
            mix(
                hash(i),
                hash(i + vec2(1, 0)),
                u.x
            ),
            mix(
                hash(i + vec2(0, 1)),
                hash(i + vec2(1, 1)),
                u.x
            ),
            u.y
    );
}

vec2 turb(vec2 p, float t) {
    return vec2(
            vnoise(p + vec2(0.0f, t)),
            vnoise(p + vec2(31.7f, -t))
    ) - 0.5f;
}

vec3 aces(vec3 x){
    return clamp(
        (x * (2.51f * x + 0.03f)) / (x * (2.43 * x + 0.59f) + 0.14f),
        0.0f,
        1.0f
    );
}

void main() {
    vec2 px = uv * resolution;

    vec2 tilt = (vec2(vnoise(vec2(time * 7.0f, 1.0f)), vnoise(vec2(time * 7.0f, 9.0f))) - 0.5f) * 1.2f;
    vec2 local = turb(px / cellPx, time * 6.0f) * 2.0f + turb(px / (cellPx * 0.4f), time * 11);

    vec2 uvw = uv + (tilt + local) * warpPx / resolution;

    vec2 off = vec2(0.0f, dispersionPx) / resolution;
    vec2 sep = vec2(focusSepPx) / resolution;

    vec2 oR = vec2(0.0f, 1.0f) * sep + off;
    vec2 oG = vec2(-0.886f, -0.5f) * sep;
    vec2 oB = vec2(0.886f, -0.5f) * sep - off;

    vec3 col = vec3(texture(scene, uvw + oR).r, texture(scene, uvw + oG).g, texture(scene, uvw + oB).b);

    col = aces(col * exposure);

    vec2 q = (uv - 0.5f) * vec2(resolution.x / resolution.y, 1.0f) * 2.0f;

    float r = length(q);

    col *= 1.0f - vignette * r * r;

    if(fieldStop) {
        col *= 1.0f - smoothstep(0.985f, 1.0f, r);
    }

    FragColor = vec4(pow(col, vec3(1.0f / 2.2f)), 1.0f);
}