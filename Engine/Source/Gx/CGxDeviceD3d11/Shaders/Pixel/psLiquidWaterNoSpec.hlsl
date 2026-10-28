// Rewritten from Shaders\Pixel\ps_3_0\psLiquidWaterNoSpec.bls (1 permutations, 1 distinct) by Tools/Shaders/bls_to_hlsl.py.
#include "../Common.hlsli"

#if PERMUTATION == 0
Texture2D tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : TEXCOORD0;
    float4 v2 : TEXCOORD1;
    float4 v3 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(1, 0, 0, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex1.Sample(smp1, (v2.xyzw).xy)).xyzw;
    r0.xyzw = (r0.xyzx * d0.xxxy).xyzw;
    r1.xyzw = (tex0.Sample(smp0, (v1.xyzw).xy)).xyzw;
    r0.xyzw = (v0.xyzw * r1.xyzw + r0.xyzw).xyzw;
    r0.xyz = (r0.xyzw + -c[4].xyzw).xyz;
    oC0.w = (r0.wwww).w;
    oC0.xyz = (v3.xxxx * r0.xyzw + c[4].xyzw).xyz;
    AlphaTest(oC0.w);
    return oC0;
}
#endif
