#include "Engine/Source/GxModern/GxModernInternal.h"

namespace GxModern {

static Target s_ao;
static Target s_aoBlurred;

void SsaoRun() {
    const Settings& settings = GetSettings();
    IDirect3DDevice9* device = g_frame.device;

    if (!settings.ssao) {
        return;
    }

    bool half = settings.ssao == 1;
    int aoWidth = half ? (g_frame.width + 1) / 2 : g_frame.width;
    int aoHeight = half ? (g_frame.height + 1) / 2 : g_frame.height;

    if (!s_ao.Ensure(device, aoWidth, aoHeight, D3DFMT_A8R8G8B8) || !s_aoBlurred.Ensure(device, aoWidth, aoHeight, D3DFMT_A8R8G8B8)) {
        return;
    }

    TimerBegin(device, Section_Ssao);

    IDirect3DTexture9* linear = EnsureLinearDepth(half);

    if (!linear) {
        TimerEnd(device, Section_Ssao);
        return;
    }

    float scale = static_cast<float>(aoWidth) / static_cast<float>(g_frame.width);

    SetTarget(device, s_ao, false, 0);
    SetCommonConstants(device, aoWidth, aoHeight);
    SetSampler(device, 0, g_sceneDepthTex, D3DTEXF_POINT, D3DTADDRESS_CLAMP);
    SetSampler(device, 1, linear, D3DTEXF_POINT, D3DTADDRESS_CLAMP);

    float gtao[4] = { settings.ssaoRadius, g_frame.pixelsPerYard * scale, 64.0f * scale, 0.0f };
    SetPixelConstants(device, 8, gtao, 1);
    device->SetPixelShader(PixelShader(half ? Ps_Gtao : Ps_GtaoFull));
    DrawFullscreen(device);

    SetTarget(device, s_aoBlurred, false, 0);
    SetSampler(device, 0, nullptr, D3DTEXF_POINT, D3DTADDRESS_CLAMP);
    SetSampler(device, 2, s_ao.tex, D3DTEXF_POINT, D3DTADDRESS_CLAMP);
    device->SetPixelShader(PixelShader(Ps_AoBlur));
    DrawFullscreen(device);

    device->SetRenderTarget(0, g_sceneColor.surf);
    BeginFullscreen(device, g_frame.width, g_frame.height);
    SetCommonConstants(device, g_frame.width, g_frame.height);
    SetSampler(device, 0, g_sceneDepthTex, D3DTEXF_POINT, D3DTADDRESS_CLAMP);
    SetSampler(device, 2, s_aoBlurred.tex, D3DTEXF_POINT, D3DTADDRESS_CLAMP);

    float apply[4] = { settings.ssaoStrength, 0.0f, static_cast<float>(aoWidth), static_cast<float>(aoHeight) };
    SetPixelConstants(device, 8, apply, 1);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
    device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
    device->SetPixelShader(PixelShader(Ps_AoApply));
    DrawFullscreen(device);

    for (uint32_t stage = 0; stage < 3; stage++) {
        device->SetTexture(stage, nullptr);
    }

    TimerEnd(device, Section_Ssao);
}

void SsaoRelease() {
    s_ao.Release();
    s_aoBlurred.Release();
}

}
