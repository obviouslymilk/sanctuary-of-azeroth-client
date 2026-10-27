#include <cmath>
#include <cstring>
#include "Engine/Source/GxModern/GxModernInternal.h"
#include "Engine/Source/ConsoleBase/ConsoleVar.h"
#include "Engine/Source/Images/tga.h"

namespace GxModern {

static Target s_graded;
static IDirect3DTexture9* s_lut;
static char s_lutPath[260];
static char s_status[96] = "";

static bool IsNeutral(const Settings& settings) {
    return settings.colorGradeExposure == 0.0f
        && settings.colorGradeContrast == 1.0f
        && settings.colorGradeSaturation == 1.0f
        && settings.colorGradeTemperature == 0.0f
        && settings.colorGradeVignette == 0.0f;
}

static void LoadLut(IDirect3DDevice9* device, const char* path) {
    if (s_lut) {
        s_lut->Release();
        s_lut = nullptr;
    }

    strncpy(s_lutPath, path, sizeof(s_lutPath) - 1);
    s_lutPath[sizeof(s_lutPath) - 1] = '\0';
    s_status[0] = '\0';

    if (!*path) {
        return;
    }

    CTgaFile tga;

    if (!tga.Open(path, 1) || !tga.LoadImageData(0)) {
        strcpy(s_status, "LUT not found or unreadable");
        return;
    }

    int32_t bytesPerPixel = tga.BytesPerPixel();

    if (tga.Width() != 256 || tga.Height() != 16 || (bytesPerPixel != 3 && bytesPerPixel != 4)) {
        strcpy(s_status, "LUT must be a 256x16 TGA");
        tga.Close();
        return;
    }

    uint8_t pixels[256 * 16 * 4];
    const uint8_t* source = tga.Image();
    bool topDown = tga.IsTopDown() != 0;

    for (int y = 0; y < 16; y++) {
        const uint8_t* row = source + (topDown ? y : 15 - y) * 256 * bytesPerPixel;

        for (int x = 0; x < 256; x++) {
            uint8_t* out = &pixels[(y * 256 + x) * 4];
            out[0] = row[x * bytesPerPixel + 0];
            out[1] = row[x * bytesPerPixel + 1];
            out[2] = row[x * bytesPerPixel + 2];
            out[3] = 0xFF;
        }
    }

    tga.Close();
    s_lut = CreateTextureFromMemory(device, 256, 16, D3DFMT_A8R8G8B8, 4, pixels, 256 * 4);

    if (!s_lut) {
        strcpy(s_status, "LUT upload failed");
    }
}

IDirect3DTexture9* GradeRun(IDirect3DTexture9* source) {
    const Settings& settings = GetSettings();
    IDirect3DDevice9* device = g_frame.device;

    if (!settings.colorGrade) {
        return source;
    }

    CVar* lutVar = FindCVar("colorGradeLut");
    const char* lutPath = lutVar ? lutVar->m_stringValue.m_str : "";

    if (!lutPath) {
        lutPath = "";
    }

    if (strcmp(lutPath, s_lutPath) != 0) {
        LoadLut(device, lutPath);
    }

    if (IsNeutral(settings) && !s_lut) {
        return source;
    }

    if (!s_graded.Ensure(device, g_frame.width, g_frame.height, D3DFMT_A8R8G8B8)) {
        return source;
    }

    TimerBegin(device, Section_Grade);

    SetTarget(device, s_graded, false, 0);
    SetSampler(device, 0, source, D3DTEXF_POINT, D3DTADDRESS_CLAMP);
    SetSampler(device, 1, s_lut, D3DTEXF_POINT, D3DTADDRESS_CLAMP);

    float temperature = settings.colorGradeTemperature;
    float grade[4] = { std::exp2(settings.colorGradeExposure), settings.colorGradeContrast, settings.colorGradeSaturation, settings.colorGradeVignette };
    float balance[4] = { 1.0f + 0.15f * temperature, 1.0f, 1.0f - 0.15f * temperature, s_lut ? 1.0f : 0.0f };
    SetPixelConstants(device, 0, grade, 1);
    SetPixelConstants(device, 1, balance, 1);

    device->SetPixelShader(PixelShader(Ps_ColorGrade));
    DrawFullscreen(device);
    device->SetTexture(0, nullptr);
    device->SetTexture(1, nullptr);

    TimerEnd(device, Section_Grade);

    return s_graded.tex;
}

const char* GradeStatus() {
    return s_status;
}

void GradeRelease() {
    s_graded.Release();

    if (s_lut) {
        s_lut->Release();
        s_lut = nullptr;
    }

    s_lutPath[0] = '\0';
    s_status[0] = '\0';
}

}
