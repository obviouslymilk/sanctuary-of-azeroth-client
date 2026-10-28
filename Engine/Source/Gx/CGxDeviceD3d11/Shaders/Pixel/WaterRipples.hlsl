// Rewritten from Shaders\Pixel\ps_3_0\WaterRipples.bls (1 permutations, 1 distinct) by Tools/Shaders/bls_to_hlsl.py.
#include "../Common.hlsli"

#if PERMUTATION == 0
Texture2D tex0 : register(t0);
SamplerState smp0 : register(s0);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : TEXCOORD0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v1.xyzw).xy)).xyzw;
    oC0.w = (r0.wwww * v0.wwww).w;
    oC0.xyz = (r0.xyzw).xyz;
    AlphaTest(oC0.w);
    return oC0;
}
#endif
