#version 330 core

in vec2 uv;

uniform sampler2D src;
uniform vec2 dir;
uniform float sigmaPx;

out vec4 FragColor;

void main() {
    if(sigmaPx < 0.25) {
        FragColor = texture(src, uv);
        return;
    }

    float step = max(1.0f, sigmaPx * 0.5f);

    vec4 sum = vec4(0.0f);
    float wsum = 0.0f;

    for(int k = -10; k <= 10; k++){
        float x = float(k) * step;
        float w = exp(-0.5 * (x * x) / (sigmaPx * sigmaPx));

        sum += texture(src, uv + dir * x) * w;
        wsum += w;
    }

    FragColor = sum / wsum;
}