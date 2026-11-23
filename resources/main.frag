#version 330 core

in vec3 vNormal;
in vec2 vUV;
in vec3 vLocal;

uniform vec3 sunDir;
uniform vec3 sunLocal;
uniform vec3 viewUnit;

uniform vec3 baseColor;
uniform bool emissive;
uniform sampler2D tex;
uniform sampler2D ringTex;

uniform bool isAtmo;
uniform bool isRing;
uniform bool hasRings;

//cant use 'flattening' as variable name
uniform float oblate;
uniform float limbDark;
uniform float ringIn;
uniform float ringOut;

uniform vec3 atmoColor;
uniform float atmoH;
uniform float atmoDensity;
uniform float atmoG;

out vec4 FragColor;

float SUN_BOOST = 6.0;

float ringShadow(vec3 P, vec3 L) {
    if(abs(L.y) < 1e-4) return 1.0f;

    float t = -P.y / L.y;

    if(t <= 0.0f) return 1.0f;

    float r = length((P + t * L).xz);

    if(r < ringIn || r > ringOut) return 1.0f;

    float a = textureLod(ringTex, vec2((r - ringIn) / (ringOut - ringIn), 0.5f), 0.0f).a;

    return 1.0f - a;
}

float planetShadow(vec3 P, vec3 L) {
    float oI = 1.0f - oblate;

    vec3 Pp = vec3(P.x, P.y / oI, P.z);
    vec3 Lp = normalize(vec3(L.x, L.y / oI, L.z));

    float proj = dot(Pp, Lp);
    if(proj >= 0.0f) return 1.0f;

    float d = sqrt(max(dot(Pp, Pp) - proj * proj, 0.0f));

    return smoothstep(1.0f - 0.004f, 1.0f + 0.004f, d);
}

vec4 atmosphere(vec3 L) {
    float R1 = 1.0f + atmoH;

    vec3 Q = normalize(vLocal) * R1;
    vec3 v = normalize(viewUnit);

    vec3 sunU = normalize(vec3(L.x, L.y / (1.0f - oblate), L.z));

    float qv = dot(Q, v);
    float b2 = max(dot(Q, Q) - qv * qv, 0.0f);
    float sEnd = (b2 < 1.0f) ? qv - sqrt(1.0f - b2) : 2.0f * qv;

    if(sEnd <= 0.0f) return vec4(0.0f);

    float H = atmoH * 0.25f;

    const int steps = 14;

    float ds = sEnd / float(steps);
    float column = 0.0f;

    for (int i = 0; i < steps; i++){
        vec3 P = Q - v * ((float(i) + 0.5f) * ds);

        float h = max(length(P) - 1.0f, 0.0f);
        float lit = smoothstep(-0.15f, 0.35f, dot(normalize(P), sunU));

        column += exp(-h / H) * lit * ds;
    }

    float cosT = -dot(sunU, v);
    float g = atmoG;
    float hg = (1.0f - g * g) / pow(max(1.0f + g * g - 2.0f * g * cosT, 1e-3), 1.5f);
    float phase = mix(1.0f, hg, 0.5f);

    float I = 1.0f - exp(-atmoDensity * column * 12.0f * phase);
    return vec4(atmoColor * I, 1.0f);
}

void main() {
    vec3 L = normalize(sunLocal);

    if(isAtmo){
        FragColor = atmosphere(L);
        return;
    }

    vec4 albedo = texture(tex, vUV) * vec4(baseColor, 1.0f);

    if (isRing) {
        if (albedo.a < 0.1) discard;
        float lit = abs(dot(normalize(vNormal), normalize(sunDir))) * planetShadow(vLocal, L);

        FragColor = vec4(albedo.rgb * lit, albedo.a);

        return;
    }

    if (emissive) {
        FragColor = vec4(albedo.rgb * SUN_BOOST, 1.0);
        return;
    }

    vec3 Nu   = normalize(vLocal);
    vec3 sunU = normalize(vec3(L.x, L.y / (1.0 - oblate), L.z));

    float mu0 = max(dot(Nu, sunU), 0.0);
    float mu  = max(dot(Nu, normalize(viewUnit)), 1e-3);
    float shade = 2.0 * mu0 / (mu0 + mu);

    shade *= 1.0 - limbDark * (1.0 - mu);

    if (hasRings) {
        vec3 P = vec3(vLocal.x, vLocal.y * (1.0 - oblate), vLocal.z);
        shade *= ringShadow(P, L);
    }

    FragColor = vec4(albedo.rgb * shade, 1.0);
}