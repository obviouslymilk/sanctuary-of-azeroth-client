// Rewritten from Shaders\Pixel\ps_3_0\psLiquidMagma.bls (1 permutations, 1 distinct) by Tools/Shaders/bls_to_hlsl.py.
#include "../Common.hlsli"

#if PERMUTATION == 0
Texture2D tex0 : register(t0);
SamplerState smp0 : register(s0);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : TEXCOORD0;
    float4 v2 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(1, 0, 0, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v1.xyzw).xy)).xyzw;
    r0.xyz = (v0.xyzw * r0.xyzw + -c[4].xyzw).xyz;
    oC0.xyz = (v2.xxxx * r0.xyzw + c[4].xyzw).xyz;
    oC0.w = (d0.xxxx).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif
