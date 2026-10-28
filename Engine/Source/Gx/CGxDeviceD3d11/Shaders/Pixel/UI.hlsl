#include "../Common.hlsli"

Texture2D tex0 : register(t0);
SamplerState sampler0 : register(s0);

float4 main(float4 position : SV_Position, float4 color : COLOR0, float2 uv : TEXCOORD0) : SV_Target {
    float4 result = tex0.Sample(sampler0, uv) * color;
    AlphaTest(result.a);
    return result;
}
