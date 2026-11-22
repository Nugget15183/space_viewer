#version 330 core

in vec3 vNormal;
in vec2 vUV;
in vec3 vLocal;

uniform vec3 sunDir;
uniform vec3 sunLocal;

uniform vec3 baseColor;
uniform bool emissive;
uniform sampler2D tex;
uniform sampler2D ringTex;

uniform bool isRing;
uniform bool hasRings;

//cant use 'flattening' as variable name
uniform float oblate;

uniform float ringIn;
uniform float ringOut;

out vec4 FragColor;

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

void main() {
    vec4 albedo = texture(tex, vUV).rgba * vec4(baseColor, 1.0f);
    vec3 N = normalize(vNormal);
    vec3 L = normalize(sunLocal);

    if(isRing) {
        if (albedo.a < 0.1f) discard;

        float lit = abs(dot(normalize(vNormal), normalize(sunDir))) * planetShadow(vLocal, L);

        FragColor = vec4(albedo.rgb * lit, albedo.a);
        return;
    }

    if (emissive) {
        FragColor = albedo;
        return;
    }

    float diff = max(dot(normalize(vNormal), normalize(sunDir)), 0.0f);
    if(hasRings){
        vec3 P = vec3(vLocal.x, vLocal.y * (1.0f - oblate), vLocal.z);
        diff *= ringShadow(P, L);
    }

    FragColor = vec4(albedo.rgb * diff, 1.0f);
}