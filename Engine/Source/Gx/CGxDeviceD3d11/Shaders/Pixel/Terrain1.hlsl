// Rewritten from Shaders\Pixel\ps_3_0\Terrain1.bls (32 permutations, 32 distinct) by Tools/Shaders/bls_to_hlsl.py.
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
    const float4 d0 = float4(0.300000012, 0.699999988, 1, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r0.x = (r0.wwww * d0.xxxx + d0.yyyy).x;
    r1.xyzw = (tex0.Sample(smp0, (v2.xyzw).xy)).xyzw;
    r0.xyz = (r0.xxxx * r1.xyzw).xyz;
    r1.xyz = (r1.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.xyzw * v0.xyzw).xyz;
    r0.xyz = (r0.xyzw + r0.xyzw).xyz;
    r0.xyz = (r1.xyzw * r0.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v4.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.zzzz).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 1
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
    r0.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r0.x = (r0.wwww * d0.xxxx + d0.yyyy).x;
    r1.xyzw = (tex0.Sample(smp0, (v2.xyzw).xyz)).xyzw;
    r0.xyz = (r0.xxxx * r1.xyzw).xyz;
    r1.xyz = (r1.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.xyzw * v0.xyzw).xyz;
    r0.xyz = (r0.xyzw + r0.xyzw).xyz;
    r0.xyz = (r1.xyzw * r0.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v4.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.zzzz).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 2
Texture2D tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(0.300000012, 0.699999988, 1, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v2.xyzw).xy)).xyzw;
    r1.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r2.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r3.xyzw = (lerp(r0.xyzw, r1.xyzw, r2.xxxx)).xyzw;
    r0.x = (r2.wwww * d0.xxxx + d0.yyyy).x;
    r0.xyz = (r3.xyzw * r0.xxxx).xyz;
    r1.xyz = (r3.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.xyzw * v0.xyzw).xyz;
    r0.xyz = (r0.xyzw + r0.xyzw).xyz;
    r0.xyz = (r1.xyzw * r2.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v5.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.zzzz).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 3
TextureCube tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(0.300000012, 0.699999988, 1, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v2.xyzw).xyz)).xyzw;
    r1.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r2.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r3.xyzw = (lerp(r0.xyzw, r1.xyzw, r2.xxxx)).xyzw;
    r0.x = (r2.wwww * d0.xxxx + d0.yyyy).x;
    r0.xyz = (r3.xyzw * r0.xxxx).xyz;
    r1.xyz = (r3.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.xyzw * v0.xyzw).xyz;
    r0.xyz = (r0.xyzw + r0.xyzw).xyz;
    r0.xyz = (r1.xyzw * r2.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v5.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.zzzz).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 4
Texture2D tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
Texture2D tex3 : register(t3);
SamplerState smp3 : register(s3);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : TEXCOORD3;
    float4 v6 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 v6 = input.v6;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(0.300000012, 0.699999988, 1, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.xyzw = (tex0.Sample(smp0, (v2.xyzw).xy)).xyzw;
    r2.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r3.xyzw = (tex3.Sample(smp3, (v5.xyzw).xy)).xyzw;
    r4.xyzw = (lerp(r1.xyzw, r2.xyzw, r3.xxxx)).xyzw;
    r1.xyzw = (lerp(r4.xyzw, r0.xyzw, r3.yyyy)).xyzw;
    r0.x = (r3.wwww * d0.xxxx + d0.yyyy).x;
    r0.xyz = (r1.xyzw * r0.xxxx).xyz;
    r1.xyz = (r1.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.xyzw * v0.xyzw).xyz;
    r0.xyz = (r0.xyzw + r0.xyzw).xyz;
    r0.xyz = (r1.xyzw * r3.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v6.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.zzzz).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 5
TextureCube tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
Texture2D tex3 : register(t3);
SamplerState smp3 : register(s3);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : TEXCOORD3;
    float4 v6 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 v6 = input.v6;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(0.300000012, 0.699999988, 1, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.xyzw = (tex0.Sample(smp0, (v2.xyzw).xyz)).xyzw;
    r2.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r3.xyzw = (tex3.Sample(smp3, (v5.xyzw).xy)).xyzw;
    r4.xyzw = (lerp(r1.xyzw, r2.xyzw, r3.xxxx)).xyzw;
    r1.xyzw = (lerp(r4.xyzw, r0.xyzw, r3.yyyy)).xyzw;
    r0.x = (r3.wwww * d0.xxxx + d0.yyyy).x;
    r0.xyz = (r1.xyzw * r0.xxxx).xyz;
    r1.xyz = (r1.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.xyzw * v0.xyzw).xyz;
    r0.xyz = (r0.xyzw + r0.xyzw).xyz;
    r0.xyz = (r1.xyzw * r3.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v6.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.zzzz).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 6
Texture2D tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
Texture2D tex3 : register(t3);
SamplerState smp3 : register(s3);
Texture2D tex4 : register(t4);
SamplerState smp4 : register(s4);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : TEXCOORD3;
    float4 v6 : TEXCOORD4;
    float4 v7 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 v6 = input.v6;
    float4 v7 = input.v7;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(0.300000012, 0.699999988, 1, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r5 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex3.Sample(smp3, (v5.xyzw).xy)).xyzw;
    r1.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r2.xyzw = (tex0.Sample(smp0, (v2.xyzw).xy)).xyzw;
    r3.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r4.xyzw = (tex4.Sample(smp4, (v6.xyzw).xy)).xyzw;
    r5.xyzw = (lerp(r2.xyzw, r3.xyzw, r4.xxxx)).xyzw;
    r2.xyzw = (lerp(r5.xyzw, r1.xyzw, r4.yyyy)).xyzw;
    r1.xyzw = (lerp(r2.xyzw, r0.xyzw, r4.zzzz)).xyzw;
    r0.x = (r4.wwww * d0.xxxx + d0.yyyy).x;
    r0.xyz = (r1.xyzw * r0.xxxx).xyz;
    r1.xyz = (r1.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.xyzw * v0.xyzw).xyz;
    r0.xyz = (r0.xyzw + r0.xyzw).xyz;
    r0.xyz = (r1.xyzw * r4.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v7.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.zzzz).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 7
TextureCube tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
Texture2D tex3 : register(t3);
SamplerState smp3 : register(s3);
Texture2D tex4 : register(t4);
SamplerState smp4 : register(s4);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : TEXCOORD3;
    float4 v6 : TEXCOORD4;
    float4 v7 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 v6 = input.v6;
    float4 v7 = input.v7;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(0.300000012, 0.699999988, 1, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r5 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex3.Sample(smp3, (v5.xyzw).xy)).xyzw;
    r1.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r2.xyzw = (tex0.Sample(smp0, (v2.xyzw).xyz)).xyzw;
    r3.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r4.xyzw = (tex4.Sample(smp4, (v6.xyzw).xy)).xyzw;
    r5.xyzw = (lerp(r2.xyzw, r3.xyzw, r4.xxxx)).xyzw;
    r2.xyzw = (lerp(r5.xyzw, r1.xyzw, r4.yyyy)).xyzw;
    r1.xyzw = (lerp(r2.xyzw, r0.xyzw, r4.zzzz)).xyzw;
    r0.x = (r4.wwww * d0.xxxx + d0.yyyy).x;
    r0.xyz = (r1.xyzw * r0.xxxx).xyz;
    r1.xyz = (r1.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.xyzw * v0.xyzw).xyz;
    r0.xyz = (r0.xyzw + r0.xyzw).xyz;
    r0.xyz = (r1.xyzw * r4.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v7.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.zzzz).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 8
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
    const float4 d0 = float4(1, 0.300000012, 0.699999988, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v2.xyzw).xy)).xyzw;
    r1.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r1.x = saturate(dot((r1.xyzw).xyz, (d0.xxxx).xyz));
    r1.x = (-r1.xxxx + d0.xxxx).x;
    r0.xyzw = (r0.xyzw * r1.xxxx).xyzw;
    r1.x = (r1.wwww * d0.yyyy + d0.zzzz).x;
    r0.xyz = (r0.xyzw * r1.xxxx).xyz;
    r1.xyz = (r0.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.xyzw * v0.xyzw).xyz;
    r0.xyz = (r0.xyzw + r0.xyzw).xyz;
    r0.xyz = (r1.xyzw * r1.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v4.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.xxxx).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 9
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
    const float4 d0 = float4(1, 0.300000012, 0.699999988, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v2.xyzw).xyz)).xyzw;
    r1.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r1.x = saturate(dot((r1.xyzw).xyz, (d0.xxxx).xyz));
    r1.x = (-r1.xxxx + d0.xxxx).x;
    r0.xyzw = (r0.xyzw * r1.xxxx).xyzw;
    r1.x = (r1.wwww * d0.yyyy + d0.zzzz).x;
    r0.xyz = (r0.xyzw * r1.xxxx).xyz;
    r1.xyz = (r0.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.xyzw * v0.xyzw).xyz;
    r0.xyz = (r0.xyzw + r0.xyzw).xyz;
    r0.xyz = (r1.xyzw * r1.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v4.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.xxxx).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 10
Texture2D tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(1, 0.300000012, 0.699999988, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v2.xyzw).xy)).xyzw;
    r1.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.y = saturate(dot((r1.xyzw).xyz, (d0.xxxx).xyz));
    r1.y = (-r1.yyyy + d0.xxxx).y;
    r2.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r2.xyzw = (r1.xxxx * r2.xyzw).xyzw;
    r0.xyzw = (r0.xyzw * r1.yyyy + r2.xyzw).xyzw;
    r1.x = (r1.wwww * d0.yyyy + d0.zzzz).x;
    r0.xyz = (r0.xyzw * r1.xxxx).xyz;
    r1.xyz = (r0.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.xyzw * v0.xyzw).xyz;
    r0.xyz = (r0.xyzw + r0.xyzw).xyz;
    r0.xyz = (r1.xyzw * r1.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v5.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.xxxx).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 11
TextureCube tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(1, 0.300000012, 0.699999988, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v2.xyzw).xyz)).xyzw;
    r1.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.y = saturate(dot((r1.xyzw).xyz, (d0.xxxx).xyz));
    r1.y = (-r1.yyyy + d0.xxxx).y;
    r2.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r2.xyzw = (r1.xxxx * r2.xyzw).xyzw;
    r0.xyzw = (r0.xyzw * r1.yyyy + r2.xyzw).xyzw;
    r1.x = (r1.wwww * d0.yyyy + d0.zzzz).x;
    r0.xyz = (r0.xyzw * r1.xxxx).xyz;
    r1.xyz = (r0.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.xyzw * v0.xyzw).xyz;
    r0.xyz = (r0.xyzw + r0.xyzw).xyz;
    r0.xyz = (r1.xyzw * r1.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v5.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.xxxx).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 12
Texture2D tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
Texture2D tex3 : register(t3);
SamplerState smp3 : register(s3);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : TEXCOORD3;
    float4 v6 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 v6 = input.v6;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(1, 0.300000012, 0.699999988, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.xyzw = (tex0.Sample(smp0, (v2.xyzw).xy)).xyzw;
    r2.xyzw = (tex3.Sample(smp3, (v5.xyzw).xy)).xyzw;
    r2.z = saturate(dot((r2.xyzw).xyz, (d0.xxxx).xyz));
    r2.z = (-r2.zzzz + d0.xxxx).z;
    r3.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r3.xyzw = (r2.xxxx * r3.xyzw).xyzw;
    r1.xyzw = (r1.xyzw * r2.zzzz + r3.xyzw).xyzw;
    r0.xyzw = (r0.xyzw * r2.yyyy + r1.xyzw).xyzw;
    r1.x = (r2.wwww * d0.yyyy + d0.zzzz).x;
    r0.xyz = (r0.xyzw * r1.xxxx).xyz;
    r1.xyz = (r0.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.xyzw * v0.xyzw).xyz;
    r0.xyz = (r0.xyzw + r0.xyzw).xyz;
    r0.xyz = (r1.xyzw * r2.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v6.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.xxxx).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 13
TextureCube tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
Texture2D tex3 : register(t3);
SamplerState smp3 : register(s3);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : TEXCOORD3;
    float4 v6 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 v6 = input.v6;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(1, 0.300000012, 0.699999988, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.xyzw = (tex0.Sample(smp0, (v2.xyzw).xyz)).xyzw;
    r2.xyzw = (tex3.Sample(smp3, (v5.xyzw).xy)).xyzw;
    r2.z = saturate(dot((r2.xyzw).xyz, (d0.xxxx).xyz));
    r2.z = (-r2.zzzz + d0.xxxx).z;
    r3.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r3.xyzw = (r2.xxxx * r3.xyzw).xyzw;
    r1.xyzw = (r1.xyzw * r2.zzzz + r3.xyzw).xyzw;
    r0.xyzw = (r0.xyzw * r2.yyyy + r1.xyzw).xyzw;
    r1.x = (r2.wwww * d0.yyyy + d0.zzzz).x;
    r0.xyz = (r0.xyzw * r1.xxxx).xyz;
    r1.xyz = (r0.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.xyzw * v0.xyzw).xyz;
    r0.xyz = (r0.xyzw + r0.xyzw).xyz;
    r0.xyz = (r1.xyzw * r2.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v6.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.xxxx).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 14
Texture2D tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
Texture2D tex3 : register(t3);
SamplerState smp3 : register(s3);
Texture2D tex4 : register(t4);
SamplerState smp4 : register(s4);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : TEXCOORD3;
    float4 v6 : TEXCOORD4;
    float4 v7 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 v6 = input.v6;
    float4 v7 = input.v7;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(1, 0.300000012, 0.699999988, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.xyzw = (tex0.Sample(smp0, (v2.xyzw).xy)).xyzw;
    r2.xyzw = (tex4.Sample(smp4, (v6.xyzw).xy)).xyzw;
    r3.x = saturate(dot((r2.xyzw).xyz, (d0.xxxx).xyz));
    r3.x = (-r3.xxxx + d0.xxxx).x;
    r4.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r4.xyzw = (r2.xxxx * r4.xyzw).xyzw;
    r1.xyzw = (r1.xyzw * r3.xxxx + r4.xyzw).xyzw;
    r0.xyzw = (r0.xyzw * r2.yyyy + r1.xyzw).xyzw;
    r1.xyzw = (tex3.Sample(smp3, (v5.xyzw).xy)).xyzw;
    r0.xyzw = (r1.xyzw * r2.zzzz + r0.xyzw).xyzw;
    r1.x = (r2.wwww * d0.yyyy + d0.zzzz).x;
    r0.xyz = (r0.xyzw * r1.xxxx).xyz;
    r1.xyz = (r0.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.xyzw * v0.xyzw).xyz;
    r0.xyz = (r0.xyzw + r0.xyzw).xyz;
    r0.xyz = (r1.xyzw * r2.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v7.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.xxxx).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 15
TextureCube tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
Texture2D tex3 : register(t3);
SamplerState smp3 : register(s3);
Texture2D tex4 : register(t4);
SamplerState smp4 : register(s4);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : TEXCOORD3;
    float4 v6 : TEXCOORD4;
    float4 v7 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 v6 = input.v6;
    float4 v7 = input.v7;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(1, 0.300000012, 0.699999988, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.xyzw = (tex0.Sample(smp0, (v2.xyzw).xyz)).xyzw;
    r2.xyzw = (tex4.Sample(smp4, (v6.xyzw).xy)).xyzw;
    r3.x = saturate(dot((r2.xyzw).xyz, (d0.xxxx).xyz));
    r3.x = (-r3.xxxx + d0.xxxx).x;
    r4.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r4.xyzw = (r2.xxxx * r4.xyzw).xyzw;
    r1.xyzw = (r1.xyzw * r3.xxxx + r4.xyzw).xyzw;
    r0.xyzw = (r0.xyzw * r2.yyyy + r1.xyzw).xyzw;
    r1.xyzw = (tex3.Sample(smp3, (v5.xyzw).xy)).xyzw;
    r0.xyzw = (r1.xyzw * r2.zzzz + r0.xyzw).xyzw;
    r1.x = (r2.wwww * d0.yyyy + d0.zzzz).x;
    r0.xyz = (r0.xyzw * r1.xxxx).xyz;
    r1.xyz = (r0.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.xyzw * v0.xyzw).xyz;
    r0.xyz = (r0.xyzw + r0.xyzw).xyz;
    r0.xyz = (r1.xyzw * r2.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v7.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.xxxx).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 16
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
    const float4 d0 = float4(0.300000012, 0.699999988, 1, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.x = (abs(c[1].xxxx)).x;
    r1.xyzw = (tex0.Sample(smp0, (v2.xyzw).xy)).xyzw;
    r0.yzw = (r1.xxyz * v0.xxyz).yzw;
    r0.yzw = (r0.xyzw + r0.xyzw).yzw;
    r0.xyz = (Cmp(-r0.xxxx, r1.xyzw, r0.yzww)).xyz;
    r1.xyz = (r1.wwww * v1.xyzw).xyz;
    r2.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r0.w = (r2.wwww * d0.xxxx + d0.yyyy).w;
    r1.xyz = (r1.xyzw * r2.wwww).xyz;
    r0.xyz = (r0.xyzw * r0.wwww + r1.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v4.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.zzzz).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 17
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
    r0.x = (abs(c[1].xxxx)).x;
    r1.xyzw = (tex0.Sample(smp0, (v2.xyzw).xyz)).xyzw;
    r0.yzw = (r1.xxyz * v0.xxyz).yzw;
    r0.yzw = (r0.xyzw + r0.xyzw).yzw;
    r0.xyz = (Cmp(-r0.xxxx, r1.xyzw, r0.yzww)).xyz;
    r1.xyz = (r1.wwww * v1.xyzw).xyz;
    r2.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r0.w = (r2.wwww * d0.xxxx + d0.yyyy).w;
    r1.xyz = (r1.xyzw * r2.wwww).xyz;
    r0.xyz = (r0.xyzw * r0.wwww + r1.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v4.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.zzzz).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 18
Texture2D tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(0.300000012, 0.699999988, 1, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xy = (abs(c[1].xyzw)).xy;
    r1.xyz = (v0.xyzw + v0.xyzw).xyz;
    r2.xyzw = (tex0.Sample(smp0, (v2.xyzw).xy)).xyzw;
    r3.xyz = (r1.xyzw * r2.xyzw).xyz;
    r2.xyz = (Cmp(-r0.xxxx, r2.xyzw, r3.xyzw)).xyz;
    r3.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r0.xzw = (r1.xyyz * r3.xyyz).xzw;
    r3.xyz = (Cmp(-r0.yyyy, r3.xyzw, r0.xzww)).xyz;
    r0.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.xyzw = (lerp(r2.xyzw, r3.xyzw, r0.xxxx)).xyzw;
    r0.xyz = (r1.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.wwww * r0.xyzw).xyz;
    r0.w = (r0.wwww * d0.xxxx + d0.yyyy).w;
    r0.xyz = (r1.xyzw * r0.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v5.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.zzzz).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 19
TextureCube tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(0.300000012, 0.699999988, 1, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xy = (abs(c[1].xyzw)).xy;
    r1.xyz = (v0.xyzw + v0.xyzw).xyz;
    r2.xyzw = (tex0.Sample(smp0, (v2.xyzw).xyz)).xyzw;
    r3.xyz = (r1.xyzw * r2.xyzw).xyz;
    r2.xyz = (Cmp(-r0.xxxx, r2.xyzw, r3.xyzw)).xyz;
    r3.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r0.xzw = (r1.xyyz * r3.xyyz).xzw;
    r3.xyz = (Cmp(-r0.yyyy, r3.xyzw, r0.xzww)).xyz;
    r0.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.xyzw = (lerp(r2.xyzw, r3.xyzw, r0.xxxx)).xyzw;
    r0.xyz = (r1.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.wwww * r0.xyzw).xyz;
    r0.w = (r0.wwww * d0.xxxx + d0.yyyy).w;
    r0.xyz = (r1.xyzw * r0.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v5.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.zzzz).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 20
Texture2D tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
Texture2D tex3 : register(t3);
SamplerState smp3 : register(s3);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : TEXCOORD3;
    float4 v6 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 v6 = input.v6;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(0.300000012, 0.699999988, 1, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r5 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v2.xyzw).xy)).xyzw;
    r1.xyz = (v0.xyzw + v0.xyzw).xyz;
    r2.xyz = (r0.xyzw * r1.xyzw).xyz;
    r3.xyz = (abs(c[1].xyzw)).xyz;
    r0.xyz = (Cmp(-r3.xxxx, r0.xyzw, r2.xyzw)).xyz;
    r2.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r4.xyz = (r1.xyzw * r2.xyzw).xyz;
    r2.xyz = (Cmp(-r3.yyyy, r2.xyzw, r4.xyzw)).xyz;
    r4.xyzw = (tex3.Sample(smp3, (v5.xyzw).xy)).xyzw;
    r5.xyzw = (lerp(r0.xyzw, r2.xyzw, r4.xxxx)).xyzw;
    r0.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.xyz = (r1.xyzw * r0.xyzw).xyz;
    r0.xyz = (Cmp(-r3.zzzz, r0.xyzw, r1.xyzw)).xyz;
    r1.xyzw = (lerp(r5.xyzw, r0.xyzw, r4.yyyy)).xyzw;
    r0.xyz = (r1.wwww * v1.xyzw).xyz;
    r0.xyz = (r4.wwww * r0.xyzw).xyz;
    r0.w = (r4.wwww * d0.xxxx + d0.yyyy).w;
    r0.xyz = (r1.xyzw * r0.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v6.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.zzzz).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 21
TextureCube tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
Texture2D tex3 : register(t3);
SamplerState smp3 : register(s3);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : TEXCOORD3;
    float4 v6 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 v6 = input.v6;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(0.300000012, 0.699999988, 1, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r5 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v2.xyzw).xyz)).xyzw;
    r1.xyz = (v0.xyzw + v0.xyzw).xyz;
    r2.xyz = (r0.xyzw * r1.xyzw).xyz;
    r3.xyz = (abs(c[1].xyzw)).xyz;
    r0.xyz = (Cmp(-r3.xxxx, r0.xyzw, r2.xyzw)).xyz;
    r2.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r4.xyz = (r1.xyzw * r2.xyzw).xyz;
    r2.xyz = (Cmp(-r3.yyyy, r2.xyzw, r4.xyzw)).xyz;
    r4.xyzw = (tex3.Sample(smp3, (v5.xyzw).xy)).xyzw;
    r5.xyzw = (lerp(r0.xyzw, r2.xyzw, r4.xxxx)).xyzw;
    r0.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.xyz = (r1.xyzw * r0.xyzw).xyz;
    r0.xyz = (Cmp(-r3.zzzz, r0.xyzw, r1.xyzw)).xyz;
    r1.xyzw = (lerp(r5.xyzw, r0.xyzw, r4.yyyy)).xyzw;
    r0.xyz = (r1.wwww * v1.xyzw).xyz;
    r0.xyz = (r4.wwww * r0.xyzw).xyz;
    r0.w = (r4.wwww * d0.xxxx + d0.yyyy).w;
    r0.xyz = (r1.xyzw * r0.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v6.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.zzzz).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 22
Texture2D tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
Texture2D tex3 : register(t3);
SamplerState smp3 : register(s3);
Texture2D tex4 : register(t4);
SamplerState smp4 : register(s4);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : TEXCOORD3;
    float4 v6 : TEXCOORD4;
    float4 v7 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 v6 = input.v6;
    float4 v7 = input.v7;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(0.300000012, 0.699999988, 1, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r5 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v2.xyzw).xy)).xyzw;
    r1.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r2.xyz = (v0.xyzw + v0.xyzw).xyz;
    r3.xyz = (r0.xyzw * r2.xyzw).xyz;
    r4.xyzw = (abs(c[1].xyzw)).xyzw;
    r0.xyz = (Cmp(-r4.xxxx, r0.xyzw, r3.xyzw)).xyz;
    r3.xyz = (r1.xyzw * r2.xyzw).xyz;
    r1.xyz = (Cmp(-r4.yyyy, r1.xyzw, r3.xyzw)).xyz;
    r3.xyzw = (tex4.Sample(smp4, (v6.xyzw).xy)).xyzw;
    r5.xyzw = (lerp(r0.xyzw, r1.xyzw, r3.xxxx)).xyzw;
    r0.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.xyz = (r2.xyzw * r0.xyzw).xyz;
    r0.xyz = (Cmp(-r4.zzzz, r0.xyzw, r1.xyzw)).xyz;
    r1.xyzw = (lerp(r5.xyzw, r0.xyzw, r3.yyyy)).xyzw;
    r0.xyzw = (tex3.Sample(smp3, (v5.xyzw).xy)).xyzw;
    r2.xyz = (r2.xyzw * r0.xyzw).xyz;
    r0.xyz = (Cmp(-r4.wwww, r0.xyzw, r2.xyzw)).xyz;
    r2.xyzw = (lerp(r1.xyzw, r0.xyzw, r3.zzzz)).xyzw;
    r0.xyz = (r2.wwww * v1.xyzw).xyz;
    r0.xyz = (r3.wwww * r0.xyzw).xyz;
    r0.w = (r3.wwww * d0.xxxx + d0.yyyy).w;
    r0.xyz = (r2.xyzw * r0.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v7.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.zzzz).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 23
TextureCube tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
Texture2D tex3 : register(t3);
SamplerState smp3 : register(s3);
Texture2D tex4 : register(t4);
SamplerState smp4 : register(s4);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : TEXCOORD3;
    float4 v6 : TEXCOORD4;
    float4 v7 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 v6 = input.v6;
    float4 v7 = input.v7;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(0.300000012, 0.699999988, 1, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r5 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v2.xyzw).xyz)).xyzw;
    r1.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r2.xyz = (v0.xyzw + v0.xyzw).xyz;
    r3.xyz = (r0.xyzw * r2.xyzw).xyz;
    r4.xyzw = (abs(c[1].xyzw)).xyzw;
    r0.xyz = (Cmp(-r4.xxxx, r0.xyzw, r3.xyzw)).xyz;
    r3.xyz = (r1.xyzw * r2.xyzw).xyz;
    r1.xyz = (Cmp(-r4.yyyy, r1.xyzw, r3.xyzw)).xyz;
    r3.xyzw = (tex4.Sample(smp4, (v6.xyzw).xy)).xyzw;
    r5.xyzw = (lerp(r0.xyzw, r1.xyzw, r3.xxxx)).xyzw;
    r0.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.xyz = (r2.xyzw * r0.xyzw).xyz;
    r0.xyz = (Cmp(-r4.zzzz, r0.xyzw, r1.xyzw)).xyz;
    r1.xyzw = (lerp(r5.xyzw, r0.xyzw, r3.yyyy)).xyzw;
    r0.xyzw = (tex3.Sample(smp3, (v5.xyzw).xy)).xyzw;
    r2.xyz = (r2.xyzw * r0.xyzw).xyz;
    r0.xyz = (Cmp(-r4.wwww, r0.xyzw, r2.xyzw)).xyz;
    r2.xyzw = (lerp(r1.xyzw, r0.xyzw, r3.zzzz)).xyzw;
    r0.xyz = (r2.wwww * v1.xyzw).xyz;
    r0.xyz = (r3.wwww * r0.xyzw).xyz;
    r0.w = (r3.wwww * d0.xxxx + d0.yyyy).w;
    r0.xyz = (r2.xyzw * r0.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v7.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.zzzz).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 24
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
    const float4 d0 = float4(1, 0.300000012, 0.699999988, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.x = (abs(c[1].xxxx)).x;
    r1.xyzw = (tex0.Sample(smp0, (v2.xyzw).xy)).xyzw;
    r0.yzw = (r1.xxyz * v0.xxyz).yzw;
    r0.yzw = (r0.xyzw + r0.xyzw).yzw;
    r1.xyz = (Cmp(-r0.xxxx, r1.xyzw, r0.yzww)).xyz;
    r0.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r0.x = saturate(dot((r0.xyzw).xyz, (d0.xxxx).xyz));
    r0.x = (-r0.xxxx + d0.xxxx).x;
    r1.xyzw = (r1.xyzw * r0.xxxx).xyzw;
    r0.xyz = (r1.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.wwww * r0.xyzw).xyz;
    r0.w = (r0.wwww * d0.yyyy + d0.zzzz).w;
    r0.xyz = (r1.xyzw * r0.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v4.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.xxxx).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 25
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
    const float4 d0 = float4(1, 0.300000012, 0.699999988, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.x = (abs(c[1].xxxx)).x;
    r1.xyzw = (tex0.Sample(smp0, (v2.xyzw).xyz)).xyzw;
    r0.yzw = (r1.xxyz * v0.xxyz).yzw;
    r0.yzw = (r0.xyzw + r0.xyzw).yzw;
    r1.xyz = (Cmp(-r0.xxxx, r1.xyzw, r0.yzww)).xyz;
    r0.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r0.x = saturate(dot((r0.xyzw).xyz, (d0.xxxx).xyz));
    r0.x = (-r0.xxxx + d0.xxxx).x;
    r1.xyzw = (r1.xyzw * r0.xxxx).xyzw;
    r0.xyz = (r1.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.wwww * r0.xyzw).xyz;
    r0.w = (r0.wwww * d0.yyyy + d0.zzzz).w;
    r0.xyz = (r1.xyzw * r0.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v4.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.xxxx).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 26
Texture2D tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(1, 0.300000012, 0.699999988, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xy = (abs(c[1].xyzw)).xy;
    r1.xyz = (v0.xyzw + v0.xyzw).xyz;
    r2.xyzw = (tex0.Sample(smp0, (v2.xyzw).xy)).xyzw;
    r3.xyz = (r1.xyzw * r2.xyzw).xyz;
    r2.xyz = (Cmp(-r0.xxxx, r2.xyzw, r3.xyzw)).xyz;
    r3.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r0.xzw = (r1.xyyz * r3.xyyz).xzw;
    r3.xyz = (Cmp(-r0.yyyy, r3.xyzw, r0.xzww)).xyz;
    r0.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.xyzw = (r3.xyzw * r0.xxxx).xyzw;
    r0.x = saturate(dot((r0.xyzw).xyz, (d0.xxxx).xyz));
    r0.x = (-r0.xxxx + d0.xxxx).x;
    r1.xyzw = (r2.xyzw * r0.xxxx + r1.xyzw).xyzw;
    r0.xyz = (r1.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.wwww * r0.xyzw).xyz;
    r0.w = (r0.wwww * d0.yyyy + d0.zzzz).w;
    r0.xyz = (r1.xyzw * r0.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v5.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.xxxx).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 27
TextureCube tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(1, 0.300000012, 0.699999988, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xy = (abs(c[1].xyzw)).xy;
    r1.xyz = (v0.xyzw + v0.xyzw).xyz;
    r2.xyzw = (tex0.Sample(smp0, (v2.xyzw).xyz)).xyzw;
    r3.xyz = (r1.xyzw * r2.xyzw).xyz;
    r2.xyz = (Cmp(-r0.xxxx, r2.xyzw, r3.xyzw)).xyz;
    r3.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r0.xzw = (r1.xyyz * r3.xyyz).xzw;
    r3.xyz = (Cmp(-r0.yyyy, r3.xyzw, r0.xzww)).xyz;
    r0.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.xyzw = (r3.xyzw * r0.xxxx).xyzw;
    r0.x = saturate(dot((r0.xyzw).xyz, (d0.xxxx).xyz));
    r0.x = (-r0.xxxx + d0.xxxx).x;
    r1.xyzw = (r2.xyzw * r0.xxxx + r1.xyzw).xyzw;
    r0.xyz = (r1.wwww * v1.xyzw).xyz;
    r0.xyz = (r0.wwww * r0.xyzw).xyz;
    r0.w = (r0.wwww * d0.yyyy + d0.zzzz).w;
    r0.xyz = (r1.xyzw * r0.wwww + r0.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v5.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.xxxx).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 28
Texture2D tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
Texture2D tex3 : register(t3);
SamplerState smp3 : register(s3);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : TEXCOORD3;
    float4 v6 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 v6 = input.v6;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(1, 0.300000012, 0.699999988, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v2.xyzw).xy)).xyzw;
    r1.xyz = (v0.xyzw + v0.xyzw).xyz;
    r2.xyz = (r0.xyzw * r1.xyzw).xyz;
    r3.xyz = (abs(c[1].xyzw)).xyz;
    r0.xyz = (Cmp(-r3.xxxx, r0.xyzw, r2.xyzw)).xyz;
    r2.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r4.xyz = (r1.xyzw * r2.xyzw).xyz;
    r2.xyz = (Cmp(-r3.yyyy, r2.xyzw, r4.xyzw)).xyz;
    r4.xyzw = (tex3.Sample(smp3, (v5.xyzw).xy)).xyzw;
    r2.xyzw = (r2.xyzw * r4.xxxx).xyzw;
    r1.w = saturate(dot((r4.xyzw).xyz, (d0.xxxx).xyz));
    r1.w = (-r1.wwww + d0.xxxx).w;
    r0.xyzw = (r0.xyzw * r1.wwww + r2.xyzw).xyzw;
    r2.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.xyz = (r1.xyzw * r2.xyzw).xyz;
    r2.xyz = (Cmp(-r3.zzzz, r2.xyzw, r1.xyzw)).xyz;
    r0.xyzw = (r2.xyzw * r4.yyyy + r0.xyzw).xyzw;
    r1.xyz = (r0.wwww * v1.xyzw).xyz;
    r1.xyz = (r4.wwww * r1.xyzw).xyz;
    r0.w = (r4.wwww * d0.yyyy + d0.zzzz).w;
    r0.xyz = (r0.xyzw * r0.wwww + r1.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v6.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.xxxx).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 29
TextureCube tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
Texture2D tex3 : register(t3);
SamplerState smp3 : register(s3);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : TEXCOORD3;
    float4 v6 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 v6 = input.v6;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(1, 0.300000012, 0.699999988, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v2.xyzw).xyz)).xyzw;
    r1.xyz = (v0.xyzw + v0.xyzw).xyz;
    r2.xyz = (r0.xyzw * r1.xyzw).xyz;
    r3.xyz = (abs(c[1].xyzw)).xyz;
    r0.xyz = (Cmp(-r3.xxxx, r0.xyzw, r2.xyzw)).xyz;
    r2.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r4.xyz = (r1.xyzw * r2.xyzw).xyz;
    r2.xyz = (Cmp(-r3.yyyy, r2.xyzw, r4.xyzw)).xyz;
    r4.xyzw = (tex3.Sample(smp3, (v5.xyzw).xy)).xyzw;
    r2.xyzw = (r2.xyzw * r4.xxxx).xyzw;
    r1.w = saturate(dot((r4.xyzw).xyz, (d0.xxxx).xyz));
    r1.w = (-r1.wwww + d0.xxxx).w;
    r0.xyzw = (r0.xyzw * r1.wwww + r2.xyzw).xyzw;
    r2.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r1.xyz = (r1.xyzw * r2.xyzw).xyz;
    r2.xyz = (Cmp(-r3.zzzz, r2.xyzw, r1.xyzw)).xyz;
    r0.xyzw = (r2.xyzw * r4.yyyy + r0.xyzw).xyzw;
    r1.xyz = (r0.wwww * v1.xyzw).xyz;
    r1.xyz = (r4.wwww * r1.xyzw).xyz;
    r0.w = (r4.wwww * d0.yyyy + d0.zzzz).w;
    r0.xyz = (r0.xyzw * r0.wwww + r1.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v6.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.xxxx).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 30
Texture2D tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
Texture2D tex3 : register(t3);
SamplerState smp3 : register(s3);
Texture2D tex4 : register(t4);
SamplerState smp4 : register(s4);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : TEXCOORD3;
    float4 v6 : TEXCOORD4;
    float4 v7 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 v6 = input.v6;
    float4 v7 = input.v7;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(1, 0.300000012, 0.699999988, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r5 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v2.xyzw).xy)).xyzw;
    r1.xyz = (v0.xyzw + v0.xyzw).xyz;
    r2.xyz = (r0.xyzw * r1.xyzw).xyz;
    r3.xyzw = (abs(c[1].xyzw)).xyzw;
    r0.xyz = (Cmp(-r3.xxxx, r0.xyzw, r2.xyzw)).xyz;
    r2.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r4.xyz = (r1.xyzw * r2.xyzw).xyz;
    r2.xyz = (Cmp(-r3.yyyy, r2.xyzw, r4.xyzw)).xyz;
    r4.xyzw = (tex4.Sample(smp4, (v6.xyzw).xy)).xyzw;
    r2.xyzw = (r2.xyzw * r4.xxxx).xyzw;
    r1.w = saturate(dot((r4.xyzw).xyz, (d0.xxxx).xyz));
    r1.w = (-r1.wwww + d0.xxxx).w;
    r0.xyzw = (r0.xyzw * r1.wwww + r2.xyzw).xyzw;
    r2.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r5.xyz = (r1.xyzw * r2.xyzw).xyz;
    r2.xyz = (Cmp(-r3.zzzz, r2.xyzw, r5.xyzw)).xyz;
    r0.xyzw = (r2.xyzw * r4.yyyy + r0.xyzw).xyzw;
    r2.xyzw = (tex3.Sample(smp3, (v5.xyzw).xy)).xyzw;
    r1.xyz = (r1.xyzw * r2.xyzw).xyz;
    r2.xyz = (Cmp(-r3.wwww, r2.xyzw, r1.xyzw)).xyz;
    r0.xyzw = (r2.xyzw * r4.zzzz + r0.xyzw).xyzw;
    r1.xyz = (r0.wwww * v1.xyzw).xyz;
    r1.xyz = (r4.wwww * r1.xyzw).xyz;
    r0.w = (r4.wwww * d0.yyyy + d0.zzzz).w;
    r0.xyz = (r0.xyzw * r0.wwww + r1.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v7.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.xxxx).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif

#if PERMUTATION == 31
TextureCube tex0 : register(t0);
SamplerState smp0 : register(s0);
Texture2D tex1 : register(t1);
SamplerState smp1 : register(s1);
Texture2D tex2 : register(t2);
SamplerState smp2 : register(s2);
Texture2D tex3 : register(t3);
SamplerState smp3 : register(s3);
Texture2D tex4 : register(t4);
SamplerState smp4 : register(s4);
struct PsInput {
    float4 position : SV_Position;
    float4 v0 : COLOR0;
    float4 v1 : COLOR1;
    float4 v2 : TEXCOORD0;
    float4 v3 : TEXCOORD1;
    float4 v4 : TEXCOORD2;
    float4 v5 : TEXCOORD3;
    float4 v6 : TEXCOORD4;
    float4 v7 : FOGFACTOR0;
};
float4 main(PsInput input) : SV_Target {
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 v4 = input.v4;
    float4 v5 = input.v5;
    float4 v6 = input.v6;
    float4 v7 = input.v7;
    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(1, 0.300000012, 0.699999988, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r5 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (tex0.Sample(smp0, (v2.xyzw).xyz)).xyzw;
    r1.xyz = (v0.xyzw + v0.xyzw).xyz;
    r2.xyz = (r0.xyzw * r1.xyzw).xyz;
    r3.xyzw = (abs(c[1].xyzw)).xyzw;
    r0.xyz = (Cmp(-r3.xxxx, r0.xyzw, r2.xyzw)).xyz;
    r2.xyzw = (tex1.Sample(smp1, (v3.xyzw).xy)).xyzw;
    r4.xyz = (r1.xyzw * r2.xyzw).xyz;
    r2.xyz = (Cmp(-r3.yyyy, r2.xyzw, r4.xyzw)).xyz;
    r4.xyzw = (tex4.Sample(smp4, (v6.xyzw).xy)).xyzw;
    r2.xyzw = (r2.xyzw * r4.xxxx).xyzw;
    r1.w = saturate(dot((r4.xyzw).xyz, (d0.xxxx).xyz));
    r1.w = (-r1.wwww + d0.xxxx).w;
    r0.xyzw = (r0.xyzw * r1.wwww + r2.xyzw).xyzw;
    r2.xyzw = (tex2.Sample(smp2, (v4.xyzw).xy)).xyzw;
    r5.xyz = (r1.xyzw * r2.xyzw).xyz;
    r2.xyz = (Cmp(-r3.zzzz, r2.xyzw, r5.xyzw)).xyz;
    r0.xyzw = (r2.xyzw * r4.yyyy + r0.xyzw).xyzw;
    r2.xyzw = (tex3.Sample(smp3, (v5.xyzw).xy)).xyzw;
    r1.xyz = (r1.xyzw * r2.xyzw).xyz;
    r2.xyz = (Cmp(-r3.wwww, r2.xyzw, r1.xyzw)).xyz;
    r0.xyzw = (r2.xyzw * r4.zzzz + r0.xyzw).xyzw;
    r1.xyz = (r0.wwww * v1.xyzw).xyz;
    r1.xyz = (r4.wwww * r1.xyzw).xyz;
    r0.w = (r4.wwww * d0.yyyy + d0.zzzz).w;
    r0.xyz = (r0.xyzw * r0.wwww + r1.xyzw).xyz;
    r0.xyz = (r0.xyzw + -c[2].xyzw).xyz;
    oC0.xyz = (v7.xxxx * r0.xyzw + c[2].xyzw).xyz;
    oC0.w = (d0.xxxx).w;
    AlphaTest(oC0.w);
    return oC0;
}
#endif
