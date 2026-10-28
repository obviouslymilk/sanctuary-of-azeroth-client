// b0 is the DX9 constant file of the stage, c[N] is register cN.
cbuffer StageConstants : register(b0) {
    float4 c[256];
};

// Mirrors CGxDeviceD3d11::DeviceConstants.
cbuffer DeviceConstants : register(b1) {
    float4 alphaRef;
    float4 fog;
    float4 fogColor;
};

void AlphaTest(float alpha) {
    if (alphaRef.y != 0.0f) {
        clip(alpha * 255.0f - alphaRef.x);
    }
}

float3 ApplyFog(float3 color, float distance) {
    float factor = saturate((fog.y - distance) * fog.w);
    return fog.z != 0.0f ? lerp(fogColor.rgb, color, factor) : color;
}

// DX9 cmp: per component, a >= 0 ? b : c
float4 Cmp(float4 a, float4 b, float4 c) {
    return float4(a.x >= 0.0f ? b.x : c.x, a.y >= 0.0f ? b.y : c.y, a.z >= 0.0f ? b.z : c.z, a.w >= 0.0f ? b.w : c.w);
}

// DX9 hardware PCF: the sampler returns the filtered result of comparing coord.z with the stored depth.
float4 ShadowCompare(Texture2D depthMap, SamplerComparisonState smp, float3 coord) {
    float lit = depthMap.SampleCmpLevelZero(smp, coord.xy, coord.z);
    return float4(lit, lit, lit, 1.0f);
}
