float4 g_texel : register(c0); // 1/width, 1/height of the target

struct VsOut {
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
};

VsOut FullscreenVS(float2 pos : POSITION) {
    VsOut o;
    o.pos = float4(pos.x - g_texel.x, pos.y + g_texel.y, 0.0, 1.0);
    o.uv = float2(pos.x * 0.5 + 0.5, 0.5 - pos.y * 0.5);
    return o;
}

sampler2D g_src : register(s0);

float4 CopyPS(float2 uv : TEXCOORD0) : COLOR0 {
    return tex2D(g_src, uv);
}
