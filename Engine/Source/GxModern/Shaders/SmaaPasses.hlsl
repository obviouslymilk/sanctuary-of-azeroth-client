float4 g_rt : register(c0); // 1/width, 1/height, width, height

#define SMAA_RT_METRICS g_rt
#define SMAA_HLSL_3 1
#define SMAA_PRESET_HIGH 1
#define SMAA_INCLUDE_VS 1
#define SMAA_INCLUDE_PS 1

#include "SMAA.hlsl"

sampler2D g_colorTex : register(s0);
sampler2D g_edgesTex : register(s1);
sampler2D g_areaTex : register(s2);
sampler2D g_searchTex : register(s3);
sampler2D g_weightsTex : register(s5);

struct EdgeVsOut {
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
    float4 offset0 : TEXCOORD1;
    float4 offset1 : TEXCOORD2;
    float4 offset2 : TEXCOORD3;
};

struct WeightsVsOut {
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
    float2 pixcoord : TEXCOORD1;
    float4 offset0 : TEXCOORD2;
    float4 offset1 : TEXCOORD3;
    float4 offset2 : TEXCOORD4;
};

struct BlendVsOut {
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
    float4 offset : TEXCOORD1;
};

float4 ClipPos(float2 pos) {
    return float4(pos.x - g_rt.x, pos.y + g_rt.y, 0.0, 1.0);
}

float2 ClipToUv(float2 pos) {
    return float2(pos.x * 0.5 + 0.5, 0.5 - pos.y * 0.5);
}

EdgeVsOut EdgeVS(float2 pos : POSITION) {
    EdgeVsOut o;
    float4 offset[3];
    o.pos = ClipPos(pos);
    o.uv = ClipToUv(pos);
    SMAAEdgeDetectionVS(o.uv, offset);
    o.offset0 = offset[0];
    o.offset1 = offset[1];
    o.offset2 = offset[2];
    return o;
}

float4 EdgePS(EdgeVsOut i) : COLOR0 {
    float4 offset[3] = { i.offset0, i.offset1, i.offset2 };
    return float4(SMAALumaEdgeDetectionPS(i.uv, offset, g_colorTex), 0.0, 0.0);
}

WeightsVsOut WeightsVS(float2 pos : POSITION) {
    WeightsVsOut o;
    float4 offset[3];
    o.pos = ClipPos(pos);
    o.uv = ClipToUv(pos);
    SMAABlendingWeightCalculationVS(o.uv, o.pixcoord, offset);
    o.offset0 = offset[0];
    o.offset1 = offset[1];
    o.offset2 = offset[2];
    return o;
}

float4 WeightsPS(WeightsVsOut i) : COLOR0 {
    float4 offset[3] = { i.offset0, i.offset1, i.offset2 };
    return SMAABlendingWeightCalculationPS(i.uv, i.pixcoord, offset, g_edgesTex, g_areaTex, g_searchTex, float4(0.0, 0.0, 0.0, 0.0));
}

BlendVsOut BlendVS(float2 pos : POSITION) {
    BlendVsOut o;
    o.pos = ClipPos(pos);
    o.uv = ClipToUv(pos);
    SMAANeighborhoodBlendingVS(o.uv, o.offset);
    return o;
}

float4 BlendPS(BlendVsOut i) : COLOR0 {
    return SMAANeighborhoodBlendingPS(i.uv, i.offset, g_colorTex, g_weightsTex);
}
