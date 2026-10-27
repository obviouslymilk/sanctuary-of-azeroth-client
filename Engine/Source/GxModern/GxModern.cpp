#include <cmath>
#include <cstdio>
#include <cstring>
#include "Engine/Source/GxModern/GxModernInternal.h"
#include "Engine/Source/ConsoleBase/ConsoleVar.h"
#include "Engine/Source/ConsoleBase/Types.h"
#include "Engine/Source/Gx/CGxDevice/CGxDevice.h"
#include "Engine/Source/Gx/CGxDeviceD3d/CGxDeviceD3d.h"
#include "Engine/Source/Gx/CGxDeviceD3d9Ex/CGxDeviceD3d9Ex.h"
#include "Engine/Source/Gx/Gx.h"
#include "Engine/Source/ShaderEffect/ShadowCache.h"
#include "Engine/Source/Tempest/CRect.h"

namespace GxModern {

enum CVarIndex {
    CV_Render,
    CV_Smaa,
    CV_Ssao,
    CV_SsaoStrength,
    CV_SsaoRadius,
    CV_VolumeFog,
    CV_VolumeFogDensity,
    CV_VolumeFogHeightFalloff,
    CV_VolumeFogHeightOffset,
    CV_VolumeFogNoise,
    CV_VolumeFogWind,
    CV_VolumeFogSunGlow,
    CV_VolumeFogAnisotropy,
    CV_ColorGrade,
    CV_ColorGradeExposure,
    CV_ColorGradeContrast,
    CV_ColorGradeSaturation,
    CV_ColorGradeTemperature,
    CV_ColorGradeVignette,
    CV_ColorGradeLut,
    CV_ShadowMapSize,
    CV_ShadowBand0Radius,
    CV_ShadowBand1Radius,
    CV_ShadowBand2Radius,
    CV_GodRays,
    CV_GodRaysStrength,
    CV_GraphicsPanel,
    CV_Count
};

struct CVarDef {
    const char* name;
    const char* help;
    const char* value;
};

static const CVarDef s_cvarDefs[CV_Count] = {
    { "gxModernRender", "Modern rendering pipeline (SSAO, volumetric fog, grading, SMAA)", "0" },
    { "smaa", "SMAA anti-aliasing while the modern pipeline is on", "1" },
    { "ssao", "Ambient occlusion: 0 off, 1 half resolution, 2 full resolution", "1" },
    { "ssaoStrength", "Ambient occlusion strength (0-1)", "0.6" },
    { "ssaoRadius", "Ambient occlusion radius in yards", "1.0" },
    { "volumeFog", "Volumetric fog: 0 off, 1 = 16 steps, 2 = 32 steps", "1" },
    { "volumeFogDensity", "Volumetric fog extinction per yard at base height", "0.02" },
    { "volumeFogHeightFalloff", "Volumetric fog density falloff per yard above base height", "0.15" },
    { "volumeFogHeightOffset", "Volumetric fog base height relative to the player in yards", "-5.0" },
    { "volumeFogNoise", "Volumetric fog drifting noise strength (0-1)", "0.5" },
    { "volumeFogWind", "Volumetric fog noise drift speed in yards per second", "1.0" },
    { "volumeFogSunGlow", "Volumetric fog in-scatter strength toward the sun or moon", "1.0" },
    { "volumeFogAnisotropy", "Volumetric fog phase anisotropy", "0.6" },
    { "colorGrade", "Color grading", "1" },
    { "colorGradeExposure", "Color grading exposure in stops", "0.0" },
    { "colorGradeContrast", "Color grading contrast", "1.0" },
    { "colorGradeSaturation", "Color grading saturation", "1.0" },
    { "colorGradeTemperature", "Color grading temperature (-1 cool, 1 warm)", "0.0" },
    { "colorGradeVignette", "Color grading vignette (0-1)", "0.0" },
    { "colorGradeLut", "Path to a 256x16 TGA color lookup table", "" },
    { "shadowMapSize", "Cascaded shadow map size (2048 or 4096)", "2048" },
    { "shadowBand0Radius", "Cascaded shadow near band radius in yards", "40" },
    { "shadowBand1Radius", "Cascaded shadow middle band radius in yards", "160" },
    { "shadowBand2Radius", "Cascaded shadow far band radius in yards", "640" },
    { "godRays", "God rays through the volumetric fog (needs extShadowQuality 3 or higher)", "1" },
    { "godRaysStrength", "God ray contrast", "1.0" },
    { "graphicsPanel", "Show the graphics settings panel", "0" },
};

static CVar* s_vars[CV_Count];
static Settings s_settings;

Frame g_frame;
IDirect3DTexture9* g_sceneDepthTex;
IDirect3DSurface9* g_sceneDepthSurf;
Target g_sceneColor;

static IDirect3DStateBlock9* s_stateBlock;
static IDirect3DSurface9* s_savedColor;
static IDirect3DSurface9* s_savedDepth;
static bool s_frameActive;
static uint32_t s_frameCounter;
static bool s_wasActive;
static IDirect3DDevice9* s_supportDevice;
static bool s_supported;
static const char* s_unsupportedReason = "";
static float s_smoothedZ;
static bool s_smoothedValid;

static bool ShadowCVarCallback(CVar* var, const char* oldValue, const char* value, void* arg) {
    CShadowCache::s_recreateTextures = 1;
    return true;
}

void RegisterCVars() {
    for (int i = 0; i < CV_Count; i++) {
        bool (*callback)(CVar*, const char*, const char*, void*) = nullptr;

        if (i >= CV_ShadowMapSize && i <= CV_ShadowBand2Radius) {
            callback = ShadowCVarCallback;
        }

        s_vars[i] = CVar::Register(s_cvarDefs[i].name, s_cvarDefs[i].help, 1, s_cvarDefs[i].value, callback, CATEGORY_GRAPHICS, false, nullptr, false);
    }
}

CVar* FindCVar(const char* name) {
    for (int i = 0; i < CV_Count; i++) {
        if (!strcmp(s_cvarDefs[i].name, name)) {
            return s_vars[i];
        }
    }

    return CVar::Lookup(name);
}

void SetCVar(const char* name, const char* value) {
    CVar* var = FindCVar(name);

    if (var) {
        var->Set(value, true, false, false, true);
    }
}

void SetCVar(const char* name, float value) {
    char text[32];
    snprintf(text, sizeof(text), "%g", value);
    SetCVar(name, text);
}

static int IntValue(CVarIndex index) {
    return s_vars[index] ? s_vars[index]->m_intValue : 0;
}

static float FloatValue(CVarIndex index) {
    return s_vars[index] ? s_vars[index]->m_floatValue : 0.0f;
}

static void RefreshSettings() {
    Settings& s = s_settings;
    s.smaa = IntValue(CV_Smaa);
    s.ssao = IntValue(CV_Ssao);
    s.ssaoStrength = FloatValue(CV_SsaoStrength);
    s.ssaoRadius = FloatValue(CV_SsaoRadius);
    s.volumeFog = IntValue(CV_VolumeFog);
    s.volumeFogDensity = FloatValue(CV_VolumeFogDensity);
    s.volumeFogHeightFalloff = FloatValue(CV_VolumeFogHeightFalloff);
    s.volumeFogHeightOffset = FloatValue(CV_VolumeFogHeightOffset);
    s.volumeFogNoise = FloatValue(CV_VolumeFogNoise);
    s.volumeFogWind = FloatValue(CV_VolumeFogWind);
    s.volumeFogSunGlow = FloatValue(CV_VolumeFogSunGlow);
    s.volumeFogAnisotropy = FloatValue(CV_VolumeFogAnisotropy);
    s.colorGrade = IntValue(CV_ColorGrade);
    s.colorGradeExposure = FloatValue(CV_ColorGradeExposure);
    s.colorGradeContrast = FloatValue(CV_ColorGradeContrast);
    s.colorGradeSaturation = FloatValue(CV_ColorGradeSaturation);
    s.colorGradeTemperature = FloatValue(CV_ColorGradeTemperature);
    s.colorGradeVignette = FloatValue(CV_ColorGradeVignette);
    s.godRays = IntValue(CV_GodRays);
    s.godRaysStrength = FloatValue(CV_GodRaysStrength);

    CVar* shadowQuality = CVar::Lookup("extShadowQuality");
    s.extShadowQuality = shadowQuality ? shadowQuality->m_intValue : 0;
}

const Settings& GetSettings() {
    return s_settings;
}

bool WantsPipelineAtDeviceCreate() {
    return IntValue(CV_Render) != 0;
}

IDirect3DDevice9* Device() {
    if (!g_theGxDevicePtr) {
        return nullptr;
    }

    switch (g_theGxDevicePtr->DeviceApi()) {
        case GxApi_D3d9Ex:
            return static_cast<CGxDeviceD3d9Ex*>(g_theGxDevicePtr)->m_d3dDevice;

        case GxApi_D3d9:
            return static_cast<CGxDeviceD3d*>(g_theGxDevicePtr)->m_d3dDevice;

        default:
            return nullptr;
    }
}

HWND WindowHandle() {
    if (!g_theGxDevicePtr) {
        return nullptr;
    }

    switch (g_theGxDevicePtr->DeviceApi()) {
        case GxApi_D3d9Ex:
            return static_cast<CGxDeviceD3d9Ex*>(g_theGxDevicePtr)->m_hwnd;

        case GxApi_D3d9:
            return static_cast<CGxDeviceD3d*>(g_theGxDevicePtr)->m_hwnd;

        default:
            return nullptr;
    }
}

// The device's default surface slots; swapped for ours while the world renders
static bool DefaultSurfaces(IDirect3DSurface9**& color, IDirect3DSurface9**& depth) {
    if (!g_theGxDevicePtr) {
        return false;
    }

    switch (g_theGxDevicePtr->DeviceApi()) {
        case GxApi_D3d9Ex: {
            CGxDeviceD3d9Ex* device = static_cast<CGxDeviceD3d9Ex*>(g_theGxDevicePtr);
            color = &device->m_defColorSurface;
            depth = &device->m_defDepthSurface;
            return true;
        }

        case GxApi_D3d9: {
            CGxDeviceD3d* device = static_cast<CGxDeviceD3d*>(g_theGxDevicePtr);
            color = &device->m_defColorSurface;
            depth = &device->m_defDepthSurface;
            return true;
        }

        default:
            return false;
    }
}

bool Supported() {
    IDirect3DDevice9* device = Device();

    if (!device) {
        s_unsupportedReason = "Direct3D 9 only (OpenGL is not supported)";
        return false;
    }

    if (device == s_supportDevice) {
        return s_supported;
    }

    s_supportDevice = device;
    s_supported = false;
    s_unsupportedReason = "INTZ depth textures are not supported by this GPU";

    IDirect3D9* d3d = nullptr;
    D3DDEVICE_CREATION_PARAMETERS creation;
    D3DDISPLAYMODE mode;

    if (SUCCEEDED(device->GetDirect3D(&d3d))) {
        if (SUCCEEDED(device->GetCreationParameters(&creation)) && SUCCEEDED(d3d->GetAdapterDisplayMode(creation.AdapterOrdinal, &mode))) {
            s_supported = SUCCEEDED(d3d->CheckDeviceFormat(creation.AdapterOrdinal, creation.DeviceType, mode.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, static_cast<D3DFORMAT>(MAKEFOURCC('I', 'N', 'T', 'Z'))));
        }

        d3d->Release();
    }

    return s_supported;
}

const char* UnsupportedReason() {
    return s_unsupportedReason;
}

bool Active() {
    return IntValue(CV_Render) != 0 && Supported();
}

bool GraphicsPanelVisible() {
    return IntValue(CV_GraphicsPanel) != 0;
}

static void ReleaseTargets() {
    SsaoRelease();
    FogRelease();
    GradeRelease();
    SmaaRelease();
    ReleaseLinearDepth();
    g_sceneColor.Release();

    if (g_sceneDepthSurf) {
        g_sceneDepthSurf->Release();
        g_sceneDepthSurf = nullptr;
    }

    if (g_sceneDepthTex) {
        g_sceneDepthTex->Release();
        g_sceneDepthTex = nullptr;
    }
}

static void RestoreDefaultSurfaces() {
    IDirect3DSurface9** color;
    IDirect3DSurface9** depth;

    if (s_savedColor && DefaultSurfaces(color, depth)) {
        *color = s_savedColor;
        *depth = s_savedDepth;
    }

    s_savedColor = nullptr;
    s_savedDepth = nullptr;
}

void ReleaseResources() {
    if (s_frameActive) {
        RestoreDefaultSurfaces();
        s_frameActive = false;
    }

    if (s_stateBlock) {
        s_stateBlock->Release();
        s_stateBlock = nullptr;
    }

    ReleaseTargets();
    ReleaseFullscreen();
    ReleaseShaders();
    ReleaseTimers();
    PanelReleaseDeviceObjects();
    s_supportDevice = nullptr;
    s_wasActive = false;
}

void Shutdown() {
    ReleaseResources();
    PanelShutdown();
}

static bool EnsureSceneTargets(IDirect3DDevice9* device, int width, int height) {
    if (g_sceneDepthTex && g_sceneColor.tex && g_sceneColor.w == width && g_sceneColor.h == height) {
        return true;
    }

    ReleaseTargets();
    FogResetHistory();

    if (!g_sceneColor.Ensure(device, width, height, D3DFMT_A8R8G8B8)) {
        return false;
    }

    HRESULT result = device->CreateTexture(width, height, 1, D3DUSAGE_DEPTHSTENCIL, static_cast<D3DFORMAT>(MAKEFOURCC('I', 'N', 'T', 'Z')), D3DPOOL_DEFAULT, &g_sceneDepthTex, nullptr);

    if (FAILED(result)) {
        ReleaseTargets();
        return false;
    }

    g_sceneDepthTex->GetSurfaceLevel(0, &g_sceneDepthSurf);
    return true;
}

static void MarkViewportDirty() {
    g_theGxDevicePtr->m_viewportDirty = 1;
    g_theGxDevicePtr->m_scissorRectDirty = 1;
}

void BeginWorld() {
    s_frameActive = false;
    RefreshSettings();

    if (!Active()) {
        if (s_wasActive) {
            ReleaseTargets();
            s_wasActive = false;
        }

        return;
    }

    IDirect3DDevice9* device = Device();
    CRect window = { 0.0f, 0.0f, 0.0f, 0.0f };
    g_theGxDevicePtr->CapsWindowSize(window);
    int width = static_cast<int>(window.maxX - window.minX);
    int height = static_cast<int>(window.maxY - window.minY);

    IDirect3DSurface9** color;
    IDirect3DSurface9** depth;

    if (width <= 0 || height <= 0 || !DefaultSurfaces(color, depth) || !EnsureSceneTargets(device, width, height)) {
        return;
    }

    if (!s_wasActive) {
        FogResetHistory();
    }

    s_wasActive = true;
    TimersNextFrame(device);

    s_savedColor = *color;
    s_savedDepth = *depth;
    *color = g_sceneColor.surf;
    *depth = g_sceneDepthSurf;

    device->SetRenderTarget(0, g_sceneColor.surf);
    device->SetDepthStencilSurface(g_sceneDepthSurf);
    MarkViewportDirty();

    g_frame.device = device;
    g_frame.width = width;
    g_frame.height = height;
    s_frameActive = true;
}

static void BuildFrame(const WorldParams& params) {
    g_frame.params = params;
    g_frame.frameIndex = ++s_frameCounter;

    GxXformView(g_frame.view);
    GxXformProjection(g_frame.proj);
    g_frame.invView = g_frame.view.Inverse(g_frame.view.Determinant());
    C44Matrix invProj = g_frame.proj.Inverse(g_frame.proj.Determinant());

    float zLow = g_theGxDevicePtr->m_viewport.m_z.m_l;
    float zHigh = g_theGxDevicePtr->m_viewport.m_z.m_h;
    float zRange = zHigh - zLow;

    if (zRange < 1e-4f) {
        zRange = 1.0f;
        zLow = 0.0f;
    }

    C44Matrix remap(1.0f);
    remap.c2 = 1.0f / zRange;
    remap.d2 = -zLow / zRange;
    C44Matrix invProjRaw = remap * invProj;
    memcpy(g_frame.invProj, &invProjRaw.a0, sizeof(g_frame.invProj));

    // Linear depth w = 1 / (A*z + B), clip.z = z*w = alpha*w + beta
    const C44Matrix& p = g_frame.proj;
    const float zs[2] = { 0.25f, 0.75f };
    float w[2];

    for (int i = 0; i < 2; i++) {
        const float* m = g_frame.invProj;
        float hx = zs[i] * m[8] + m[12];
        float hy = zs[i] * m[9] + m[13];
        float hz = zs[i] * m[10] + m[14];
        float hw = zs[i] * m[11] + m[15];
        hx /= hw;
        hy /= hw;
        hz /= hw;
        w[i] = hx * p.a3 + hy * p.b3 + hz * p.c3 + p.d3;
    }

    float a = (1.0f / w[1] - 1.0f / w[0]) / (zs[1] - zs[0]);
    float b = 1.0f / w[0] - a * zs[0];
    float alpha = (zs[1] * w[1] - zs[0] * w[0]) / (w[1] - w[0]);
    float beta = zs[0] * w[0] - alpha * w[0];
    g_frame.depthFit[0] = a;
    g_frame.depthFit[1] = b;
    g_frame.depthFit[2] = alpha;
    g_frame.depthFit[3] = beta;

    // World yards covered by one NDC x unit per unit of linear depth, at the screen center
    {
        const float* m = g_frame.invProj;
        float z = 0.5f;
        float p0[3];
        float p1[3];

        for (int k = 0; k < 2; k++) {
            float x = static_cast<float>(k);
            float h[4];

            for (int c = 0; c < 4; c++) {
                h[c] = x * m[c] + z * m[8 + c] + m[12 + c];
            }

            float* out = k ? p1 : p0;
            out[0] = h[0] / h[3];
            out[1] = h[1] / h[3];
            out[2] = h[2] / h[3];
        }

        float dx = p1[0] - p0[0];
        float dy = p1[1] - p0[1];
        float dz = p1[2] - p0[2];
        float depth = 1.0f / (a * z + b);
        float yardsPerNdc = std::sqrt(dx * dx + dy * dy + dz * dz) / depth;
        g_frame.pixelsPerYard = yardsPerNdc > 1e-6f ? (g_frame.width * 0.5f) / yardsPerNdc : 0.0f;
    }

    const C3Vector sun(params.sunDir[0], params.sunDir[1], params.sunDir[2]);
    const C44Matrix& v = g_frame.view;
    float sx = v.a0 * sun.x + v.b0 * sun.y + v.c0 * sun.z;
    float sy = v.a1 * sun.x + v.b1 * sun.y + v.c1 * sun.z;
    float sz = v.a2 * sun.x + v.b2 * sun.y + v.c2 * sun.z;
    float sl = std::sqrt(sx * sx + sy * sy + sz * sz);
    sl = sl > 1e-6f ? 1.0f / sl : 0.0f;
    g_frame.sunDirView[0] = sx * sl;
    g_frame.sunDirView[1] = sy * sl;
    g_frame.sunDirView[2] = sz * sl;

    g_frame.renderOffset[0] = params.cameraPos[0] - g_frame.invView.d0;
    g_frame.renderOffset[1] = params.cameraPos[1] - g_frame.invView.d1;
    g_frame.renderOffset[2] = params.cameraPos[2] - g_frame.invView.d2;

    if (!s_smoothedValid || std::fabs(params.playerZ - s_smoothedZ) > 50.0f) {
        s_smoothedZ = params.playerZ;
        s_smoothedValid = true;
    } else {
        float blend = 1.0f - std::exp(-params.deltaSec / 10.0f);
        s_smoothedZ += (params.playerZ - s_smoothedZ) * blend;
    }

    g_frame.smoothedPlayerZ = s_smoothedZ;
    g_frame.historyReset = false;
}

static bool CaptureState(IDirect3DDevice9* device) {
    if (!s_stateBlock && FAILED(device->CreateStateBlock(D3DSBT_ALL, &s_stateBlock))) {
        s_stateBlock = nullptr;
        return false;
    }

    return SUCCEEDED(s_stateBlock->Capture());
}

void AfterOpaque(const WorldParams& params) {
    if (!s_frameActive) {
        return;
    }

    IDirect3DDevice9* device = g_frame.device;

    if (!CaptureState(device)) {
        return;
    }

    BuildFrame(params);

    device->SetDepthStencilSurface(nullptr);

    SsaoRun();
    FogRun();

    device->SetRenderTarget(0, g_sceneColor.surf);
    device->SetDepthStencilSurface(g_sceneDepthSurf);
    s_stateBlock->Apply();
}

static void CopyToTarget(IDirect3DDevice9* device, IDirect3DTexture9* source, IDirect3DSurface9* destination) {
    device->SetRenderTarget(0, destination);
    BeginFullscreen(device, g_frame.width, g_frame.height);

    RECT rect = g_frame.worldRect;
    device->SetScissorRect(&rect);
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

    SetSampler(device, 0, source, D3DTEXF_POINT, D3DTADDRESS_CLAMP);
    device->SetPixelShader(PixelShader(Ps_Copy));
    DrawFullscreen(device);
    device->SetTexture(0, nullptr);
}

void EndWorld(float minX, float minY, float maxX, float maxY) {
    if (!s_frameActive) {
        return;
    }

    s_frameActive = false;

    IDirect3DDevice9* device = g_frame.device;
    IDirect3DSurface9* destination = s_savedColor;
    IDirect3DSurface9* destinationDepth = s_savedDepth;
    RestoreDefaultSurfaces();
    device->SetDepthStencilSurface(destinationDepth);

    int width = g_frame.width;
    int height = g_frame.height;
    g_frame.worldRect.left = static_cast<LONG>(std::floor(minX * width));
    g_frame.worldRect.right = static_cast<LONG>(std::ceil(maxX * width));
    g_frame.worldRect.top = static_cast<LONG>(std::floor((1.0f - maxY) * height));
    g_frame.worldRect.bottom = static_cast<LONG>(std::ceil((1.0f - minY) * height));

    if (!destination || !CaptureState(device)) {
        device->SetRenderTarget(0, destination);
        device->SetDepthStencilSurface(destinationDepth);
        MarkViewportDirty();
        return;
    }

    IDirect3DTexture9* final = GradeRun(g_sceneColor.tex);

    if (GetSettings().smaa) {
        SmaaRun(final, destination, width, height);
    } else {
        CopyToTarget(device, final, destination);
    }

    device->SetRenderTarget(0, destination);
    device->SetDepthStencilSurface(destinationDepth);
    s_stateBlock->Apply();
    MarkViewportDirty();
}

}
