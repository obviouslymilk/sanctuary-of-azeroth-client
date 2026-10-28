// Rewritten from Shaders\Vertex\vs_3_0\vsLiquidMagma.bls (1 permutations, 1 distinct) by Tools/Shaders/bls_to_hlsl.py.
#include "../Common.hlsli"

#if PERMUTATION == 0
struct VsInput {
    float4 v0 : POSITION0;
    float4 v1 : COLOR0;
    float4 v2 : TEXCOORD0;
};
struct VsOutput {
    float4 o0 : SV_Position;
    float4 o1 : COLOR0;
    float4 o2 : TEXCOORD0;
    float4 o3 : FOGFACTOR0;
};
VsOutput main(VsInput input) {
    VsOutput output = (VsOutput)0;
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 o0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 o1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 o2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 o3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 d11 = float4(0, 1, 0, 0);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (c[6].xyzw * v0.yyyy).xyzw;
    r0.xyzw = (v0.xxxx * c[5].xyzw + r0.xyzw).xyzw;
    r0.xyzw = (v0.zzzz * c[7].xyzw + r0.xyzw).xyzw;
    r0.xyzw = (r0.xyzw + c[8].xyzw).xyzw;
    r1.xyzw = (r0.yyyy * c[1].xyzw).xyzw;
    r1.xyzw = (r0.xxxx * c[0].xyzw + r1.xyzw).xyzw;
    r1.xyzw = (r0.zzzz * c[2].xyzw + r1.xyzw).xyzw;
    o0.xyzw = (r0.wwww * c[3].xyzw + r1.xyzw).xyzw;
    r0.x = (r0.zzzz * c[4].xxxx + c[4].yyyy).x;
    r0.x = (max(r0.xxxx, d11.xxxx)).x;
    r1.x = pow(abs(r0.x), c[4].z);
    o3.x = (min(r1.xxxx, d11.yyyy)).x;
    r0.xy = (c[10].xyzw * v2.yyyy).xy;
    r0.xy = (v2.xxxx * c[9].xyzw + r0.xyzw).xy;
    o2.xy = (r0.xyzw + c[12].xyzw).xy;
    o1.xyzw = (v1.xyzw).xyzw;
    output.o0 = o0;
    output.o1 = o1;
    output.o2 = o2;
    output.o3 = o3;
    return output;
}
#endif
