#include <cmath>
#include <cstring>
#include "Engine/Source/GxModern/GxModernInternal.h"
#include "Engine/Source/Gx/CGxTex.h"
#include "Engine/Source/Services/Texture.h"
#include "Engine/Source/ShaderEffect/ShaderEffect.h"
#include "Engine/Source/ShaderEffect/ShadowCache.h"

namespace GxModern {

static const int NOISE_SIZE = 32;
static const float NOISE_SCALE = 0.01f;

static IDirect3DVolumeTexture9* s_noise;
static Target s_current;
static Target s_history[2];
static int s_historyIndex;
static bool s_historyValid;
static C44Matrix s_prevMatrix;
static float s_prevCamera[3];
static bool s_hasPrev;
static int s_formatChecked;
static D3DFORMAT s_fogFormat = D3DFMT_A8R8G8B8;

static float Hash(int x, int y, int z, int period) {
    x = ((x % period) + period) % period;
    y = ((y % period) + period) % period;
    z = ((z % period) + period) % period;
    uint32_t h = static_cast<uint32_t>(x) * 73856093u ^ static_cast<uint32_t>(y) * 19349663u ^ static_cast<uint32_t>(z) * 83492791u;
    h ^= h >> 13;
    h *= 0x5BD1E995u;
    h ^= h >> 15;
    return static_cast<float>(h & 0xFFFFu) / 65535.0f;
}

static float ValueNoise(float x, float y, float z, int period) {
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    int iz = static_cast<int>(std::floor(z));
    float fx = x - ix;
    float fy = y - iy;
    float fz = z - iz;
    fx = fx * fx * (3.0f - 2.0f * fx);
    fy = fy * fy * (3.0f - 2.0f * fy);
    fz = fz * fz * (3.0f - 2.0f * fz);

    float result = 0.0f;

    for (int i = 0; i < 8; i++) {
        int dx = i & 1;
        int dy = (i >> 1) & 1;
        int dz = (i >> 2) & 1;
        float weight = (dx ? fx : 1.0f - fx) * (dy ? fy : 1.0f - fy) * (dz ? fz : 1.0f - fz);
        result += weight * Hash(ix + dx, iy + dy, iz + dz, period);
    }

    return result;
}

static IDirect3DVolumeTexture9* CreateNoise(IDirect3DDevice9* device) {
    IDirect3DVolumeTexture9* staging = nullptr;
    IDirect3DVolumeTexture9* volume = nullptr;

    if (FAILED(device->CreateVolumeTexture(NOISE_SIZE, NOISE_SIZE, NOISE_SIZE, 1, 0, D3DFMT_L8, D3DPOOL_SYSTEMMEM, &staging, nullptr))) {
        return nullptr;
    }

    D3DLOCKED_BOX box;

    if (SUCCEEDED(staging->LockBox(0, &box, nullptr, 0))) {
        for (int z = 0; z < NOISE_SIZE; z++) {
            for (int y = 0; y < NOISE_SIZE; y++) {
                uint8_t* row = static_cast<uint8_t*>(box.pBits) + z * box.SlicePitch + y * box.RowPitch;

                for (int x = 0; x < NOISE_SIZE; x++) {
                    float u = static_cast<float>(x) / NOISE_SIZE;
                    float v = static_cast<float>(y) / NOISE_SIZE;
                    float w = static_cast<float>(z) / NOISE_SIZE;
                    float n = 0.65f * ValueNoise(u * 4.0f, v * 4.0f, w * 4.0f, 4) + 0.35f * ValueNoise(u * 8.0f, v * 8.0f, w * 8.0f, 8);
                    row[x] = static_cast<uint8_t>((0.35f + 0.65f * n) * 255.0f);
                }
            }
        }

        staging->UnlockBox(0);

        if (SUCCEEDED(device->CreateVolumeTexture(NOISE_SIZE, NOISE_SIZE, NOISE_SIZE, 1, 0, D3DFMT_L8, D3DPOOL_DEFAULT, &volume, nullptr))) {
            if (FAILED(device->UpdateTexture(staging, volume))) {
                volume->Release();
                volume = nullptr;
            }
        }
    }

    staging->Release();
    return volume;
}

static IDirect3DTexture9* BandTexture(int band) {
    CShadowBand& shadowBand = CShadowCache::s_extShadowBands[band];
    CTexture* texture = shadowBand.m_textures[shadowBand.m_current];

    if (!texture) {
        return nullptr;
    }

    CGxTex* gxTex = TextureGetGxTex(texture, 0, nullptr);
    return gxTex ? static_cast<IDirect3DTexture9*>(gxTex->m_apiSpecificData) : nullptr;
}

static void ChooseFormat(IDirect3DDevice9* device) {
    if (s_formatChecked) {
        return;
    }

    s_formatChecked = 1;

    IDirect3D9* d3d = nullptr;
    D3DDISPLAYMODE mode;
    D3DDEVICE_CREATION_PARAMETERS creation;

    if (FAILED(device->GetDirect3D(&d3d)) || FAILED(device->GetCreationParameters(&creation)) || FAILED(d3d->GetAdapterDisplayMode(creation.AdapterOrdinal, &mode))) {
        if (d3d) {
            d3d->Release();
        }

        return;
    }

    if (SUCCEEDED(d3d->CheckDeviceFormat(creation.AdapterOrdinal, creation.DeviceType, mode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_A16B16G16R16F))) {
        s_fogFormat = D3DFMT_A16B16G16R16F;
    }

    d3d->Release();
}

void FogResetHistory() {
    s_historyValid = false;
    s_hasPrev = false;
}

void FogRun() {
    const Settings& settings = GetSettings();
    const WorldParams& params = g_frame.params;
    IDirect3DDevice9* device = g_frame.device;

    float layerWeight = 1.0f - params.interiorBlend;
    bool enabled = settings.volumeFog && !params.underwater && layerWeight > 0.001f;

    // Remember the camera even while off so the first enabled frame has a valid history
    C44Matrix absoluteView(1.0f);
    absoluteView.d0 = -g_frame.renderOffset[0];
    absoluteView.d1 = -g_frame.renderOffset[1];
    absoluteView.d2 = -g_frame.renderOffset[2];
    C44Matrix currentMatrix = absoluteView * g_frame.view * g_frame.proj;

    float dx = params.cameraPos[0] - s_prevCamera[0];
    float dy = params.cameraPos[1] - s_prevCamera[1];
    float dz = params.cameraPos[2] - s_prevCamera[2];
    bool jumped = dx * dx + dy * dy + dz * dz > 20.0f * 20.0f;
    s_prevCamera[0] = params.cameraPos[0];
    s_prevCamera[1] = params.cameraPos[1];
    s_prevCamera[2] = params.cameraPos[2];

    if (!enabled) {
        s_historyValid = false;
        s_hasPrev = false;
        return;
    }

    ChooseFormat(device);

    if (!s_noise) {
        s_noise = CreateNoise(device);
    }

    int fogWidth = (g_frame.width + 1) / 2;
    int fogHeight = (g_frame.height + 1) / 2;
    bool resized = s_current.w != fogWidth || s_current.h != fogHeight || s_current.format != s_fogFormat;

    if (!s_noise
        || !s_current.Ensure(device, fogWidth, fogHeight, s_fogFormat)
        || !s_history[0].Ensure(device, fogWidth, fogHeight, s_fogFormat)
        || !s_history[1].Ensure(device, fogWidth, fogHeight, s_fogFormat)) {
        return;
    }

    if (g_frame.historyReset || jumped || resized) {
        s_historyValid = false;
    }

    TimerBegin(device, Section_Fog);

    IDirect3DTexture9* halfLinear = EnsureLinearDepth(true);

    if (!halfLinear) {
        TimerEnd(device, Section_Fog);
        return;
    }

    bool godRays = settings.godRays && settings.extShadowQuality >= 3 && !CShadowCache::s_recreateTextures
        && CShadowCache::GetShadowMode() > SHADOW_MEDIUM
        && BandTexture(0) && BandTexture(1) && BandTexture(2);
    bool hwPcf = CShaderEffect::s_usePcfFiltering != 0;
    bool high = settings.volumeFog >= 2;

    PsId marchShader;

    if (!godRays) {
        marchShader = high ? Ps_FogMarchHigh : Ps_FogMarch;
    } else if (hwPcf) {
        marchShader = high ? Ps_FogMarchShadowHigh : Ps_FogMarchShadow;
    } else {
        marchShader = high ? Ps_FogMarchShadowRawHigh : Ps_FogMarchShadowRaw;
    }

    SetTarget(device, s_current, false, 0);
    SetCommonConstants(device, fogWidth, fogHeight);
    SetSampler(device, 0, g_sceneDepthTex, D3DTEXF_POINT, D3DTADDRESS_CLAMP);
    SetSampler(device, 1, s_noise, D3DTEXF_LINEAR, D3DTADDRESS_WRAP);

    if (godRays) {
        D3DTEXTUREFILTERTYPE filter = hwPcf ? D3DTEXF_LINEAR : D3DTEXF_POINT;

        for (int band = 0; band < 3; band++) {
            SetSampler(device, 2 + band, BandTexture(band), filter, D3DTADDRESS_CLAMP);
        }

        float shadow[9 * 4];

        for (int band = 0; band < 3; band++) {
            memcpy(&shadow[band * 12], &CShadowCache::s_shadowMatrices[3 + 3 * band].x, sizeof(float) * 12);
        }

        SetPixelConstants(device, 20, shadow, 9);
    }

    const float* invView = &g_frame.invView.a0;
    SetPixelConstants(device, 8, invView, 4);

    float time = params.time;
    float offsetTime[4] = { g_frame.renderOffset[0], g_frame.renderOffset[1], g_frame.renderOffset[2], 0.0f };
    float fogColorEnd[4] = { params.fogColor[0], params.fogColor[1], params.fogColor[2], params.fogEnd };
    float glow = settings.volumeFogSunGlow;
    float sunColorG[4] = { params.sunColor[0] * glow, params.sunColor[1] * glow, params.sunColor[2] * glow, settings.volumeFogAnisotropy };
    float toSun[4] = { -g_frame.sunDirView[0], -g_frame.sunDirView[1], -g_frame.sunDirView[2], 0.0f };
    float density[4] = { settings.volumeFogDensity, settings.volumeFogHeightFalloff, g_frame.smoothedPlayerZ + settings.volumeFogHeightOffset, settings.volumeFogNoise };
    float drift = settings.volumeFogWind * time * NOISE_SCALE;
    float wind[4] = { 0.8f * drift, 0.55f * drift, 0.05f * drift, layerWeight };
    float misc[4] = { params.farClip, static_cast<float>(g_frame.frameIndex & 1023), settings.godRaysStrength, 0.0f };
    SetPixelConstants(device, 12, offsetTime, 1);
    SetPixelConstants(device, 13, fogColorEnd, 1);
    SetPixelConstants(device, 14, sunColorG, 1);
    SetPixelConstants(device, 15, toSun, 1);
    SetPixelConstants(device, 16, density, 1);
    SetPixelConstants(device, 17, wind, 1);
    SetPixelConstants(device, 18, misc, 1);

    device->SetPixelShader(PixelShader(marchShader));
    DrawFullscreen(device);

    for (uint32_t stage = 1; stage < 5; stage++) {
        device->SetTexture(stage, nullptr);
    }

    int next = s_historyIndex ^ 1;

    SetTarget(device, s_history[next], false, 0);
    SetCommonConstants(device, fogWidth, fogHeight);
    SetSampler(device, 0, s_current.tex, D3DTEXF_POINT, D3DTADDRESS_CLAMP);
    SetSampler(device, 1, s_history[s_historyIndex].tex, D3DTEXF_LINEAR, D3DTADDRESS_CLAMP);
    SetSampler(device, 2, g_sceneDepthTex, D3DTEXF_POINT, D3DTADDRESS_CLAMP);
    SetPixelConstants(device, 8, invView, 4);
    SetPixelConstants(device, 12, offsetTime, 1);

    bool useHistory = s_historyValid && s_hasPrev;
    float temporal[4] = { useHistory ? 0.9f : 0.0f, 0.0f, 0.0f, 0.0f };
    SetPixelConstants(device, 40, &s_prevMatrix.a0, 4);
    SetPixelConstants(device, 44, temporal, 1);

    device->SetPixelShader(PixelShader(Ps_FogTemporal));
    DrawFullscreen(device);

    device->SetRenderTarget(0, g_sceneColor.surf);
    BeginFullscreen(device, g_frame.width, g_frame.height);
    SetCommonConstants(device, g_frame.width, g_frame.height);
    SetSampler(device, 0, g_sceneDepthTex, D3DTEXF_POINT, D3DTADDRESS_CLAMP);
    SetSampler(device, 1, s_history[next].tex, D3DTEXF_POINT, D3DTADDRESS_CLAMP);
    SetSampler(device, 2, halfLinear, D3DTEXF_POINT, D3DTADDRESS_CLAMP);

    float fogSize[4] = { static_cast<float>(fogWidth), static_cast<float>(fogHeight), 0.0f, 0.0f };
    SetPixelConstants(device, 30, fogSize, 1);

    device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
    device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
    device->SetPixelShader(PixelShader(Ps_FogApply));
    DrawFullscreen(device);

    for (uint32_t stage = 0; stage < 3; stage++) {
        device->SetTexture(stage, nullptr);
    }

    s_historyIndex = next;
    s_historyValid = true;
    s_prevMatrix = currentMatrix;
    s_hasPrev = true;

    TimerEnd(device, Section_Fog);
}

void FogRelease() {
    if (s_noise) {
        s_noise->Release();
        s_noise = nullptr;
    }

    s_current.Release();
    s_history[0].Release();
    s_history[1].Release();
    s_historyValid = false;
    s_hasPrev = false;
    s_formatChecked = 0;
    s_fogFormat = D3DFMT_A8R8G8B8;
}

}
