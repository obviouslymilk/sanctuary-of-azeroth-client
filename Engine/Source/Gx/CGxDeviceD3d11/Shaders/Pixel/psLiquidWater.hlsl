// Rewritten from Shaders\Pixel\ps_3_0\psLiquidWater.bls (1 permutations, 1 distinct) by Tools/Shaders/bls_to_hlsl.py.
#include "../Common.hlsli"

#if PERMUTATION == 0
Texture2D tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(0.25, 0, 0, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyz = (d0.xxxx + v1.xyzw).xyz;
    r1.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r0.xyz = (r0.xyzw * r1.wwww).xyz;
    r2.xyzw = (tex0.Sample(smp0, (v2.xyzw).xy)).xyzw;
    r3.w = (r2.wwww * v0.wwww).w;
    r3.xyz = (v0.xyzw * r2.xyzw + r1.xyzw).xyz;
    r0.w = (d0.yyyy).w;
    r0.xyzw = (r0.xyzw + r3.xyzw).xyzw;
    r0.xyz = (r0.xyzw + -c[4].xyzw).xyz;
    oC0.w = (r0.wwww).w;
    oC0.xyz = (v4.xxxx * r0.xyzw + c[4].xyzw).xyz;
    AlphaTest(oC0.w);
    return oC0;
}
#endif
