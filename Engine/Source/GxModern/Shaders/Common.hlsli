// Shared by the depth based passes. Matrices are uploaded row by row (row vector convention).
row_major float4x4 g_invProj : register(c0);
float4 g_depthFit : register(c4);   // x,y: 1/w = x*z + y; z,w: clip.z = z*w + w
float4 g_screen : register(c5);     // 1/width, 1/height, width, height of the pass target

static const float SKY_DEPTH = 100000.0;

float2 UvToNdc(float2 uv) {
    return float2(uv.x * 2.0 - 1.0, 1.0 - uv.y * 2.0);
}

float RawToLinear(float z) {
    return z >= 0.999999 ? SKY_DEPTH : 1.0 / (g_depthFit.x * z + g_depthFit.y);
}

float3 ViewPosFromRaw(float2 uv, float z) {
    float4 p = mul(float4(UvToNdc(uv), z, 1.0), g_invProj);
    return p.xyz / p.w;
}

float3 ViewPosFromLinear(float2 uv, float w) {
    float4 base = mul(float4(UvToNdc(uv), g_depthFit.z, 1.0), g_invProj);
    return (base * w + g_invProj[2] * g_depthFit.w).xyz;
}

float Bayer4(float2 pixel) {
    float2 p = floor(fmod(pixel, 4.0));
    float2 a = fmod(p, 2.0);
    float2 b = floor(p * 0.5);
    float ba = fmod(2.0 * a.x + 3.0 * a.y, 4.0);
    float bb = fmod(2.0 * b.x + 3.0 * b.y, 4.0);
    return (ba * 4.0 + bb + 0.5) / 16.0;
}

float InterleavedGradient(float2 pixel, float frame) {
    pixel += frame * 5.588238;
    return frac(52.9829189 * frac(dot(pixel, float2(0.06711056, 0.00583715))));
}
