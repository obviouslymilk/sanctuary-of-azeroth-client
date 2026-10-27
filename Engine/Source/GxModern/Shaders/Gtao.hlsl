#include "Common.hlsli"

sampler2D g_depthTex : register(s0);
sampler2D g_linDepth : register(s1);
sampler2D g_aoTex : register(s2);

float4 g_params : register(c8);
// LinearDepthPS: xy = 1/size of the INTZ source, z = 1 when halving
// GtaoPS: x = radius yards, y = pixels per yard at unit depth, z = max radius px
// AoApplyPS: x = strength, zw = AO texture size


float4 LinearDepthPS(float2 uv : TEXCOORD0) : COLOR0 {
    float z0 = tex2D(g_depthTex, uv).r;

    if (g_params.z < 0.5) {
        return RawToLinear(z0).xxxx;
    }

    float2 o = g_params.xy * 0.5;
    float z1 = tex2D(g_depthTex, uv + float2(o.x, o.y)).r;
    float z2 = tex2D(g_depthTex, uv + float2(-o.x, o.y)).r;
    float z3 = tex2D(g_depthTex, uv + float2(o.x, -o.y)).r;
    return RawToLinear(min(min(z0, z1), min(z2, z3))).xxxx;
}

float3 LoadPos(float2 uv) {
    float w = tex2Dlod(g_linDepth, float4(uv, 0.0, 0.0)).r;
    return ViewPosFromLinear(uv, w);
}

float4 GtaoPS(float2 uv : TEXCOORD0, float2 vpos : VPOS) : COLOR0 {
#ifdef GTAO_FULL
    static const int SLICES = 3;
    static const int STEPS = 6;
#else
    static const int SLICES = 2;
    static const int STEPS = 4;
#endif
    static const float PI = 3.14159265;
    static const float HALF_PI = 1.57079633;

    float w = tex2D(g_linDepth, uv).r;

    if (w >= SKY_DEPTH * 0.5) {
        return float4(1.0, 1.0, 1.0, 1.0);
    }

    float2 texel = g_screen.xy;
    float3 P = ViewPosFromLinear(uv, w);

    float3 pL = LoadPos(uv - float2(texel.x, 0.0));
    float3 pR = LoadPos(uv + float2(texel.x, 0.0));
    float3 pU = LoadPos(uv - float2(0.0, texel.y));
    float3 pD = LoadPos(uv + float2(0.0, texel.y));
    float3 dx = length(pL - P) < length(pR - P) ? P - pL : pR - P;
    float3 dy = length(pU - P) < length(pD - P) ? P - pU : pD - P;
    float3 V = normalize(-P);
    float3 N = cross(dx, dy);
    N /= max(length(N), 1e-8);
    N = dot(N, V) < 0.0 ? -N : N;

    float radius = g_params.x;
    float radiusPx = min(g_params.y * radius / w, g_params.z);

    if (radiusPx < 1.0) {
        return float4(1.0, 1.0, 1.0, 1.0);
    }

    float noiseSlice = Bayer4(vpos);
    float noiseStep = frac(Bayer4(vpos.yx + 1.0) + noiseSlice * 0.618);
    float visibility = 0.0;

    [unroll]
    for (int slice = 0; slice < SLICES; slice++) {
        float phi = (slice + noiseSlice) / SLICES * PI;
        float2 dir2 = float2(cos(phi), sin(phi));

        float3 probe = LoadPos(uv + dir2 * texel * 4.0);
        float3 d3 = probe - P;
        d3 = d3 - V * dot(d3, V);
        d3 /= max(length(d3), 1e-8);
        float3 axis = cross(d3, V);

        float3 projN = N - axis * dot(N, axis);
        float projLen = length(projN);
        float cosN = clamp(dot(projN, V) / max(projLen, 1e-5), -1.0, 1.0);
        float n = (dot(d3, projN) < 0.0 ? -1.0 : 1.0) * acos(cosN);

        float lowCos0 = cos(n + HALF_PI);
        float lowCos1 = cos(n - HALF_PI);
        float hCos0 = lowCos0;
        float hCos1 = lowCos1;

        [unroll]
        for (int s = 0; s < STEPS; s++) {
            float t = (s + noiseStep) / STEPS;
            t *= t;
            float2 off = dir2 * max(t * radiusPx, 1.0 + s) * texel;

            float3 S0 = LoadPos(uv + off) - P;
            float3 S1 = LoadPos(uv - off) - P;
            float l0 = length(S0);
            float l1 = length(S1);
            float f0 = saturate((l0 - radius * 0.6) / (radius * 0.4));
            float f1 = saturate((l1 - radius * 0.6) / (radius * 0.4));
            float c0 = lerp(dot(S0 / max(l0, 1e-5), V), lowCos0, f0);
            float c1 = lerp(dot(S1 / max(l1, 1e-5), V), lowCos1, f1);
            hCos0 = max(hCos0, c0);
            hCos1 = max(hCos1, c1);
        }

        float h0 = -acos(clamp(hCos1, -1.0, 1.0));
        float h1 = acos(clamp(hCos0, -1.0, 1.0));
        h0 = n + clamp(h0 - n, -HALF_PI, HALF_PI);
        h1 = n + clamp(h1 - n, -HALF_PI, HALF_PI);

        float sinN = sin(n);
        float arc0 = (cosN + 2.0 * h0 * sinN - cos(2.0 * h0 - n)) * 0.25;
        float arc1 = (cosN + 2.0 * h1 * sinN - cos(2.0 * h1 - n)) * 0.25;
        visibility += projLen * (arc0 + arc1);
    }

    float ao = saturate(visibility / SLICES);
    ao = isnan(ao) ? 1.0 : ao;
    return float4(ao, ao, ao, ao);
}

float4 AoBlurPS(float2 uv : TEXCOORD0) : COLOR0 {
    float2 texel = g_screen.xy;
    float wc = tex2D(g_linDepth, uv).r;

    if (wc >= SKY_DEPTH * 0.5) {
        return float4(1.0, 1.0, 1.0, 1.0);
    }

    float sum = 0.0;
    float total = 0.0;

    [unroll]
    for (int y = -1; y <= 2; y++) {
        [unroll]
        for (int x = -1; x <= 2; x++) {
            float2 tuv = uv + (float2(x, y) - 0.5) * texel;
            float ws = tex2D(g_linDepth, tuv).r;
            float weight = saturate(1.0 - abs(ws - wc) / (0.05 * wc + 0.05));
            sum += tex2D(g_aoTex, tuv).r * weight;
            total += weight;
        }
    }

    float ao = total > 1e-4 ? sum / total : tex2D(g_aoTex, uv).r;
    return float4(ao, ao, ao, ao);
}

float4 AoApplyPS(float2 uv : TEXCOORD0) : COLOR0 {
    float z = tex2D(g_depthTex, uv).r;

    if (z >= 0.999999) {
        return float4(1.0, 1.0, 1.0, 1.0);
    }

    float w = RawToLinear(z);
    float2 aoSize = g_params.zw;
    float2 p = uv * aoSize - 0.5;
    float2 base = floor(p);
    float2 f = p - base;
    float sum = 0.0;
    float total = 0.0;

    [unroll]
    for (int i = 0; i < 4; i++) {
        float2 o = float2(fmod((float)i, 2.0), floor(i * 0.5));
        float2 tuv = (base + o + 0.5) / aoSize;
        float2 bw = lerp(1.0 - f, f, o);
        float ws = tex2D(g_linDepth, tuv).r;
        float weight = bw.x * bw.y / (0.01 + abs(ws - w) / (0.05 * w + 0.05));
        sum += tex2D(g_aoTex, tuv).r * weight;
        total += weight;
    }

    float ao = sum / max(total, 1e-6);
    float v = lerp(1.0, ao, g_params.x);
    return float4(v, v, v, 1.0);
}
