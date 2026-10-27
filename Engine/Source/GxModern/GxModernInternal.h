#ifndef ENGINE_SOURCE_GXMODERN_GXMODERNINTERNAL_H
#define ENGINE_SOURCE_GXMODERN_GXMODERNINTERNAL_H

#include <cstdint>
#include <d3d9.h>
#include "Engine/Source/GxModern/GxModern.h"
#include "Engine/Source/Tempest/C44Matrix.h"

struct CVar;

namespace GxModern {

enum PsId {
    Ps_Copy,
    Ps_LinearDepth,
    Ps_Gtao,
    Ps_GtaoFull,
    Ps_AoBlur,
    Ps_AoApply,
    Ps_FogMarch,
    Ps_FogMarchHigh,
    Ps_FogMarchShadow,
    Ps_FogMarchShadowHigh,
    Ps_FogMarchShadowRaw,
    Ps_FogMarchShadowRawHigh,
    Ps_FogTemporal,
    Ps_FogApply,
    Ps_ColorGrade,
    Ps_SmaaEdge,
    Ps_SmaaWeights,
    Ps_SmaaBlend,
    Ps_Count
};

enum VsId {
    Vs_Fullscreen,
    Vs_SmaaEdge,
    Vs_SmaaWeights,
    Vs_SmaaBlend,
    Vs_Count
};

enum Section {
    Section_Ssao,
    Section_Fog,
    Section_Grade,
    Section_Smaa,
    Section_Count
};

struct Settings {
    int smaa;
    int ssao;
    float ssaoStrength;
    float ssaoRadius;
    int volumeFog;
    float volumeFogDensity;
    float volumeFogHeightFalloff;
    float volumeFogHeightOffset;
    float volumeFogNoise;
    float volumeFogWind;
    float volumeFogSunGlow;
    float volumeFogAnisotropy;
    int colorGrade;
    float colorGradeExposure;
    float colorGradeContrast;
    float colorGradeSaturation;
    float colorGradeTemperature;
    float colorGradeVignette;
    int godRays;
    float godRaysStrength;
    int extShadowQuality;
};

struct Target {
    IDirect3DTexture9* tex;
    IDirect3DSurface9* surf;
    int w;
    int h;
    D3DFORMAT format;

    bool Ensure(IDirect3DDevice9* device, int width, int height, D3DFORMAT fmt);
    void Release();
};

// Everything the passes need about the world view this frame
struct Frame {
    IDirect3DDevice9* device;
    int width;
    int height;
    RECT worldRect;
    WorldParams params;
    C44Matrix view;
    C44Matrix invView;
    C44Matrix proj;
    float invProj[16];
    float depthFit[4];
    float pixelsPerYard;
    float sunDirView[3];
    float renderOffset[3];
    float smoothedPlayerZ;
    uint32_t frameIndex;
    bool historyReset;
};

extern Frame g_frame;
extern IDirect3DTexture9* g_sceneDepthTex;
extern IDirect3DSurface9* g_sceneDepthSurf;
extern Target g_sceneColor;

const Settings& GetSettings();
IDirect3DDevice9* Device();
bool Supported();
const char* UnsupportedReason();
HWND WindowHandle();
bool GraphicsPanelVisible();
CVar* FindCVar(const char* name);
void SetCVar(const char* name, const char* value);
void SetCVar(const char* name, float value);

IDirect3DPixelShader9* PixelShader(PsId id);
IDirect3DVertexShader9* VertexShader(VsId id);
void ReleaseShaders();

void BeginFullscreen(IDirect3DDevice9* device, int width, int height);
void DrawFullscreen(IDirect3DDevice9* device);
void SetPixelConstants(IDirect3DDevice9* device, uint32_t reg, const float* data, uint32_t count);
void SetVertexConstants(IDirect3DDevice9* device, uint32_t reg, const float* data, uint32_t count);
void SetSampler(IDirect3DDevice9* device, uint32_t stage, IDirect3DBaseTexture9* tex, D3DTEXTUREFILTERTYPE filter, D3DTEXTUREADDRESS address);
void SetTarget(IDirect3DDevice9* device, Target& target, bool clear, uint32_t clearColor);
void ReleaseFullscreen();
IDirect3DTexture9* CreateTextureFromMemory(IDirect3DDevice9* device, int width, int height, D3DFORMAT format, uint32_t bytesPerPixel, const uint8_t* data, uint32_t srcPitch);
void SetCommonConstants(IDirect3DDevice9* device, int targetWidth, int targetHeight);

void TimerBegin(IDirect3DDevice9* device, Section section);
void TimerEnd(IDirect3DDevice9* device, Section section);
void TimersNextFrame(IDirect3DDevice9* device);
float GpuMs(Section section);
void ReleaseTimers();

IDirect3DTexture9* EnsureLinearDepth(bool half);
void ReleaseLinearDepth();

void SsaoRun();
void SsaoRelease();

void FogRun();
void FogRelease();
void FogResetHistory();

IDirect3DTexture9* GradeRun(IDirect3DTexture9* source);
void GradeRelease();
const char* GradeStatus();

void SmaaRun(IDirect3DTexture9* source, IDirect3DSurface9* destination, int width, int height);
void SmaaRelease();

void PanelShutdown();
void PanelReleaseDeviceObjects();

}

#endif
