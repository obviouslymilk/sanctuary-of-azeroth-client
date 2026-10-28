#include "../Common.hlsli"

struct VsInput {
    float4 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct VsOutput {
    float4 position : SV_Position;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

VsOutput main(VsInput input) {
    VsOutput output;

    float4 p = float4(dot(c[0], input.position), dot(c[1], input.position), dot(c[2], input.position), dot(c[3], input.position));

#if PERMUTATION == 1
    // stereo variant, xy pre-multiplied by w
    output.position = float4(p.xy * p.w, p.z, p.w);
#else
    output.position = p;
#endif

    output.color = input.color;
    output.uv = input.uv;
    return output;
}
