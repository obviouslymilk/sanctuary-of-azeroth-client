#include <cstring>
#include <windowsx.h>
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "Engine/Source/GxModern/GxModernInternal.h"
#include "Engine/Source/ConsoleBase/ConsoleVar.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace GxModern {

static bool s_initialized;
static IDirect3DDevice9* s_device;

static bool Init(IDirect3DDevice9* device) {
    if (s_initialized) {
        return s_device == device;
    }

    HWND hwnd = WindowHandle();

    if (!hwnd) {
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_Init(hwnd) || !ImGui_ImplDX9_Init(device)) {
        ImGui::DestroyContext();
        return false;
    }

    s_initialized = true;
    s_device = device;
    return true;
}

void PanelShutdown() {
    if (!s_initialized) {
        return;
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    s_initialized = false;
    s_device = nullptr;
}

void PanelReleaseDeviceObjects() {
    if (s_initialized) {
        ImGui_ImplDX9_InvalidateDeviceObjects();
    }
}

bool PanelWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_KEYDOWN && wparam == VK_F12 && (GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_SHIFT) & 0x8000)) {
        SetCVar("graphicsPanel", GraphicsPanelVisible() ? "0" : "1");
        return true;
    }

    if (!s_initialized || !GraphicsPanelVisible()) {
        return false;
    }

    ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam);
    ImGuiIO& io = ImGui::GetIO();

    switch (message) {
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_MOUSEWHEEL:
        case WM_MOUSEHWHEEL:
            return io.WantCaptureMouse;

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_CHAR:
        case WM_SYSCHAR:
            return io.WantCaptureKeyboard;

        default:
            return false;
    }
}

static void CheckboxCVar(const char* label, const char* name) {
    CVar* var = FindCVar(name);

    if (!var) {
        return;
    }

    bool value = var->m_intValue != 0;

    if (ImGui::Checkbox(label, &value)) {
        SetCVar(name, value ? "1" : "0");
    }
}

static void SliderCVar(const char* label, const char* name, float min, float max) {
    CVar* var = FindCVar(name);

    if (!var) {
        return;
    }

    float value = var->m_floatValue;

    if (ImGui::SliderFloat(label, &value, min, max)) {
        SetCVar(name, value);
    }
}

static void ComboCVar(const char* label, const char* name, const char* const* items, int count) {
    CVar* var = FindCVar(name);

    if (!var) {
        return;
    }

    int value = var->m_intValue;
    value = value < 0 ? 0 : (value >= count ? count - 1 : value);

    if (ImGui::Combo(label, &value, items, count)) {
        SetCVar(name, static_cast<float>(value));
    }
}

static void DefaultsButton(const char* const* names, int count) {
    if (!ImGui::Button("Defaults")) {
        return;
    }

    for (int i = 0; i < count; i++) {
        CVar* var = FindCVar(names[i]);

        if (var) {
            var->Default();
        }
    }
}

static void TimeLine(Section section) {
    ImGui::SameLine();
    ImGui::TextDisabled("%.2f ms", GpuMs(section));
}

static void BuildPanel() {
    ImGui::SetNextWindowSize(ImVec2(420.0f, 560.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);

    bool open = true;

    if (!ImGui::Begin("Graphics", &open)) {
        ImGui::End();
        return;
    }

    if (!open) {
        SetCVar("graphicsPanel", "0");
    }

    CheckboxCVar("Modern rendering", "gxModernRender");

    if (!Supported()) {
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.3f, 1.0f), "Unavailable: %s", UnsupportedReason());
    } else if (FindCVar("gxModernRender") && FindCVar("gxModernRender")->m_intValue) {
        ImGui::TextDisabled("Multisampling returns after gxRestart when this is turned off.");
    } else {
        ImGui::TextDisabled("Off. The device is created without multisampling when this is on at startup.");
    }

    const char* const levels[] = { "Off", "Half resolution", "Full resolution" };
    const char* const fogLevels[] = { "Off", "16 steps", "32 steps" };

    if (ImGui::CollapsingHeader("SMAA", ImGuiTreeNodeFlags_DefaultOpen)) {
        CheckboxCVar("Enabled##smaa", "smaa");
        TimeLine(Section_Smaa);
    }

    if (ImGui::CollapsingHeader("Ambient occlusion", ImGuiTreeNodeFlags_DefaultOpen)) {
        ComboCVar("Mode##ssao", "ssao", levels, 3);
        TimeLine(Section_Ssao);
        SliderCVar("Strength##ssao", "ssaoStrength", 0.0f, 1.0f);
        SliderCVar("Radius (yards)##ssao", "ssaoRadius", 0.1f, 5.0f);
        static const char* const names[] = { "ssao", "ssaoStrength", "ssaoRadius" };
        DefaultsButton(names, 3);
    }

    if (ImGui::CollapsingHeader("Volumetric fog", ImGuiTreeNodeFlags_DefaultOpen)) {
        ComboCVar("Quality##fog", "volumeFog", fogLevels, 3);
        TimeLine(Section_Fog);
        SliderCVar("Density", "volumeFogDensity", 0.0f, 0.2f);
        SliderCVar("Height falloff", "volumeFogHeightFalloff", 0.0f, 1.0f);
        SliderCVar("Height offset", "volumeFogHeightOffset", -50.0f, 50.0f);
        SliderCVar("Noise", "volumeFogNoise", 0.0f, 1.0f);
        SliderCVar("Wind", "volumeFogWind", 0.0f, 10.0f);
        SliderCVar("Sun glow", "volumeFogSunGlow", 0.0f, 4.0f);
        SliderCVar("Anisotropy", "volumeFogAnisotropy", 0.0f, 0.95f);
        CheckboxCVar("God rays", "godRays");
        SliderCVar("God ray strength", "godRaysStrength", 0.0f, 1.0f);

        CVar* shadows = FindCVar("extShadowQuality");

        if (shadows && shadows->m_intValue < 3) {
            ImGui::TextDisabled("God rays need extShadowQuality 3 or higher.");
        }

        static const char* const names[] = { "volumeFog", "volumeFogDensity", "volumeFogHeightFalloff", "volumeFogHeightOffset", "volumeFogNoise", "volumeFogWind", "volumeFogSunGlow", "volumeFogAnisotropy", "godRays", "godRaysStrength" };
        DefaultsButton(names, 10);
    }

    if (ImGui::CollapsingHeader("Color grading", ImGuiTreeNodeFlags_DefaultOpen)) {
        CheckboxCVar("Enabled##grade", "colorGrade");
        TimeLine(Section_Grade);
        SliderCVar("Exposure (EV)", "colorGradeExposure", -3.0f, 3.0f);
        SliderCVar("Contrast", "colorGradeContrast", 0.5f, 1.5f);
        SliderCVar("Saturation", "colorGradeSaturation", 0.0f, 2.0f);
        SliderCVar("Temperature", "colorGradeTemperature", -1.0f, 1.0f);
        SliderCVar("Vignette", "colorGradeVignette", 0.0f, 1.0f);

        CVar* lut = FindCVar("colorGradeLut");
        char path[260] = "";

        if (lut && lut->m_stringValue.m_str) {
            strncpy(path, lut->m_stringValue.m_str, sizeof(path) - 1);
        }

        if (ImGui::InputText("LUT (.tga)", path, sizeof(path), ImGuiInputTextFlags_EnterReturnsTrue)) {
            SetCVar("colorGradeLut", path);
        }

        if (GradeStatus()[0]) {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.3f, 1.0f), "%s", GradeStatus());
        }

        static const char* const names[] = { "colorGrade", "colorGradeExposure", "colorGradeContrast", "colorGradeSaturation", "colorGradeTemperature", "colorGradeVignette", "colorGradeLut" };
        DefaultsButton(names, 7);
    }

    if (ImGui::CollapsingHeader("Shadows (extShadowQuality 5)")) {
        static const char* const sizes[] = { "2048", "4096" };
        CVar* size = FindCVar("shadowMapSize");

        if (size) {
            int index = size->m_intValue >= 4096 ? 1 : 0;

            if (ImGui::Combo("Map size", &index, sizes, 2)) {
                SetCVar("shadowMapSize", sizes[index]);
            }
        }

        SliderCVar("Near band radius", "shadowBand0Radius", 10.0f, 200.0f);
        SliderCVar("Middle band radius", "shadowBand1Radius", 40.0f, 600.0f);
        SliderCVar("Far band radius", "shadowBand2Radius", 100.0f, 1500.0f);
        static const char* const names[] = { "shadowMapSize", "shadowBand0Radius", "shadowBand1Radius", "shadowBand2Radius" };
        DefaultsButton(names, 4);
    }

    ImGui::End();
}

void PanelRender() {
    if (!GraphicsPanelVisible()) {
        return;
    }

    IDirect3DDevice9* device = Device();

    if (!device || !Init(device)) {
        return;
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::GetIO().MouseDrawCursor = true;

    BuildPanel();

    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

}
