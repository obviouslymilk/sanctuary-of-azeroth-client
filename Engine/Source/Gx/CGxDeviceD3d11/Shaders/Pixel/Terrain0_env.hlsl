// Rewritten from Shaders\Pixel\ps_3_0\Terrain0_env.bls (1 permutations, 1 distinct) by Tools/Shaders/bls_to_hlsl.py.
#include "../Common.hlsli"

#if PERMUTATION == 0
TextureCube tex0 : register(t0);
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
    const float4 d0 = float4(0.300000012, 0.699999988, 1, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v2.xyzw).xyz)).xyzw;
    r1.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r2.x = (r1.wwww * d0.xxxx + d0.yyyy).x;
    r0.xyz = (r0.xyzw * r2.xxxx).xyz;
    r2.xyz = (r0.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.xyzw * v0.xyzw).xyz;
    r0.xyz = (r0.xyzw + r0.xyzw).xyz;
    r0.xyz = (r2.xyzw * r1.wwww + r0.xyzw).xyz;
    r0.w = saturate(dot((r1.xyzw).xyz, (d0.zzzz).xyz));
    r0.w = (-r0.wwww + d0.zzzz).w;
    r0.xyz = (r0.xyzw * r0.wwww + -c[2].xyzw).xyz;
    oC0.xyz = (v4.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.zzzz).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif
