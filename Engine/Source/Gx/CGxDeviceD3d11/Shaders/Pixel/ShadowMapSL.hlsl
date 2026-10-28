// Rewritten from Shaders\Pixel\ps_3_0\ShadowMapSL.bls (16 permutations, 2 distinct) by Tools/Shaders/bls_to_hlsl.py.
#include "../Common.hlsli"

#if PERMUTATION == 0 || PERMUTATION == 1 || PERMUTATION == 2 || PERMUTATION == 3 || PERMUTATION == 4 || PERMUTATION == 5 || PERMUTATION == 6 || PERMUTATION == 7
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : TEXCOORD1;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d1 = float4(0, 1, 0, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.x = (c[0].wwww * v0.zzzz).x;
    oC0.xy = (max(r0.xxxx, d1.xxxx)).xy;
    oC0.zw = (d1.xyxy).zw;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 8 || PERMUTATION == 9 || PERMUTATION == 10 || PERMUTATION == 11 || PERMUTATION == 12 || PERMUTATION == 13 || PERMUTATION == 14 || PERMUTATION == 15
Texture2D tex0 : register(t0);
SamplerState smp0 : register(s0);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : TEXCOORD0;
    float4 v1 : TEXCOORD1;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d1 = float4(0, 1, 0, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v0.xyzw).xy)).xyzw;
    r0.xyzw = (r0.wwww + -c[2].wwww).xyzw;
    clip((r0.xyzw).xyz);
    r0.x = (c[0].wwww * v1.zzzz).x;
    oC0.xy = (max(r0.xxxx, d1.xxxx)).xy;
    oC0.zw = (d1.xyxy).zw;
    AlphaTest(oC0.w);
    return oC0;
}
#endif
