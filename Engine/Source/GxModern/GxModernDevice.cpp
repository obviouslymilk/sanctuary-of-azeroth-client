#include <cstring>
#include "Engine/Source/GxModern/GxModernInternal.h"

namespace GxModern {

static IDirect3DVertexDeclaration9* s_fullscreenDecl;
static Target s_linearHalf;
static Target s_linearFull;
static uint32_t s_linearFrame[2];

bool Target::Ensure(IDirect3DDevice9* device, int width, int height, D3DFORMAT fmt) {
    if (this->tex && this->w == width && this->h == height && this->format == fmt) {
        return true;
    }

    this->Release();

    if (FAILED(device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, fmt, D3DPOOL_DEFAULT, &this->tex, nullptr))) {
        this->tex = nullptr;
        return false;
    }

    this->tex->GetSurfaceLevel(0, &this->surf);
    this->w = width;
    this->h = height;
    this->format = fmt;
    return true;
}

void Target::Release() {
    if (this->surf) {
        this->surf->Release();
        this->surf = nullptr;
    }

    if (this->tex) {
        this->tex->Release();
        this->tex = nullptr;
    }

    this->w = 0;
    this->h = 0;
}

void BeginFullscreen(IDirect3DDevice9* device, int width, int height) {
    if (!s_fullscreenDecl) {
        static const D3DVERTEXELEMENT9 elements[] = {
            { 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
            D3DDECL_END()
        };

        device->CreateVertexDeclaration(elements, &s_fullscreenDecl);
    }

    device->SetVertexDeclaration(s_fullscreenDecl);
    device->SetVertexShader(VertexShader(Vs_Fullscreen));
    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    device->SetRenderState(D3DRS_FOGENABLE, FALSE);
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF);
    device->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);
    device->SetRenderState(D3DRS_CLIPPING, TRUE);
    device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

    float texel[4] = { 1.0f / width, 1.0f / height, 0.0f, 0.0f };
    SetVertexConstants(device, 0, texel, 1);

    D3DVIEWPORT9 viewport = { 0, 0, static_cast<DWORD>(width), static_cast<DWORD>(height), 0.0f, 1.0f };
    device->SetViewport(&viewport);
}

void DrawFullscreen(IDirect3DDevice9* device) {
    static const float vertices[6] = { -1.0f, -1.0f, -1.0f, 3.0f, 3.0f, -1.0f };
    device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, vertices, sizeof(float) * 2);
}

void SetPixelConstants(IDirect3DDevice9* device, uint32_t reg, const float* data, uint32_t count) {
    device->SetPixelShaderConstantF(reg, data, count);
}

void SetVertexConstants(IDirect3DDevice9* device, uint32_t reg, const float* data, uint32_t count) {
    device->SetVertexShaderConstantF(reg, data, count);
}

void SetSampler(IDirect3DDevice9* device, uint32_t stage, IDirect3DBaseTexture9* tex, D3DTEXTUREFILTERTYPE filter, D3DTEXTUREADDRESS address) {
    device->SetTexture(stage, tex);
    device->SetSamplerState(stage, D3DSAMP_MINFILTER, filter);
    device->SetSamplerState(stage, D3DSAMP_MAGFILTER, filter);
    device->SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    device->SetSamplerState(stage, D3DSAMP_ADDRESSU, address);
    device->SetSamplerState(stage, D3DSAMP_ADDRESSV, address);
    device->SetSamplerState(stage, D3DSAMP_ADDRESSW, address);
    device->SetSamplerState(stage, D3DSAMP_SRGBTEXTURE, FALSE);
    device->SetSamplerState(stage, D3DSAMP_MAXMIPLEVEL, 0);
}

void SetTarget(IDirect3DDevice9* device, Target& target, bool clear, uint32_t clearColor) {
    device->SetRenderTarget(0, target.surf);
    BeginFullscreen(device, target.w, target.h);

    if (clear) {
        device->Clear(0, nullptr, D3DCLEAR_TARGET, clearColor, 1.0f, 0);
    }
}

IDirect3DTexture9* CreateTextureFromMemory(IDirect3DDevice9* device, int width, int height, D3DFORMAT format, uint32_t bytesPerPixel, const uint8_t* data, uint32_t srcPitch) {
    IDirect3DTexture9* staging = nullptr;
    IDirect3DTexture9* texture = nullptr;

    if (FAILED(device->CreateTexture(width, height, 1, 0, format, D3DPOOL_SYSTEMMEM, &staging, nullptr))) {
        return nullptr;
    }

    D3DLOCKED_RECT locked;

    if (SUCCEEDED(staging->LockRect(0, &locked, nullptr, 0))) {
        for (int y = 0; y < height; y++) {
            memcpy(static_cast<uint8_t*>(locked.pBits) + y * locked.Pitch, data + y * srcPitch, width * bytesPerPixel);
        }

        staging->UnlockRect(0);

        if (SUCCEEDED(device->CreateTexture(width, height, 1, 0, format, D3DPOOL_DEFAULT, &texture, nullptr))) {
            if (FAILED(device->UpdateTexture(staging, texture))) {
                texture->Release();
                texture = nullptr;
            }
        }
    }

    staging->Release();
    return texture;
}

void SetCommonConstants(IDirect3DDevice9* device, int targetWidth, int targetHeight) {
    SetPixelConstants(device, 0, g_frame.invProj, 4);
    SetPixelConstants(device, 4, g_frame.depthFit, 1);

    float screen[4] = { 1.0f / targetWidth, 1.0f / targetHeight, static_cast<float>(targetWidth), static_cast<float>(targetHeight) };
    SetPixelConstants(device, 5, screen, 1);
}

IDirect3DTexture9* EnsureLinearDepth(bool half) {
    Target& target = half ? s_linearHalf : s_linearFull;
    uint32_t& frame = s_linearFrame[half ? 0 : 1];
    IDirect3DDevice9* device = g_frame.device;

    int width = half ? (g_frame.width + 1) / 2 : g_frame.width;
    int height = half ? (g_frame.height + 1) / 2 : g_frame.height;

    if (!target.Ensure(device, width, height, D3DFMT_R32F)) {
        return nullptr;
    }

    if (frame == g_frame.frameIndex) {
        return target.tex;
    }

    frame = g_frame.frameIndex;

    SetTarget(device, target, false, 0);
    SetCommonConstants(device, width, height);
    SetSampler(device, 0, g_sceneDepthTex, D3DTEXF_POINT, D3DTADDRESS_CLAMP);

    float params[4] = { 1.0f / g_frame.width, 1.0f / g_frame.height, half ? 1.0f : 0.0f, 0.0f };
    SetPixelConstants(device, 8, params, 1);

    device->SetPixelShader(PixelShader(Ps_LinearDepth));
    DrawFullscreen(device);
    device->SetTexture(0, nullptr);

    return target.tex;
}

void ReleaseLinearDepth() {
    s_linearHalf.Release();
    s_linearFull.Release();
    s_linearFrame[0] = 0;
    s_linearFrame[1] = 0;
}

void ReleaseFullscreen() {
    if (s_fullscreenDecl) {
        s_fullscreenDecl->Release();
        s_fullscreenDecl = nullptr;
    }
}

struct SectionQueries {
    IDirect3DQuery9* disjoint;
    IDirect3DQuery9* begin;
    IDirect3DQuery9* end;
    bool issued;
};

static SectionQueries s_queries[Section_Count][2];
static IDirect3DQuery9* s_freqQuery;
static UINT64 s_frequency;
static int s_timerSlot;
static float s_gpuMs[Section_Count];
static bool s_timersBroken;

static bool CreateQueries(IDirect3DDevice9* device, SectionQueries& q) {
    return SUCCEEDED(device->CreateQuery(D3DQUERYTYPE_TIMESTAMPDISJOINT, &q.disjoint))
        && SUCCEEDED(device->CreateQuery(D3DQUERYTYPE_TIMESTAMP, &q.begin))
        && SUCCEEDED(device->CreateQuery(D3DQUERYTYPE_TIMESTAMP, &q.end));
}

void TimersNextFrame(IDirect3DDevice9* device) {
    if (s_timersBroken) {
        return;
    }

    if (!s_frequency) {
        if (!s_freqQuery && FAILED(device->CreateQuery(D3DQUERYTYPE_TIMESTAMPFREQ, &s_freqQuery))) {
            s_timersBroken = true;
            return;
        }

        s_freqQuery->Issue(D3DISSUE_END);
        s_freqQuery->GetData(&s_frequency, sizeof(s_frequency), 0);
    }

    s_timerSlot ^= 1;

    for (int section = 0; section < Section_Count; section++) {
        SectionQueries& q = s_queries[section][s_timerSlot];

        if (!q.issued || !s_frequency) {
            continue;
        }

        BOOL disjoint = FALSE;
        UINT64 begin = 0;
        UINT64 end = 0;

        if (q.disjoint->GetData(&disjoint, sizeof(disjoint), 0) == S_OK
            && q.begin->GetData(&begin, sizeof(begin), 0) == S_OK
            && q.end->GetData(&end, sizeof(end), 0) == S_OK) {
            if (!disjoint) {
                s_gpuMs[section] = static_cast<float>(static_cast<double>(end - begin) * 1000.0 / static_cast<double>(s_frequency));
            }

            q.issued = false;
        }
    }
}

void TimerBegin(IDirect3DDevice9* device, Section section) {
    if (s_timersBroken || !s_frequency) {
        return;
    }

    SectionQueries& q = s_queries[section][s_timerSlot];

    if (q.issued) {
        return;
    }

    if (!q.disjoint && !CreateQueries(device, q)) {
        s_timersBroken = true;
        return;
    }

    q.disjoint->Issue(D3DISSUE_BEGIN);
    q.begin->Issue(D3DISSUE_END);
}

void TimerEnd(IDirect3DDevice9* device, Section section) {
    SectionQueries& q = s_queries[section][s_timerSlot];

    if (s_timersBroken || !q.disjoint || q.issued) {
        return;
    }

    q.end->Issue(D3DISSUE_END);
    q.disjoint->Issue(D3DISSUE_END);
    q.issued = true;
}

float GpuMs(Section section) {
    return s_gpuMs[section];
}

void ReleaseTimers() {
    for (SectionQueries (&pair)[2] : s_queries) {
        for (SectionQueries& q : pair) {
            if (q.disjoint) {
                q.disjoint->Release();
            }

            if (q.begin) {
                q.begin->Release();
            }

            if (q.end) {
                q.end->Release();
            }

            memset(&q, 0, sizeof(q));
        }
    }

    if (s_freqQuery) {
        s_freqQuery->Release();
        s_freqQuery = nullptr;
    }

    s_frequency = 0;
    s_timersBroken = false;
    memset(s_gpuMs, 0, sizeof(s_gpuMs));
}

}
