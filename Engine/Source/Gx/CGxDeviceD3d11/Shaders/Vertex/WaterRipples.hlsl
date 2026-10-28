// Rewritten from Shaders\Vertex\vs_3_0\WaterRipples.bls (1 permutations, 1 distinct) by Tools/Shaders/bls_to_hlsl.py.
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
};
VsOutput main(VsInput input) {
    VsOutput output = (VsOutput)0;
    float4 v0 = input.v0;
    float4 v1 = input.v1;
    float4 v2 = input.v2;
    float4 o0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 o1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 o2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 r0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    r0.xyzw = (c[1].xyzw * v0.yyyy).xyzw;
    r0.xyzw = (v0.xxxx * c[0].xyzw + r0.xyzw).xyzw;
    r0.xyzw = (v0.zzzz * c[2].xyzw + r0.xyzw).xyzw;
    o0.xyzw = (r0.xyzw + c[3].xyzw).xyzw;
    o1.xyzw = (v1.xyzw).xyzw;
    o2.xy = (v2.xyzw).xy;
    output.o0 = o0;
    output.o1 = o1;
    output.o2 = o2;
    return output;
}
#endif
