#include "Common.hlsli"

#ifndef FOG_STEPS
#define FOG_STEPS 16
#endif

sampler2D g_depthTex : register(s0);
sampler3D g_noise : register(s1);
sampler2D g_band0 : register(s2);
sampler2D g_band1 : register(s3);
sampler2D g_band2 : register(s4);

row_major float4x4 g_invView : register(c8);
float4 g_offsetTime : register(c12);  // xyz: render to absolute world offset, w: unused
float4 g_fogColorEnd : register(c13); // rgb: game fog color, w: game fog end
float4 g_sunColorG : register(c14);   // rgb: sun color * glow, w: HG anisotropy
float4 g_toSun : register(c15);       // xyz: view space direction toward the sun
float4 g_density : register(c16);     // x: density, y: height falloff, z: base height, w: noise strength
float4 g_windWeight : register(c17);  // xyz: noise offset (uv units), w: layer weight
float4 g_misc : register(c18);        // x: far clip, y: frame index, z: god ray strength
float4 g_shadow[9] : register(c20);   // per band: x, y, depth rows

static const float NOISE_SCALE = 0.01;

float HenyeyGreenstein(float c, float g) {
    float g2 = g * g;
    return (1.0 - g2) / pow(max(1.0 + g2 - 2.0 * g * c, 1e-4), 1.5);
}

#ifdef GODRAYS
float ShadowTap(sampler2D s, float2 uv, float d) {
#if GODRAYS_HWPCF
    return tex2Dproj(s, float4(uv, d, 1.0)).r;
#else
    return d <= tex2Dlod(s, float4(uv, 0.0, 0.0)).r ? 1.0 : 0.0;
#endif
}

float InBand(float2 uv) {
    float2 m = step(0.02, uv) * step(uv, 0.98);
    return m.x * m.y;
}

float ShadowVis(float3 pv) {
    float4 p = float4(pv, 1.0);
    float2 uv0 = float2(dot(p, g_shadow[0]), dot(p, g_shadow[1]));
    float2 uv1 = float2(dot(p, g_shadow[3]), dot(p, g_shadow[4]));
    float2 uv2 = float2(dot(p, g_shadow[6]), dot(p, g_shadow[7]));
    float v0 = ShadowTap(g_band0, uv0, dot(p, g_shadow[2]));
    float v1 = ShadowTap(g_band1, uv1, dot(p, g_shadow[5]));
    float v2 = ShadowTap(g_band2, uv2, dot(p, g_shadow[8]));
    float m0 = InBand(uv0);
    float m1 = (1.0 - m0) * InBand(uv1);
    float m2 = (1.0 - m0 - m1) * InBand(uv2);
    return m0 * v0 + m1 * v1 + m2 * v2 + (1.0 - m0 - m1 - m2);
}
#endif

float4 FogMarchPS(float2 uv : TEXCOORD0, float2 vpos : VPOS) : COLOR0 {
    float2 o = g_screen.xy * 0.25;
    float z = min(min(tex2D(g_depthTex, uv + float2(o.x, o.y)).r, tex2D(g_depthTex, uv + float2(-o.x, o.y)).r),
                  min(tex2D(g_depthTex, uv + float2(o.x, -o.y)).r, tex2D(g_depthTex, uv + float2(-o.x, -o.y)).r));
    bool sky = z >= 0.999999;
    float3 P = ViewPosFromRaw(uv, min(z, 0.99999));
    float dist = length(P);
    float3 dir = P / dist;
    float farClip = g_misc.x;
    float len = sky ? farClip : min(dist, farClip);
    float fogEnd = max(g_fogColorEnd.w, 1.0);

    float jitter = InterleavedGradient(vpos, g_misc.y);
    float dt = len / FOG_STEPS;
    float phase = HenyeyGreenstein(dot(dir, g_toSun.xyz), g_sunColorG.w) * 0.25;
    float T = 1.0;
    float3 L = float3(0.0, 0.0, 0.0);

    [unroll]
    for (int i = 0; i < FOG_STEPS; i++) {
        float t = (i + jitter) * dt;
        float3 pv = dir * t;
        float3 wp = mul(float4(pv, 1.0), g_invView).xyz + g_offsetTime.xyz;

        float sigma = g_density.x * exp(-g_density.y * max(wp.z - g_density.z, 0.0));
        float n = tex3Dlod(g_noise, float4(wp * NOISE_SCALE + g_windWeight.xyz, 0.0)).r;
        sigma *= lerp(1.0, n, g_density.w);
        sigma *= 1.0 - smoothstep(0.6 * fogEnd, fogEnd, t);
        sigma *= g_windWeight.w;

        float vis = 1.0;
#ifdef GODRAYS
        vis = lerp(1.0, ShadowVis(pv), g_misc.z);
#endif
        float3 S = g_fogColorEnd.rgb + g_sunColorG.rgb * phase * vis;
        float ext = exp(-sigma * dt);
        L += T * S * (1.0 - ext);
        T *= ext;
    }

    return float4(saturate(L), T);
}

sampler2D g_cur : register(s0);
sampler2D g_hist : register(s1);
sampler2D g_depthTemporal : register(s2);
row_major float4x4 g_prevViewProj : register(c40);
float4 g_temporal : register(c44); // x: history weight (0 when invalid)

float4 FogTemporalPS(float2 uv : TEXCOORD0) : COLOR0 {
    float4 cur = tex2D(g_cur, uv);
    float2 texel = g_screen.xy;

    float4 mn = cur;
    float4 mx = cur;

    [unroll]
    for (int y = -1; y <= 1; y++) {
        [unroll]
        for (int x = -1; x <= 1; x++) {
            float4 c = tex2D(g_cur, uv + float2(x, y) * texel);
            mn = min(mn, c);
            mx = max(mx, c);
        }
    }

    float z = tex2D(g_depthTemporal, uv).r;
    float3 P = ViewPosFromRaw(uv, min(z, 0.99999));
    float3 wp = mul(float4(P, 1.0), g_invView).xyz + g_offsetTime.xyz;
    float4 pc = mul(float4(wp, 1.0), g_prevViewProj);
    float2 prevUv = float2(pc.x / pc.w * 0.5 + 0.5, 0.5 - pc.y / pc.w * 0.5);
    float2 inside = step(0.0, prevUv) * step(prevUv, 1.0);
    float valid = pc.w > 0.0 ? inside.x * inside.y : 0.0;

    float4 hist = clamp(tex2D(g_hist, prevUv), mn, mx);
    return lerp(cur, hist, g_temporal.x * valid);
}

sampler2D g_fog : register(s1);
sampler2D g_fogDepth : register(s2);
float4 g_fogSize : register(c30);

float4 FogApplyPS(float2 uv : TEXCOORD0) : COLOR0 {
    float z = tex2D(g_depthTex, uv).r;
    float w = RawToLinear(z);
    float2 p = uv * g_fogSize.xy - 0.5;
    float2 base = floor(p);
    float2 f = p - base;
    float4 sum = float4(0.0, 0.0, 0.0, 0.0);
    float total = 0.0;

    [unroll]
    for (int i = 0; i < 4; i++) {
        float2 o = float2(fmod((float)i, 2.0), floor(i * 0.5));
        float2 tuv = (base + o + 0.5) / g_fogSize.xy;
        float2 bw = lerp(1.0 - f, f, o);
        float ws = tex2D(g_fogDepth, tuv).r;
        float weight = bw.x * bw.y / (0.01 + abs(ws - w) / (0.05 * min(w, 2000.0) + 0.05));
        sum += tex2D(g_fog, tuv) * weight;
        total += weight;
    }

    return sum / max(total, 1e-6);
}
