#include "../Common.hlsli"

Texture2D tex0 : register(t0);
SamplerState sampler0 : register(s0);

float4 main(float4 position : SV_Position, float4 color : COLOR0, float2 uv : TEXCOORD0) : SV_Target {
    float4 texel = tex0.Sample(sampler0, uv);
    float gray = dot(texel.rgb, float3(0.299f, 0.587f, 0.114f));
    float4 result = float4(gray, gray, gray, texel.a * color.a);
    AlphaTest(result.a);
    return result;
}
