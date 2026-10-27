#include "Engine/Source/GxModern/GxModernInternal.h"
#include "Textures/AreaTex.h"
#include "Textures/SearchTex.h"

namespace GxModern {

static Target s_edges;
static Target s_weights;
static IDirect3DTexture9* s_areaTex;
static IDirect3DTexture9* s_searchTex;

static bool EnsureTextures(IDirect3DDevice9* device) {
    if (!s_areaTex) {
        s_areaTex = CreateTextureFromMemory(device, AREATEX_WIDTH, AREATEX_HEIGHT, D3DFMT_A8L8, 2, areaTexBytes, AREATEX_PITCH);
    }

    if (!s_searchTex) {
        s_searchTex = CreateTextureFromMemory(device, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, D3DFMT_L8, 1, searchTexBytes, SEARCHTEX_PITCH);
    }

    return s_areaTex && s_searchTex;
}

static void SetMetrics(IDirect3DDevice9* device, int width, int height) {
    float metrics[4] = { 1.0f / width, 1.0f / height, static_cast<float>(width), static_cast<float>(height) };
    SetVertexConstants(device, 0, metrics, 1);
    SetPixelConstants(device, 0, metrics, 1);
}

void SmaaRun(IDirect3DTexture9* source, IDirect3DSurface9* destination, int width, int height) {
    IDirect3DDevice9* device = g_frame.device;

    if (!EnsureTextures(device) || !s_edges.Ensure(device, width, height, D3DFMT_A8R8G8B8) || !s_weights.Ensure(device, width, height, D3DFMT_A8R8G8B8)) {
        return;
    }

    TimerBegin(device, Section_Smaa);

    SetTarget(device, s_edges, true, 0);
    SetMetrics(device, width, height);
    SetSampler(device, 0, source, D3DTEXF_POINT, D3DTADDRESS_CLAMP);
    device->SetVertexShader(VertexShader(Vs_SmaaEdge));
    device->SetPixelShader(PixelShader(Ps_SmaaEdge));
    DrawFullscreen(device);

    SetTarget(device, s_weights, true, 0);
    SetMetrics(device, width, height);
    SetSampler(device, 0, nullptr, D3DTEXF_POINT, D3DTADDRESS_CLAMP);
    SetSampler(device, 1, s_edges.tex, D3DTEXF_LINEAR, D3DTADDRESS_CLAMP);
    SetSampler(device, 2, s_areaTex, D3DTEXF_LINEAR, D3DTADDRESS_CLAMP);
    SetSampler(device, 3, s_searchTex, D3DTEXF_POINT, D3DTADDRESS_CLAMP);
    device->SetVertexShader(VertexShader(Vs_SmaaWeights));
    device->SetPixelShader(PixelShader(Ps_SmaaWeights));
    DrawFullscreen(device);

    device->SetRenderTarget(0, destination);
    BeginFullscreen(device, width, height);
    SetMetrics(device, width, height);

    RECT rect = g_frame.worldRect;
    device->SetScissorRect(&rect);
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

    SetSampler(device, 0, source, D3DTEXF_LINEAR, D3DTADDRESS_CLAMP);
    SetSampler(device, 1, nullptr, D3DTEXF_POINT, D3DTADDRESS_CLAMP);
    SetSampler(device, 2, nullptr, D3DTEXF_POINT, D3DTADDRESS_CLAMP);
    SetSampler(device, 3, nullptr, D3DTEXF_POINT, D3DTADDRESS_CLAMP);
    SetSampler(device, 5, s_weights.tex, D3DTEXF_LINEAR, D3DTADDRESS_CLAMP);
    device->SetVertexShader(VertexShader(Vs_SmaaBlend));
    device->SetPixelShader(PixelShader(Ps_SmaaBlend));
    DrawFullscreen(device);

    for (uint32_t stage = 0; stage < 6; stage++) {
        device->SetTexture(stage, nullptr);
    }

    TimerEnd(device, Section_Smaa);
}

void SmaaRelease() {
    s_edges.Release();
    s_weights.Release();

    if (s_areaTex) {
        s_areaTex->Release();
        s_areaTex = nullptr;
    }

    if (s_searchTex) {
        s_searchTex->Release();
        s_searchTex = nullptr;
    }
}

}
