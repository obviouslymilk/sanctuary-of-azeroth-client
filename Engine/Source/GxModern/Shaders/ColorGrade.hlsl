sampler2D g_src : register(s0);
sampler2D g_lut : register(s1);

float4 g_grade : register(c0);   // x: exposure multiplier, y: contrast, z: saturation, w: vignette
float4 g_balance : register(c1); // rgb: white balance gain, w: LUT amount

float3 ApplyLut(float3 c) {
    c = saturate(c);
    float b = c.b * 15.0;
    float b0 = floor(b);
    float b1 = min(b0 + 1.0, 15.0);
    float f = b - b0;
    float2 uv0 = float2((b0 * 16.0 + c.r * 15.0 + 0.5) / 256.0, (c.g * 15.0 + 0.5) / 16.0);
    float2 uv1 = float2((b1 * 16.0 + c.r * 15.0 + 0.5) / 256.0, uv0.y);
    return lerp(tex2D(g_lut, uv0).rgb, tex2D(g_lut, uv1).rgb, f);
}

float4 ColorGradePS(float2 uv : TEXCOORD0) : COLOR0 {
    float3 c = tex2D(g_src, uv).rgb;

    c *= g_grade.x * g_balance.rgb;
    c = (c - 0.5) * g_grade.y + 0.5;

    float luma = dot(c, float3(0.299, 0.587, 0.114));
    c = lerp(luma.xxx, c, g_grade.z);

    if (g_balance.w > 0.0) {
        c = lerp(c, ApplyLut(c), g_balance.w);
    }

    float2 d = uv - 0.5;
    c *= 1.0 - saturate(dot(d, d) * 4.0) * saturate(g_grade.w);

    return float4(saturate(c), 1.0);
}
