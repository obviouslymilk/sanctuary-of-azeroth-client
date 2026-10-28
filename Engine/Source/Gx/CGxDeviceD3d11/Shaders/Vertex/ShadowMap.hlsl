// Rewritten from Shaders\Vertex\vs_3_0\ShadowMap.bls (90 permutations, 3 distinct) by Tools/Shaders/bls_to_hlsl.py.
#include "../Common.hlsli"

#if PERMUTATION == 0 || PERMUTATION == 1 || PERMUTATION == 2 || PERMUTATION == 3 || PERMUTATION == 4 || PERMUTATION == 5 || PERMUTATION == 6 || PERMUTATION == 7 || PERMUTATION == 8 || PERMUTATION == 9 || PERMUTATION == 30 || PERMUTATION == 31 || PERMUTATION == 32 || PERMUTATION == 33 || PERMUTATION == 34 || PERMUTATION == 35 || PERMUTATION == 36 || PERMUTATION == 37 || PERMUTATION == 38 || PERMUTATION == 39 || PERMUTATION == 60 || PERMUTATION == 61 || PERMUTATION == 62 || PERMUTATION == 63 || PERMUTATION == 64 || PERMUTATION == 65 || PERMUTATION == 66 || PERMUTATION == 67 || PERMUTATION == 68 || PERMUTATION == 69
struct VsInput {
    float4 v0 : POSITION0;
    float4 v1 : TEXCOORD0;
};
struct VsOutput {
    float4 o0 : SV_Position;
    float4 o1 : TEXCOORD0;
    float4 o2 : TEXCOORD1;
};
VsOutput main(VsInput input) {
    VsOutput output = (VsOutput)0;
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 o0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 o1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 o2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(1, 0, 0, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.w = (d0.xxxx).w;
    r0.x = dot(c[31].xyzw, v0.xyzw);
    r0.y = dot(c[32].xyzw, v0.xyzw);
    r0.z = dot(c[33].xyzw, v0.xyzw);
    o0.x = dot(c[2].xyzw, r0.xyzw);
    o0.y = dot(c[3].xyzw, r0.xyzw);
    o0.z = dot(c[4].xyzw, r0.xyzw);
    o0.w = dot(c[5].xyzw, r0.xyzw);
    o2.xyz = (r0.xyzw).xyz;
    o1.x = dot(c[6].xyzw, v1.xyzw);
    o1.y = dot(c[7].xyzw, v1.xyzw);
    output.o0 = o0;
    output.o1 = o1;
    output.o2 = o2;
    return output;
}
#endif

#if PERMUTATION == 10 || PERMUTATION == 11 || PERMUTATION == 12 || PERMUTATION == 13 || PERMUTATION == 14 || PERMUTATION == 15 || PERMUTATION == 16 || PERMUTATION == 17 || PERMUTATION == 18 || PERMUTATION == 19 || PERMUTATION == 40 || PERMUTATION == 41 || PERMUTATION == 42 || PERMUTATION == 43 || PERMUTATION == 44 || PERMUTATION == 45 || PERMUTATION == 46 || PERMUTATION == 47 || PERMUTATION == 48 || PERMUTATION == 49 || PERMUTATION == 70 || PERMUTATION == 71 || PERMUTATION == 72 || PERMUTATION == 73 || PERMUTATION == 74 || PERMUTATION == 75 || PERMUTATION == 76 || PERMUTATION == 77 || PERMUTATION == 78 || PERMUTATION == 79
struct VsInput {
    float4 v0 : POSITION0;
    float4 v1 : BLENDINDICES0;
    float4 v2 : TEXCOORD0;
};
struct VsOutput {
    float4 o0 : SV_Position;
    float4 o1 : TEXCOORD0;
    float4 o2 : TEXCOORD1;
};
VsOutput main(VsInput input) {
    VsOutput output = (VsOutput)0;
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 o0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 o1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 o2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(3, 1, 0, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    int4 a0 = int4(0, 0, 0, 0);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.x = (d0.xxxx * v1.xxxx).x;
    a0.x = (int)floor((r0.xxxx).x + 0.5f);
    r0.x = dot(c[31 + a0.x].xyzw, v0.xyzw);
    r0.y = dot(c[32 + a0.x].xyzw, v0.xyzw);
    r0.z = dot(c[33 + a0.x].xyzw, v0.xyzw);
    r0.w = (d0.yyyy).w;
    r1.x = dot(c[14].xyzw, r0.xyzw);
    r1.y = dot(c[15].xyzw, r0.xyzw);
    r1.z = dot(c[16].xyzw, r0.xyzw);
    r1.w = (d0.yyyy).w;
    o0.x = dot(c[2].xyzw, r1.xyzw);
    o0.y = dot(c[3].xyzw, r1.xyzw);
    o0.z = dot(c[4].xyzw, r1.xyzw);
    o0.w = dot(c[5].xyzw, r1.xyzw);
    o2.xyz = (r1.xyzw).xyz;
    o1.x = dot(c[6].xyzw, v2.xyzw);
    o1.y = dot(c[7].xyzw, v2.xyzw);
    output.o0 = o0;
    output.o1 = o1;
    output.o2 = o2;
    return output;
}
#endif

#if PERMUTATION == 20 || PERMUTATION == 21 || PERMUTATION == 22 || PERMUTATION == 23 || PERMUTATION == 24 || PERMUTATION == 25 || PERMUTATION == 26 || PERMUTATION == 27 || PERMUTATION == 28 || PERMUTATION == 29 || PERMUTATION == 50 || PERMUTATION == 51 || PERMUTATION == 52 || PERMUTATION == 53 || PERMUTATION == 54 || PERMUTATION == 55 || PERMUTATION == 56 || PERMUTATION == 57 || PERMUTATION == 58 || PERMUTATION == 59 || PERMUTATION == 80 || PERMUTATION == 81 || PERMUTATION == 82 || PERMUTATION == 83 || PERMUTATION == 84 || PERMUTATION == 85 || PERMUTATION == 86 || PERMUTATION == 87 || PERMUTATION == 88 || PERMUTATION == 89
struct VsInput {
    float4 v0 : POSITION0;
    float4 v1 : BLENDWEIGHT0;
    float4 v2 : BLENDINDICES0;
    float4 v3 : TEXCOORD0;
};
struct VsOutput {
    float4 o0 : SV_Position;
    float4 o1 : TEXCOORD0;
    float4 o2 : TEXCOORD1;
};
VsOutput main(VsInput input) {
    VsOutput output = (VsOutput)0;
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 v3 = input.v3;
    float4 o0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 o1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 o2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d0 = float4(3, 1, 0, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    int4 a0 = int4(0, 0, 0, 0);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (d0.xxxx * v2.xyzw).xyzw;
    a0.x = (int)floor((r0.xyzw).x + 0.5f);
    a0.y = (int)floor((r0.xyzw).y + 0.5f);
    a0.z = (int)floor((r0.xyzw).z + 0.5f);
    a0.w = (int)floor((r0.xyzw).w + 0.5f);
    r0.xyzw = (v1.yyyy * c[31 + a0.y].xyzw).xyzw;
    r0.xyzw = (c[31 + a0.x].xyzw * v1.xxxx + r0.xyzw).xyzw;
    r0.xyzw = (c[31 + a0.z].xyzw * v1.zzzz + r0.xyzw).xyzw;
    r0.xyzw = (c[31 + a0.w].xyzw * v1.wwww + r0.xyzw).xyzw;
    r0.x = dot(r0.xyzw, v0.xyzw);
    r1.xyzw = (v1.yyyy * c[32 + a0.y].xyzw).xyzw;
    r1.xyzw = (c[32 + a0.x].xyzw * v1.xxxx + r1.xyzw).xyzw;
    r1.xyzw = (c[32 + a0.z].xyzw * v1.zzzz + r1.xyzw).xyzw;
    r1.xyzw = (c[32 + a0.w].xyzw * v1.wwww + r1.xyzw).xyzw;
    r0.y = dot(r1.xyzw, v0.xyzw);
    r1.xyzw = (v1.yyyy * c[33 + a0.y].xyzw).xyzw;
    r1.xyzw = (c[33 + a0.x].xyzw * v1.xxxx + r1.xyzw).xyzw;
    r1.xyzw = (c[33 + a0.z].xyzw * v1.zzzz + r1.xyzw).xyzw;
    r1.xyzw = (c[33 + a0.w].xyzw * v1.wwww + r1.xyzw).xyzw;
    r0.z = dot(r1.xyzw, v0.xyzw);
    r0.w = (d0.yyyy).w;
    r1.x = dot(c[14].xyzw, r0.xyzw);
    r1.y = dot(c[15].xyzw, r0.xyzw);
    r1.z = dot(c[16].xyzw, r0.xyzw);
    r1.w = (d0.yyyy).w;
    o0.x = dot(c[2].xyzw, r1.xyzw);
    o0.y = dot(c[3].xyzw, r1.xyzw);
    o0.z = dot(c[4].xyzw, r1.xyzw);
    o0.w = dot(c[5].xyzw, r1.xyzw);
    o2.xyz = (r1.xyzw).xyz;
    o1.x = dot(c[6].xyzw, v3.xyzw);
    o1.y = dot(c[7].xyzw, v3.xyzw);
    output.o0 = o0;
    output.o1 = o1;
    output.o2 = o2;
    return output;
}
#endif
