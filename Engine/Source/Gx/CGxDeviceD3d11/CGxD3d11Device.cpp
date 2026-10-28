#include <cstdint>
#include <cstring>
#include <new>
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include "Engine/Source/Gx/CGxDeviceD3d11/CGxDeviceD3d11.h"
#include "Engine/Source/Gx/CGxBuf.h"
#include "Engine/Source/Gx/CGxPool.h"
#include "Engine/Source/Gx/Gx.h"
#include "Engine/Source/Os/Error.h"
#include "Engine/Source/Os/TimeManager.h"
#include "Engine/Source/Storm/H/storm.h"
#include "GameVersion.h"

// OFFSET-15595: 0x00BD530C/0x1409CA468
const DXGI_FORMAT CGxDeviceD3d11::s_GxFormatToDxgiFormat[CGxFormat::Formats_Last] = { DXGI_FORMAT_B5G6R5_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_FORMAT_D16_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_D32_FLOAT };

// OFFSET-15595: 0x00BD532C/0x1409CA488
const DXGI_FORMAT CGxDeviceD3d11::s_GxFormatToDxgiTypeless[CGxFormat::Formats_Last] = { DXGI_FORMAT_B5G6R5_UNORM, DXGI_FORMAT_B8G8R8A8_TYPELESS, DXGI_FORMAT_B8G8R8A8_TYPELESS, DXGI_FORMAT_R10G10B10A2_TYPELESS, DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_R32_TYPELESS };

static bool s_hideCursorOnCreate = true;

static ATOM WindowClassCreate() {
    HINSTANCE instance = GetModuleHandleA(nullptr);

    WNDCLASSEXA windowClass;
    memset(&windowClass, 0, sizeof(windowClass));
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_OWNDC;
    windowClass.lpfnWndProc = CGxDeviceD3d11::WindowProcD3d11;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "GxWindowClassD3d11";
    windowClass.hIcon = static_cast<HICON>(LoadImageA(instance, "BlizzardIcon.ico", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
    windowClass.hCursor = LoadCursorA(instance, "BlizzardCursor.cur");

    if (!windowClass.hCursor) {
        windowClass.hCursor = LoadCursorA(instance, IDC_ARROW);
    }

    return RegisterClassExA(&windowClass);
}

CGxDevice* CreateCGxDeviceD3d11() {
    void* mem = SMemAlloc(sizeof(CGxDeviceD3d11), __FILE__, __LINE__, 0);

    if (!mem) {
        return nullptr;
    }

    return new (mem) CGxDeviceD3d11();
}

CGxDeviceD3d11::CGxDeviceD3d11() : m_deviceConstants() {
    this->m_api = GxApi_D3d11;
    this->m_hwnd = nullptr;
    this->m_hwndClass = 0;
    this->m_ownhwnd = 0;
    this->m_cursorHandle = nullptr;
    this->m_inModeSwitch = false;
    this->m_d3dDevice = nullptr;
    this->m_d3dContext = nullptr;
    this->m_dxgiFactory = nullptr;
    this->m_dxgiAdapter = nullptr;
    this->m_swapChain = nullptr;
    this->m_backBufferFormat = DXGI_FORMAT_UNKNOWN;
    this->m_sampleCount = 1;
    this->m_backBufferWidth = 0;
    this->m_backBufferHeight = 0;
    this->m_backBufferRtv = nullptr;
    this->m_depthTexture = nullptr;
    this->m_depthDsv = nullptr;
    this->m_scratchDepthTexture = nullptr;
    this->m_scratchDepthDsv = nullptr;
    this->m_scratchDepthWidth = 0;
    this->m_scratchDepthHeight = 0;
    this->m_curRtv = nullptr;
    this->m_curDsv = nullptr;
    this->m_savedDsv = nullptr;
    this->m_vsConstants = nullptr;
    this->m_psConstants = nullptr;
    this->m_deviceConstantsBuffer = nullptr;
    this->m_emuConstantsBuffer = nullptr;
    this->m_clearConstants = nullptr;
    this->m_clearRasterizer = nullptr;
    memset(this->m_clearShaders, 0, sizeof(this->m_clearShaders));
    memset(this->m_clearBlend, 0, sizeof(this->m_clearBlend));
    memset(this->m_clearDepth, 0, sizeof(this->m_clearDepth));
    memset(this->m_shadowSamplers, 0, sizeof(this->m_shadowSamplers));
    this->m_compilerModule = nullptr;
    this->m_d3dCompile = nullptr;
    this->m_curVertexShader = nullptr;
    this->m_curPixelShader = nullptr;
    this->m_curVsCode = nullptr;
    this->m_curVsCodeSize = 0;
    this->m_fanIndexBuffer = nullptr;
    this->m_fanIndexCount = 0;
    this->m_caps.m_colorFormat = GxCF_argb;
    this->m_caps.unk_0018 = 1;
    memset(this->m_samplerStates, 0, sizeof(this->m_samplerStates));
    memset(this->m_vsMirror, 0, sizeof(this->m_vsMirror));
    memset(this->m_psMirror, 0, sizeof(this->m_psMirror));
    this->IStateReset();
    this->DeviceCreatePools();
    this->DeviceCreateStreamBufs();
}

CGxDeviceD3d11::~CGxDeviceD3d11() {
}

void CGxDeviceD3d11::IStateReset() {
    this->m_stateDirty = StateDirty_Blend | StateDirty_Depth | StateDirty_Rasterizer | StateDirty_InputLayout | StateDirty_DeviceConstants;
    this->m_curInputLayout = nullptr;
    this->m_curIndexBuffer = nullptr;
    this->m_curVertexBufferCount = 0;
    this->m_curRasterizerState = nullptr;
    this->m_curBlendState = nullptr;
    this->m_curDepthState = nullptr;
    this->m_curTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    this->m_curVertexShader = nullptr;
    this->m_curPixelShader = nullptr;
    this->m_curVsCode = nullptr;
    this->m_curVsCodeSize = 0;
    memset(this->m_curVertexBuffers, 0, sizeof(this->m_curVertexBuffers));
    memset(this->m_curVertexOffsets, 0, sizeof(this->m_curVertexOffsets));
    memset(this->m_curVertexStrides, 0, sizeof(this->m_curVertexStrides));
    memset(this->m_boundSrv, 0, sizeof(this->m_boundSrv));
    memset(this->m_boundSampler, 0, sizeof(this->m_boundSampler));
    this->m_scissorRectDirty = 1;
    this->m_viewportDirty = 1;
}

void CGxDeviceD3d11::DeviceUnk81(uint32_t a, uint32_t b) {
}

void CGxDeviceD3d11::DeviceUnk82() {
}

void CGxDeviceD3d11::DeviceUnk83(uint32_t a, uint32_t b) {
}

void CGxDeviceD3d11::StereoSetConvergence(float convergence) {
}

float CGxDeviceD3d11::StereoGetConvergence() {
    return 1.0f;
}

void CGxDeviceD3d11::StereoSetSeparation(float separation) {
}

float CGxDeviceD3d11::StereoGetSeparation() {
    return 0.0f;
}

bool CGxDeviceD3d11::StereoEnabled() {
    return false;
}

void* CGxDeviceD3d11::DeviceWindow() {
    return this->m_hwnd;
}

void CGxDeviceD3d11::DeviceOverride(EGxOverride override, uint32_t value) {
    CGxDevice::DeviceOverride(override, value);

    if (override == GxOverride_Unk7) {
        this->m_caps.m_texNonPow2 = value != 0;
    }
}

void CGxDeviceD3d11::DeviceEvictResources() {
}

void CGxDeviceD3d11::DeviceSetBaseMipLevel(uint32_t level) {
    this->ITexForceRecreation(1);
    CGxDevice::DeviceSetBaseMipLevel(level);
}

void CGxDeviceD3d11::CapsWindowSize(CRect& rect) {
    rect = this->DeviceCurWindow();
}

void CGxDeviceD3d11::CapsWindowSizeInScreenCoords(CRect& rect) {
    if (this->IDevIsWindowed()) {
        const CRect& window = this->DeviceCurWindow();
        POINT points[2];
        points[0].x = 0;
        points[0].y = 0;
        points[1].x = static_cast<LONG>(window.maxX);
        points[1].y = static_cast<LONG>(window.maxY);
        MapWindowPoints(this->m_hwnd, nullptr, points, 2);
        rect.minY = static_cast<float>(points[0].y);
        rect.minX = static_cast<float>(points[0].x);
        rect.maxY = static_cast<float>(points[1].y);
        rect.maxX = static_cast<float>(points[1].x);
    } else {
        rect = this->DeviceCurWindow();
    }
}

void CGxDeviceD3d11::LogCrashInfo(char* buffer, uint32_t size) {
    IDXGIFactory1* factory = nullptr;

    if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&factory)))) {
        return;
    }

    char line[260];
    IDXGIAdapter1* adapter = nullptr;

    for (uint32_t i = 0; SUCCEEDED(factory->EnumAdapters1(i, &adapter)); i++) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        char name[128];
        WideCharToMultiByte(CP_ACP, 0, desc.Description, -1, name, sizeof(name), nullptr, nullptr);

        SStrPrintf(line, sizeof(line), "\r\nAdapter %d:\r\n  Description: %s\r\n  VendorId: %04X DeviceId: %04X\r\n", i, name, desc.VendorId, desc.DeviceId);
        SStrPack(buffer, line, size);
        adapter->Release();
    }

    factory->Release();
}

LRESULT CALLBACK CGxDeviceD3d11::WindowProcD3d11(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    CGxDeviceD3d11* device = reinterpret_cast<CGxDeviceD3d11*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));

    switch (message) {
        case WM_CREATE:
            SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCTA*>(lparam)->lpCreateParams));
            return 0;

        case WM_DESTROY:
            if (device) {
                device->DeviceWM(GxWM_Destroy, 0, 0);
            }

            return 0;

        case WM_SIZE: {
            if (!device) {
                break;
            }

            CRect rect;
            rect.minY = 0.0f;
            rect.minX = 0.0f;
            rect.maxY = HIWORD(lparam);
            rect.maxX = LOWORD(lparam);

            uint32_t resizeType = 0;

            if (wparam == SIZE_MINIMIZED) {
                resizeType = Gx_SizeMinimized;
            } else if (wparam == SIZE_MAXHIDE) {
                resizeType = Gx_SizeMaxHide;
            }

            device->DeviceWM(GxWM_Size, reinterpret_cast<uintptr_t>(&rect), resizeType);
            break;
        }

        case WM_ACTIVATE:
            if (!device) {
                break;
            }

            if (LOWORD(wparam) == WA_INACTIVE) {
                if (!device->IDevIsWindowed()) {
                    // DXGI wants the swap chain out of fullscreen before the window is minimized
                    device->ISetWindowFocus(0);

                    CRect rect;
                    rect.minY = 0.0f;
                    rect.minX = 0.0f;
                    rect.maxY = 0.0f;
                    rect.maxX = 0.0f;
                    device->DeviceWM(GxWM_Size, reinterpret_cast<uintptr_t>(&rect), Gx_SizeMinimized);
                }
            } else if (LOWORD(wparam) == WA_ACTIVE && !device->IDevIsWindowed()) {
                device->ISetWindowFocus(1);
            }

            break;

        case WM_SETFOCUS:
            if (device) {
                device->DeviceWM(GxWM_SetFocus, 0, 0);
            }

            return 0;

        case WM_KILLFOCUS:
            if (device) {
                device->DeviceWM(GxWM_KillFocus, 0, 0);
            }

            return 0;

        case WM_PAINT: {
            PAINTSTRUCT paint;
            BeginPaint(hwnd, &paint);
            EndPaint(hwnd, &paint);
            return 0;
        }

        case WM_ERASEBKGND:
            return 0;

        case WM_SETCURSOR:
            if (!device) {
                return DefWindowProcA(hwnd, message, wparam, lparam);
            }

            if (device->m_d3dDevice && LOWORD(lparam) == HTCLIENT) {
                SetCursor(device->m_cursorVisible && device->m_hwCursor ? device->m_cursorHandle : nullptr);
                return 1;
            }

            break;

        case WM_DISPLAYCHANGE:
            if (device && !device->IDevIsWindowed()) {
                CRect rect;
                rect.minY = 0.0f;
                rect.minX = 0.0f;
                rect.maxY = HIWORD(lparam);
                rect.maxX = LOWORD(lparam);
                device->DeviceWM(GxWM_DisplayChange, reinterpret_cast<uintptr_t>(&rect), 0);
            }

            break;

        case WM_SYSCOMMAND:
            if (wparam == SC_SCREENSAVE || wparam == SC_MONITORPOWER) {
                return 0;
            }

            break;

        case WM_SIZING: {
            RECT* sizingRect = reinterpret_cast<RECT*>(lparam);

            CiRect rect;
            rect.minX = sizingRect->left;
            rect.minY = sizingRect->top;
            rect.maxX = sizingRect->right;
            rect.maxY = sizingRect->bottom;

            if (rect.maxX - rect.minX <= 0 || rect.maxY - rect.minY <= 0) {
                return 0;
            }

            switch (wparam) {
                case WMSZ_LEFT:
                    CGxDevice::ClampRectLeft(rect);
                    CGxDevice::ClampRectTopByAspect(rect);
                    break;

                case WMSZ_RIGHT:
                    CGxDevice::ClampRectRight(rect);
                    CGxDevice::ClampRectBottomByAspect(rect);
                    break;

                case WMSZ_TOP:
                    CGxDevice::ClampRectTop(rect);
                    CGxDevice::ClampRectLeftByAspect(rect);
                    break;

                case WMSZ_TOPLEFT:
                    CGxDevice::ClampRectTop(rect);
                    CGxDevice::ClampRectLeft(rect);
                    CGxDevice::ClampRectTopByAspect(rect);
                    CGxDevice::ClampRectLeftByAspect(rect);
                    break;

                case WMSZ_TOPRIGHT:
                    CGxDevice::ClampRectTop(rect);
                    CGxDevice::ClampRectRight(rect);
                    CGxDevice::ClampRectTopByAspect(rect);
                    CGxDevice::ClampRectRightByAspect(rect);
                    break;

                case WMSZ_BOTTOM:
                    CGxDevice::ClampRectBottom(rect);
                    CGxDevice::ClampRectRightByAspect(rect);
                    break;

                case WMSZ_BOTTOMLEFT:
                    CGxDevice::ClampRectBottom(rect);
                    CGxDevice::ClampRectLeft(rect);
                    CGxDevice::ClampRectLeftByAspect(rect);
                    CGxDevice::ClampRectBottomByAspect(rect);
                    break;

                case WMSZ_BOTTOMRIGHT:
                    CGxDevice::ClampRectBottom(rect);
                    CGxDevice::ClampRectRight(rect);
                    CGxDevice::ClampRectBottomByAspect(rect);
                    CGxDevice::ClampRectRightByAspect(rect);
                    break;

                default:
                    break;
            }

            sizingRect->left = rect.minX;
            sizingRect->bottom = rect.maxY;
            sizingRect->top = rect.minY;
            sizingRect->right = rect.maxX;

            return 1;
        }

        default:
            break;
    }

    if (device && device->m_windowProc) {
        return device->m_windowProc(hwnd, message, wparam, lparam);
    }

    return DefWindowProcA(hwnd, message, wparam, lparam);
}

int32_t CGxDeviceD3d11::ICreateWindow(CGxFormat& format) {
    HINSTANCE instance = GetModuleHandleA(nullptr);
    DWORD style;

    if (!format.m_window) {
        style = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU;
    } else if (format.m_maximize == 1) {
        style = WS_POPUP | WS_VISIBLE;
    } else if (format.m_maximize == 2) {
        style = WS_POPUP;
    } else {
        style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    }

    float aspectRatio = 0.0f;

    if (format.m_aspect && format.m_size.x && format.m_size.y) {
        aspectRatio = static_cast<float>(format.m_size.x) / format.m_size.y;
    }

    RECT clientRect;
    clientRect.right = format.m_size.x;
    clientRect.left = 0;
    clientRect.top = 0;
    clientRect.bottom = format.m_size.y;
    AdjustWindowRectEx(&clientRect, style, FALSE, 0);

    CGxDevice::s_clientAdjust.x = clientRect.right - format.m_size.x - clientRect.left;
    CGxDevice::s_clientAdjust.y = clientRect.bottom - format.m_size.y - clientRect.top;

    if (format.m_window) {
        if (format.m_maximize == 1) {
            format.m_pos.x = 0;
            format.m_pos.y = 0;
            format.m_size.x = GetSystemMetrics(SM_CXSCREEN);
            format.m_size.y = GetSystemMetrics(SM_CYSCREEN);
        } else {
            RECT windowRect;
            windowRect.left = 0;
            windowRect.top = 0;
            windowRect.right = format.m_size.x;
            windowRect.bottom = format.m_size.y;

            if (CGxDevice::AdjustWindowRectToWorkArea(aspectRatio, &CGxDevice::s_clientAdjust, &windowRect)) {
                format.m_pos.x = windowRect.left;
                format.m_pos.y = windowRect.top;
                format.m_size.x = windowRect.right - windowRect.left;
                format.m_size.y = windowRect.bottom - windowRect.top;
            } else {
                format.m_pos.x = 0;
                format.m_pos.y = 0;
            }
        }
    }

    int32_t width = format.m_size.x;
    int32_t height = format.m_size.y;

    if (!width) {
        width = CW_USEDEFAULT;
    }

    if (!height) {
        height = CW_USEDEFAULT;
    }

    if (format.m_window && format.m_maximize != 1 && format.m_size.x && format.m_size.y) {
        width += CGxDevice::s_clientAdjust.x;
        height += CGxDevice::s_clientAdjust.y;
    }

    this->m_hwnd = CreateWindowExA(WS_EX_APPWINDOW, "GxWindowClassD3d11", CLIENT_NAME, style, format.m_pos.x, format.m_pos.y, width, height, nullptr, nullptr, instance, this);

    if (this->m_hwnd && format.m_maximize != 2) {
        ShowWindow(this->m_hwnd, SW_SHOWNORMAL);
    }

    return this->m_hwnd != nullptr;
}

void CGxDeviceD3d11::ISetCaps(const CGxFormat& format) {
    CGxCaps& caps = this->m_caps;

    caps.m_numTmus = 8;
    // D3D11 has no half pixel offset, so the UI must not add the D3D9 one
    caps.m_pixelCenterOnEdge = 1;
    caps.m_texelCenterOnEdge = 1;
    caps.m_numStreams = 16;
    caps.m_streamOffset = 1;
    caps.m_maxIndex = 0xFFFFF;
    caps.m_generateMipMaps = 0;
    caps.m_texTarget[GxTex_2d] = 1;
    caps.m_texTarget[GxTex_CubeMap] = 1;
    caps.m_texTarget[GxTex_Rectangle] = 0;
    caps.m_texTarget[GxTex_NonPow2] = 1;
    caps.m_texNonPow2 = 1;

    for (uint32_t i = 0; i < GxTexTargets_Last; i++) {
        caps.m_texMaxSize[i] = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    }

    caps.m_texFilterTrilinear = 1;
    caps.m_texFilterAnisotropic = 1;
    caps.m_maxTexAnisotropy = 16;
    caps.m_depthBias = 1;
    caps.m_colorWrite = 1;
    caps.m_maxClipPlanes = 6;
    caps.m_hwCursor = 1;
    caps.m_occlusionQuery = 1;
    caps.m_pointParameters = 0;
    caps.m_pointScaleMax = 0.0f;
    caps.m_pointSprite = 0;
    caps.m_blendFactor = 1;
    caps.m_shaderAlphaRef = 1;
    caps.m_stereoAvailable = 0;
    // world shaders blend the fog themselves (fog color in ps c2), like ps_3_0 on D3D9
    caps.unk_0134 = 0;
    caps.unk_0138 = 0;

    // 5 is vs_5_0 and 6 is ps_5_0 in the target name tables
    for (uint32_t i = 0; i < GxShTargets_Last; i++) {
        caps.m_shaderTargets[i] = 0;
        caps.m_shaderConstants[i] = 0;
    }

    caps.m_shaderTargets[GxSh_Vertex] = 5;
    caps.m_shaderTargets[GxSh_Pixel] = 6;
    caps.m_shaderConstants[GxSh_Vertex] = 256;
    caps.m_shaderConstants[GxSh_Pixel] = 256;

    for (uint32_t i = 1; i < GxTexFormats_Last; i++) {
        UINT support = 0;
        caps.m_texFmt[i] = 0;
        caps.m_rttFormat[i] = 0;

        if (SUCCEEDED(this->m_d3dDevice->CheckFormatSupport(s_GxTexFormatToSrvFormat[i], &support))) {
            caps.m_texFmt[i] = (support & D3D11_FORMAT_SUPPORT_TEXTURE2D) && (support & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE);
        }

        if (i == GxTex_D24X8) {
            if (SUCCEEDED(this->m_d3dDevice->CheckFormatSupport(DXGI_FORMAT_D24_UNORM_S8_UINT, &support))) {
                caps.m_rttFormat[i] = (support & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL) != 0;
            }
        } else if (SUCCEEDED(this->m_d3dDevice->CheckFormatSupport(s_GxTexFormatToDxgiFormat[i], &support))) {
            caps.m_rttFormat[i] = (support & D3D11_FORMAT_SUPPORT_RENDER_TARGET) != 0;
        }
    }

    caps.m_texFmt[GxTex_Unknown] = 0;
    caps.m_rttFormat[GxTex_Unknown] = 0;
}

int32_t CGxDeviceD3d11::ICreateSwapChain(const CGxFormat& format) {
    this->m_backBufferFormat = s_GxFormatToDxgiFormat[format.m_colorFormat];

    DXGI_SWAP_CHAIN_DESC desc;
    memset(&desc, 0, sizeof(desc));

    uint32_t sampleCount = 1;
    uint32_t sampleQuality = 0;

    if (format.m_multisampleCount > 1) {
        UINT colorQuality = 0;
        UINT depthQuality = 0;
        DXGI_FORMAT depthFormat = s_GxFormatToDxgiFormat[format.m_depthFormat];

        if (SUCCEEDED(this->m_d3dDevice->CheckMultisampleQualityLevels(this->m_backBufferFormat, format.m_multisampleCount, &colorQuality)) && SUCCEEDED(this->m_d3dDevice->CheckMultisampleQualityLevels(depthFormat, format.m_multisampleCount, &depthQuality)) && colorQuality && depthQuality) {
            UINT quality = colorQuality < depthQuality ? colorQuality : depthQuality;
            sampleCount = format.m_multisampleCount;
            sampleQuality = static_cast<uint32_t>((quality - 1) * format.m_multisampleQuality);
        }
    }

    this->m_sampleCount = sampleCount;

    if (format.m_window) {
        RECT client;
        GetClientRect(this->m_hwnd, &client);
        desc.BufferDesc.Width = client.right > 0 ? client.right : format.m_size.x;
        desc.BufferDesc.Height = client.bottom > 0 ? client.bottom : format.m_size.y;
    } else {
        desc.BufferDesc.Width = format.m_size.x;
        desc.BufferDesc.Height = format.m_size.y;
        desc.BufferDesc.RefreshRate.Numerator = format.m_refreshRate;
        desc.BufferDesc.RefreshRate.Denominator = format.m_refreshRate ? 1 : 0;
    }

    desc.BufferDesc.Format = this->m_backBufferFormat;
    desc.SampleDesc.Count = sampleCount;
    desc.SampleDesc.Quality = sampleQuality;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 1;
    desc.OutputWindow = this->m_hwnd;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    desc.Windowed = format.m_window != 0;

    if (!format.m_window) {
        desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    }

    HRESULT result = this->m_dxgiFactory->CreateSwapChain(this->m_d3dDevice, &desc, &this->m_swapChain);

    if (FAILED(result)) {
        CGxDevice::Log("CGxDeviceD3d11::ICreateSwapChain(): CreateSwapChain failed: 0x%08x", result);
        return 0;
    }

    return 1;
}

int32_t CGxDeviceD3d11::ICreateTargets() {
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    this->m_swapChain->GetDesc(&swapChainDesc);
    this->m_backBufferWidth = swapChainDesc.BufferDesc.Width;
    this->m_backBufferHeight = swapChainDesc.BufferDesc.Height;

    ID3D11Texture2D* backBuffer = nullptr;

    if (FAILED(this->m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)))) {
        return 0;
    }

    HRESULT result = this->m_d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &this->m_backBufferRtv);
    backBuffer->Release();

    if (FAILED(result)) {
        return 0;
    }

    D3D11_TEXTURE2D_DESC depthDesc;
    memset(&depthDesc, 0, sizeof(depthDesc));
    depthDesc.Width = this->m_backBufferWidth;
    depthDesc.Height = this->m_backBufferHeight;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = s_GxFormatToDxgiTypeless[this->m_format.m_depthFormat];
    depthDesc.SampleDesc = swapChainDesc.SampleDesc;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    if (FAILED(this->m_d3dDevice->CreateTexture2D(&depthDesc, nullptr, &this->m_depthTexture))) {
        return 0;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;
    memset(&viewDesc, 0, sizeof(viewDesc));
    viewDesc.Format = s_GxFormatToDxgiFormat[this->m_format.m_depthFormat];
    viewDesc.ViewDimension = swapChainDesc.SampleDesc.Count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;

    if (FAILED(this->m_d3dDevice->CreateDepthStencilView(this->m_depthTexture, &viewDesc, &this->m_depthDsv))) {
        return 0;
    }

    this->m_curRtv = this->m_backBufferRtv;
    this->m_curDsv = this->m_depthDsv;
    memset(this->m_textureTarget, 0, sizeof(this->m_textureTarget));
    this->ISetRenderTargets();

    return 1;
}

void CGxDeviceD3d11::IReleaseTargets() {
    if (this->m_d3dContext) {
        this->m_d3dContext->OMSetRenderTargets(0, nullptr, nullptr);
    }

    for (uint32_t i = 0; i < 2; i++) {
        if (this->m_textureTarget[i].m_apiSpecific) {
            static_cast<IUnknown*>(this->m_textureTarget[i].m_apiSpecific)->Release();
            this->m_textureTarget[i].m_apiSpecific = nullptr;
        }
    }

    this->m_curRtv = nullptr;
    this->m_curDsv = nullptr;
    this->m_savedDsv = nullptr;

    if (this->m_backBufferRtv) {
        this->m_backBufferRtv->Release();
        this->m_backBufferRtv = nullptr;
    }

    if (this->m_depthDsv) {
        this->m_depthDsv->Release();
        this->m_depthDsv = nullptr;
    }

    if (this->m_depthTexture) {
        this->m_depthTexture->Release();
        this->m_depthTexture = nullptr;
    }

    if (this->m_scratchDepthDsv) {
        this->m_scratchDepthDsv->Release();
        this->m_scratchDepthDsv = nullptr;
    }

    if (this->m_scratchDepthTexture) {
        this->m_scratchDepthTexture->Release();
        this->m_scratchDepthTexture = nullptr;
    }

    this->m_scratchDepthWidth = 0;
    this->m_scratchDepthHeight = 0;
}

void CGxDeviceD3d11::IUnbindTextures() {
    ID3D11ShaderResourceView* nullViews[16] = {};

    if (this->m_d3dContext) {
        this->m_d3dContext->PSSetShaderResources(0, 16, nullViews);
    }

    memset(this->m_boundSrv, 0, sizeof(this->m_boundSrv));
}

void CGxDeviceD3d11::ISetRenderTargets() {
    // The runtime silently unbinds shader resources that alias a render target, which the binding cache cannot see.
    this->IUnbindTextures();
    this->m_d3dContext->OMSetRenderTargets(1, &this->m_curRtv, this->m_curDsv);
}

int32_t CGxDeviceD3d11::ICreateD3dDevice(const CGxFormat& format) {
    HRESULT result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&this->m_dxgiFactory));

    if (FAILED(result)) {
        CGxDevice::Log("CGxDeviceD3d11::ICreateD3dDevice(): CreateDXGIFactory1 failed: 0x%08x", result);
        return 0;
    }

    result = this->m_dxgiFactory->EnumAdapters1(0, &this->m_dxgiAdapter);

    if (FAILED(result)) {
        CGxDevice::Log("CGxDeviceD3d11::ICreateD3dDevice(): EnumAdapters1 failed: 0x%08x", result);
        return 0;
    }

    UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_FEATURE_LEVEL featureLevels[1] = { D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL featureLevel;

    result = D3D11CreateDevice(this->m_dxgiAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, flags, featureLevels, 1, D3D11_SDK_VERSION, &this->m_d3dDevice, &featureLevel, &this->m_d3dContext);

#ifdef _DEBUG
    if (result == DXGI_ERROR_SDK_COMPONENT_MISSING) {
        flags &= ~D3D11_CREATE_DEVICE_DEBUG;
        result = D3D11CreateDevice(this->m_dxgiAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, flags, featureLevels, 1, D3D11_SDK_VERSION, &this->m_d3dDevice, &featureLevel, &this->m_d3dContext);
    }
#endif

    if (FAILED(result)) {
        CGxDevice::Log("CGxDeviceD3d11::ICreateD3dDevice(): D3D11CreateDevice with feature level 11_0 failed: 0x%08x", result);
        return 0;
    }

    if (!this->ICreateSwapChain(format)) {
        return 0;
    }

    if (format.m_fixLag) {
        IDXGIDevice1* dxgiDevice = nullptr;

        if (SUCCEEDED(this->m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice1), reinterpret_cast<void**>(&dxgiDevice)))) {
            dxgiDevice->SetMaximumFrameLatency(1);
            dxgiDevice->Release();
        }
    }

    this->m_dxgiFactory->MakeWindowAssociation(this->m_hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);

    this->m_context = 1;
    this->m_format.m_depthFormat = format.m_depthFormat;
    this->m_format.m_colorFormat = format.m_colorFormat;
    this->ISetCaps(format);
    this->Log(this->m_caps);

    if (!this->ICreateTargets()) {
        CGxDevice::Log("CGxDeviceD3d11::ICreateD3dDevice(): unable to create render targets");
        return 0;
    }

    D3D11_BUFFER_DESC bufferDesc;
    memset(&bufferDesc, 0, sizeof(bufferDesc));
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.ByteWidth = 16 * 256;

    if (FAILED(this->m_d3dDevice->CreateBuffer(&bufferDesc, nullptr, &this->m_vsConstants)) || FAILED(this->m_d3dDevice->CreateBuffer(&bufferDesc, nullptr, &this->m_psConstants))) {
        return 0;
    }

    bufferDesc.ByteWidth = (sizeof(DeviceConstants) + 15) & ~15u;

    if (FAILED(this->m_d3dDevice->CreateBuffer(&bufferDesc, nullptr, &this->m_deviceConstantsBuffer))) {
        return 0;
    }

    bufferDesc.ByteWidth = (sizeof(EmuConstants) + 15) & ~15u;

    if (FAILED(this->m_d3dDevice->CreateBuffer(&bufferDesc, nullptr, &this->m_emuConstantsBuffer))) {
        return 0;
    }

    memset(&this->m_emuConstants, 0xCD, sizeof(this->m_emuConstants));

    // garbage on purpose, so the first sync always uploads
    memset(&this->m_deviceConstants, 0xCD, sizeof(this->m_deviceConstants));
    this->m_d3dContext->VSSetConstantBuffers(0, 1, &this->m_vsConstants);
    this->m_d3dContext->PSSetConstantBuffers(0, 1, &this->m_psConstants);
    this->m_d3dContext->VSSetConstantBuffers(1, 1, &this->m_deviceConstantsBuffer);
    this->m_d3dContext->PSSetConstantBuffers(1, 1, &this->m_deviceConstantsBuffer);
    this->m_d3dContext->VSSetConstantBuffers(2, 1, &this->m_emuConstantsBuffer);

    this->IStateReset();
    this->IRsForceUpdate();
    this->IRsSync(0);
    this->InitLights();
    this->ShaderConstantsClear();

    if (!this->m_format.m_window) {
        this->DeviceSetGamma(this->m_gammaRamp);
    }

    this->ICursorCreate(format);

    if (s_hideCursorOnCreate) {
        this->CursorSetVisible(0);
        s_hideCursorOnCreate = false;
    }

    g_theGxDevicePtr->ShaderCreate(CGxDevice::s_uiVertexShader, GxSh_Vertex, "Shaders\\Vertex", "UI", 2);
    g_theGxDevicePtr->ShaderCreate(&CGxDevice::s_uiPixelShader, GxSh_Pixel, "Shaders\\Pixel", "UI", 1);

    return 1;
}

void CGxDeviceD3d11::IDestroyD3d() {
    this->m_context = 0;

    if (this->m_d3dContext) {
        this->m_d3dContext->ClearState();
        this->m_d3dContext->Flush();
    }

    if (this->m_swapChain && !this->m_format.m_window) {
        this->m_inModeSwitch = true;
        this->m_swapChain->SetFullscreenState(FALSE, nullptr);
        this->m_inModeSwitch = false;
    }

    for (uint32_t i = 0; i < 2; i++) {
        if (CGxDevice::s_uiVertexShader[i]) {
            g_theGxDevicePtr->ShaderDestroy(CGxDevice::s_uiVertexShader[i]);
        }
    }

    if (CGxDevice::s_uiPixelShader) {
        g_theGxDevicePtr->ShaderDestroy(CGxDevice::s_uiPixelShader);
    }

    for (uint32_t i = 0; i < 2; i++) {
        if (this->m_clearShaders[i]) {
            this->ShaderDestroy(this->m_clearShaders[i]);
        }
    }

    this->ICursorDestroy();
    this->ITexForceRecreation(1);
    this->IReleaseD3dShaders(1);
    this->IReleaseD3dPools(1);
    this->IReleaseD3dQueries();
    this->IReleaseTargets();
    this->IReleaseStateObjects();

    if (this->m_fanIndexBuffer) {
        this->m_fanIndexBuffer->Release();
        this->m_fanIndexBuffer = nullptr;
        this->m_fanIndexCount = 0;
    }

    ID3D11Buffer** constants[5] = { &this->m_vsConstants, &this->m_psConstants, &this->m_deviceConstantsBuffer, &this->m_emuConstantsBuffer, &this->m_clearConstants };

    for (uint32_t i = 0; i < 5; i++) {
        if (*constants[i]) {
            (*constants[i])->Release();
            *constants[i] = nullptr;
        }
    }

    if (this->m_swapChain) {
        this->m_swapChain->Release();
        this->m_swapChain = nullptr;
    }

    if (this->m_d3dContext) {
        this->m_d3dContext->Release();
        this->m_d3dContext = nullptr;
    }

    if (this->m_d3dDevice) {
        this->m_d3dDevice->Release();
        this->m_d3dDevice = nullptr;
    }

    if (this->m_dxgiAdapter) {
        this->m_dxgiAdapter->Release();
        this->m_dxgiAdapter = nullptr;
    }

    if (this->m_dxgiFactory) {
        this->m_dxgiFactory->Release();
        this->m_dxgiFactory = nullptr;
    }

    this->IStateReset();
}

int32_t CGxDeviceD3d11::DeviceCreate(GxWindowProc windowProc, const CGxFormat& format) {
    this->m_ownhwnd = 1;

    HDC dc = GetDC(nullptr);

    if (GetDeviceGammaRamp(dc, &this->m_systemGammaRamp) && &this->m_gammaRamp != &this->m_systemGammaRamp) {
        this->m_gammaRamp = this->m_systemGammaRamp;
    }

    ReleaseDC(nullptr, dc);

    this->m_hwndClass = WindowClassCreate();

    if (!this->m_hwndClass) {
        char* error = OsGetLastErrorStr();
        CGxDevice::Log("CGxDeviceD3d11::DeviceCreate(): WindowClassCreate() failed: %s", error);
        OsFreeLastErrorStr(error);
        this->DeviceDestroy();
        return 0;
    }

    if (CGxDevice::DeviceCreate(windowProc, format)) {
        return 1;
    }

    this->DeviceDestroy();
    return 0;
}

int32_t CGxDeviceD3d11::DeviceCreate(void* window, const CGxFormat& format) {
    this->m_ownhwnd = 0;
    CGxDevice::DeviceCreate(window, format);
    this->m_hwnd = static_cast<HWND>(window);

    if (this->ICreateD3dDevice(format) && CGxDevice::DeviceCreate(window, format)) {
        return 1;
    }

    this->DeviceDestroy();
    return 0;
}

void CGxDeviceD3d11::DeviceDestroy() {
    CGxDevice::DeviceDestroy();

    this->IDestroyD3d();

    if (this->m_hwnd && this->m_ownhwnd) {
        DestroyWindow(this->m_hwnd);
        this->m_hwnd = nullptr;
    }

    if (this->m_hwndClass) {
        UnregisterClassA(reinterpret_cast<const char*>(static_cast<uintptr_t>(this->m_hwndClass)), GetModuleHandleA(nullptr));
        this->m_hwndClass = 0;
    }
}

int32_t CGxDeviceD3d11::DeviceSetFormat(const CGxFormat& format) {
    CGxDevice::Log("CGxDeviceD3d11::DeviceSetFormat():");
    this->Log(format);

    if (this->m_hwnd) {
        ShowWindow(this->m_hwnd, SW_HIDE);
    }

    this->IDestroyD3d();

    if (this->m_hwnd) {
        DestroyWindow(this->m_hwnd);
    }

    this->m_hwnd = nullptr;
    this->m_format = format;

    CGxFormat createFormat = format;

    if (this->ICreateWindow(createFormat) && this->ICreateD3dDevice(createFormat) && CGxDevice::DeviceSetFormat(createFormat)) {
        this->ISetWindowFocus(1);
        return 1;
    }

    CGxDevice::Log("CGxDeviceD3d11::DeviceSetFormat(): unable to set format!");
    this->IDestroyD3d();

    if (this->m_hwnd) {
        DestroyWindow(this->m_hwnd);
    }

    this->m_hwnd = nullptr;

    return 0;
}

void CGxDeviceD3d11::IResizeSwapChain(uint32_t width, uint32_t height) {
    if (!this->m_swapChain || !width || !height || (width == this->m_backBufferWidth && height == this->m_backBufferHeight)) {
        return;
    }

    this->m_context = 0;
    this->m_d3dContext->Flush();
    this->IReleaseTargets();

    HRESULT result = this->m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, this->m_format.m_window ? 0 : DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

    if (FAILED(result)) {
        CGxDevice::Log("CGxDeviceD3d11::IResizeSwapChain(): ResizeBuffers failed: 0x%08x", result);
    }

    this->m_context = this->ICreateTargets();
    this->IStateReset();
    this->IRsForceUpdate();
    this->m_viewportDirty = 1;
    this->m_scissorRectDirty = 1;
    this->NotifyOnDeviceRestored();
}

int32_t CGxDeviceD3d11::DeviceWM(EGxWM wm, uintptr_t param1, uintptr_t param2) {
    switch (wm) {
        case GxWM_Size:
            if (param2 == Gx_SizeMinimized || param2 == Gx_SizeMaxHide) {
                this->m_windowVisible = 0;
                break;
            }

            this->m_windowVisible = 1;
            this->DeviceSetDefWindow(*reinterpret_cast<const CRect*>(param1));

            if (this->m_d3dDevice && this->m_context && !this->m_inModeSwitch) {
                const CRect& rect = *reinterpret_cast<const CRect*>(param1);
                this->IResizeSwapChain(static_cast<uint32_t>(rect.maxX - rect.minX), static_cast<uint32_t>(rect.maxY - rect.minY));
            }

            this->m_viewportDirty = 1;
            break;

        case GxWM_DisplayChange:
            if (this->m_windowVisible) {
                this->DeviceSetDefWindow(*reinterpret_cast<const CRect*>(param1));
                this->m_viewportDirty = 1;
            }

            break;

        case GxWM_Destroy:
        case GxWM_KillFocus:
            this->m_windowFocus = 0;
            break;

        case GxWM_SetFocus:
            this->ISetWindowFocus(1);
            break;

        default:
            break;
    }

    return 0;
}

void CGxDeviceD3d11::ISetWindowFocus(int32_t focus) {
    this->m_windowFocus = focus;

    if (this->m_format.m_window || !this->m_swapChain) {
        return;
    }

    this->m_inModeSwitch = true;

    if (focus) {
        DXGI_MODE_DESC mode;
        memset(&mode, 0, sizeof(mode));
        mode.Width = this->m_format.m_size.x;
        mode.Height = this->m_format.m_size.y;
        mode.Format = this->m_backBufferFormat;
        mode.RefreshRate.Numerator = this->m_format.m_refreshRate;
        mode.RefreshRate.Denominator = this->m_format.m_refreshRate ? 1 : 0;

        this->m_swapChain->ResizeTarget(&mode);
        this->m_swapChain->SetFullscreenState(TRUE, nullptr);
        this->m_inModeSwitch = false;
        this->IResizeSwapChain(this->m_format.m_size.x, this->m_format.m_size.y);
        this->m_windowVisible = 1;
        this->IApplyGamma();

        RECT rect;
        GetWindowRect(this->m_hwnd, &rect);
        ClipCursor(&rect);
        this->m_viewportDirty = 1;
        return;
    }

    this->m_swapChain->SetFullscreenState(FALSE, nullptr);
    this->m_inModeSwitch = false;
    ClipCursor(nullptr);
    ShowWindow(this->m_hwnd, SW_MINIMIZE);
}

void CGxDeviceD3d11::DeviceSetGamma(float gamma) {
    CGxDevice::DeviceSetGamma(gamma);
    this->IApplyGamma();
}

void CGxDeviceD3d11::DeviceSetGamma(const CGxGammaRamp& ramp) {
    CGxDevice::DeviceSetGamma(ramp);
    this->IApplyGamma();
}

void CGxDeviceD3d11::IApplyGamma() {
    if (!this->m_context || !this->m_swapChain || this->IDevIsWindowed()) {
        return;
    }

    IDXGIOutput* output = nullptr;

    if (FAILED(this->m_swapChain->GetContainingOutput(&output))) {
        return;
    }

    DXGI_GAMMA_CONTROL_CAPABILITIES caps;

    if (SUCCEEDED(output->GetGammaControlCapabilities(&caps)) && caps.NumGammaControlPoints) {
        DXGI_GAMMA_CONTROL control;
        memset(&control, 0, sizeof(control));
        control.Scale.Red = 1.0f;
        control.Scale.Green = 1.0f;
        control.Scale.Blue = 1.0f;

        for (uint32_t i = 0; i < caps.NumGammaControlPoints && i < 1025; i++) {
            uint32_t index = (i * 255) / (caps.NumGammaControlPoints > 1 ? caps.NumGammaControlPoints - 1 : 1);
            control.GammaCurve[i].Red = this->m_gammaRamp.m_red[index] * (1.0f / 65535.0f);
            control.GammaCurve[i].Green = this->m_gammaRamp.m_green[index] * (1.0f / 65535.0f);
            control.GammaCurve[i].Blue = this->m_gammaRamp.m_blue[index] * (1.0f / 65535.0f);
        }

        output->SetGammaControl(&control);
    }

    output->Release();
}

void CGxDeviceD3d11::ScenePresent() {
    if (this->m_context) {
        int32_t scrShotClick = this->m_scrShotClick;

        CGxDevice::ScenePresent();
        this->ICursorDraw();

        if (scrShotClick) {
            this->DeviceScreenShot();
        }

        this->WaitForFPSCap();

        uint32_t interval = this->m_format.m_vsync;

        if (interval > 1 && this->m_format.m_window) {
            interval = 1;
        }

        if (interval > 4) {
            interval = 4;
        }

        HRESULT result = this->m_swapChain->Present(interval, 0);

        if (result == DXGI_STATUS_OCCLUDED) {
            OsSleep(20);
        } else if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET) {
            HRESULT reason = this->m_d3dDevice->GetDeviceRemovedReason();
            CGxDevice::Log("CGxDeviceD3d11::ScenePresent(): device lost (present 0x%08x, reason 0x%08x), recreating", result, reason);

            CGxFormat format = this->m_format;
            this->DeviceSetFormat(format);
            this->NotifyOnDeviceRestored();
        } else if (FAILED(result)) {
            CGxDevice::Log("CGxDeviceD3d11::ScenePresent(): Present failed: 0x%08x", result);
        }

        this->ShaderConstantsClear();
    }
}

// D3D11 clears ignore the viewport. D3D9 clears stay inside it, and the shadow maps rely on that to refresh one tile at a time.
bool CGxDeviceD3d11::ISceneClearViewport(uint32_t mask, CImVector color) {
    D3D11_VIEWPORT viewport;
    const CRect& window = this->DeviceCurWindow();

    if (!this->ICalcViewport(viewport)) {
        return true;
    }

    if (viewport.TopLeftX <= 0.0f && viewport.TopLeftY <= 0.0f && viewport.Width >= window.maxX && viewport.Height >= window.maxY) {
        return false;
    }

    bool clearColor = (mask & GxBufferBit_Color) && this->m_curRtv;
    bool clearDepth = (mask & GxBufferBit_Depth) && this->m_curDsv;

    if (!clearColor && !clearDepth) {
        return true;
    }

    if (!this->m_clearShaders[0]) {
        this->ShaderCreate(&this->m_clearShaders[0], GxSh_Vertex, "Shaders\\Vertex", "Clear", 1);
        this->ShaderCreate(&this->m_clearShaders[1], GxSh_Pixel, "Shaders\\Pixel", "Clear", 1);
    }

    if (!this->m_clearConstants) {
        D3D11_BUFFER_DESC bufferDesc;
        memset(&bufferDesc, 0, sizeof(bufferDesc));
        bufferDesc.ByteWidth = 16;
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        this->m_d3dDevice->CreateBuffer(&bufferDesc, nullptr, &this->m_clearConstants);
    }

    if (!this->m_clearRasterizer) {
        D3D11_RASTERIZER_DESC desc;
        memset(&desc, 0, sizeof(desc));
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        this->m_d3dDevice->CreateRasterizerState(&desc, &this->m_clearRasterizer);
    }

    if (!this->m_clearBlend[clearColor]) {
        D3D11_BLEND_DESC desc;
        memset(&desc, 0, sizeof(desc));
        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].RenderTargetWriteMask = clearColor ? D3D11_COLOR_WRITE_ENABLE_ALL : 0;
        this->m_d3dDevice->CreateBlendState(&desc, &this->m_clearBlend[clearColor]);
    }

    if (!this->m_clearDepth[clearDepth]) {
        D3D11_DEPTH_STENCIL_DESC desc;
        memset(&desc, 0, sizeof(desc));
        desc.DepthEnable = clearDepth;
        desc.DepthWriteMask = clearDepth ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
        this->m_d3dDevice->CreateDepthStencilState(&desc, &this->m_clearDepth[clearDepth]);
    }

    CGxShader* vertexShader = this->m_clearShaders[0];
    CGxShader* pixelShader = this->m_clearShaders[1];

    if (!vertexShader->m_loaded) {
        this->IShaderCreate(vertexShader);
    }

    if (!pixelShader->m_loaded) {
        this->IShaderCreate(pixelShader);
    }

    if (!vertexShader->m_valid || !pixelShader->m_valid || !this->m_clearConstants || !this->m_clearRasterizer || !this->m_clearBlend[clearColor] || !this->m_clearDepth[clearDepth]) {
        return false;
    }

    D3D11_MAPPED_SUBRESOURCE mapped;

    if (FAILED(this->m_d3dContext->Map(this->m_clearConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        return false;
    }

    float* values = static_cast<float*>(mapped.pData);
    values[0] = color.r * oo255;
    values[1] = color.g * oo255;
    values[2] = color.b * oo255;
    values[3] = color.a * oo255;
    this->m_d3dContext->Unmap(this->m_clearConstants, 0);

    const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    this->m_d3dContext->OMSetBlendState(this->m_clearBlend[clearColor], blendFactor, 0xFFFFFFFF);
    this->m_d3dContext->OMSetDepthStencilState(this->m_clearDepth[clearDepth], 0);
    this->m_d3dContext->RSSetState(this->m_clearRasterizer);
    this->m_d3dContext->RSSetViewports(1, &viewport);
    this->m_d3dContext->IASetInputLayout(nullptr);
    this->m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    this->m_d3dContext->VSSetShader(static_cast<ID3D11VertexShader*>(vertexShader->m_apiSpecific), nullptr, 0);
    this->m_d3dContext->PSSetShader(static_cast<ID3D11PixelShader*>(pixelShader->m_apiSpecific), nullptr, 0);
    this->m_d3dContext->PSSetConstantBuffers(2, 1, &this->m_clearConstants);
    this->m_d3dContext->Draw(3, 0);

    this->IStateReset();

    return true;
}

void CGxDeviceD3d11::SceneClear(uint32_t mask, CImVector color) {
    if (!this->m_context) {
        return;
    }

    if (this->ISceneClearViewport(mask, color)) {
        return;
    }

    if ((mask & GxBufferBit_Color) && this->m_curRtv) {
        const float clearColor[4] = { color.r * oo255, color.g * oo255, color.b * oo255, color.a * oo255 };
        this->m_d3dContext->ClearRenderTargetView(this->m_curRtv, clearColor);
    }

    if ((mask & GxBufferBit_Depth) && this->m_curDsv) {
        this->m_d3dContext->ClearDepthStencilView(this->m_curDsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }
}

void CGxDeviceD3d11::DeviceSaveDepthSurface() {
    this->m_savedDsv = this->m_curDsv;
}

void CGxDeviceD3d11::DeviceRestoreDepthSurface() {
    if (!this->m_context) {
        return;
    }

    this->m_curDsv = this->m_savedDsv;
    this->ISetRenderTargets();
}

void CGxDeviceD3d11::DeviceReadPixels(CiRect& rect, TSGrowableArray<CImVector>& pixels) {
    this->ClampRectToWindow(rect);

    uint32_t height = rect.maxY - rect.minY;
    uint32_t width = rect.maxX - rect.minX;
    uint32_t count = width * height;
    pixels.SetCount(count);

    if (!width || !height || !this->m_context) {
        return;
    }

    ID3D11Texture2D* backBuffer = nullptr;

    if (FAILED(this->m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)))) {
        return;
    }

    D3D11_TEXTURE2D_DESC desc;
    backBuffer->GetDesc(&desc);

    ID3D11Texture2D* source = backBuffer;
    ID3D11Texture2D* resolved = nullptr;

    if (desc.SampleDesc.Count > 1) {
        D3D11_TEXTURE2D_DESC resolveDesc = desc;
        resolveDesc.SampleDesc.Count = 1;
        resolveDesc.SampleDesc.Quality = 0;
        resolveDesc.BindFlags = 0;

        if (SUCCEEDED(this->m_d3dDevice->CreateTexture2D(&resolveDesc, nullptr, &resolved))) {
            this->m_d3dContext->ResolveSubresource(resolved, 0, backBuffer, 0, desc.Format);
            source = resolved;
        }
    }

    D3D11_TEXTURE2D_DESC stagingDesc = desc;
    stagingDesc.SampleDesc.Count = 1;
    stagingDesc.SampleDesc.Quality = 0;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;

    ID3D11Texture2D* staging = nullptr;

    if (SUCCEEDED(this->m_d3dDevice->CreateTexture2D(&stagingDesc, nullptr, &staging))) {
        this->m_d3dContext->CopyResource(staging, source);

        D3D11_MAPPED_SUBRESOURCE mapped;

        if (SUCCEEDED(this->m_d3dContext->Map(staging, 0, D3D11_MAP_READ, 0, &mapped))) {
            if (width <= desc.Width && height <= desc.Height && (desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM || desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM)) {
                bool swap = desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM;
                uint32_t* dest = reinterpret_cast<uint32_t*>(pixels.Ptr());

                for (uint32_t y = 0; y < height; y++) {
                    const uint32_t* src = reinterpret_cast<const uint32_t*>(static_cast<const uint8_t*>(mapped.pData) + (rect.minY + y) * mapped.RowPitch) + rect.minX;

                    for (uint32_t x = 0; x < width; x++) {
                        uint32_t value = src[x];
                        *dest++ = swap ? (value & 0xFF00FF00) | ((value & 0xFF) << 16) | ((value >> 16) & 0xFF) : value;
                    }
                }
            } else {
                memset(pixels.Ptr(), 0xFF, sizeof(CImVector) * count);
            }

            this->m_d3dContext->Unmap(staging, 0);
        }

        staging->Release();
    }

    if (resolved) {
        resolved->Release();
    }

    backBuffer->Release();
}

void CGxDeviceD3d11::DeviceReadDepths(CiRect& rect, TSGrowableArray<float>& depths) {
    this->ClampRectToWindow(rect);

    uint32_t count = (rect.maxY - rect.minY) * (rect.maxX - rect.minX);
    depths.SetCount(count);
    memset(depths.Ptr(), 0, sizeof(float) * count);
}

void CGxDeviceD3d11::CursorSetVisible(int32_t visible) {
    CGxDevice::CursorSetVisible(visible);

    if (this->m_hwCursor && this->m_context) {
        POINT point;
        GetCursorPos(&point);
        ScreenToClient(this->m_hwnd, &point);

        RECT rect;
        GetClientRect(this->m_hwnd, &rect);

        if (point.x >= rect.left && point.x < rect.right && point.y >= rect.top && point.y < rect.bottom) {
            SetCursor(this->m_cursorVisible ? this->m_cursorHandle : nullptr);
        }
    }
}

void CGxDeviceD3d11::ICursorBuild() {
    if (this->m_cursorHandle) {
        DestroyCursor(this->m_cursorHandle);
        this->m_cursorHandle = nullptr;
    }

    if (!this->m_hwCursor) {
        return;
    }

    BITMAPV5HEADER header;
    memset(&header, 0, sizeof(header));
    header.bV5Size = sizeof(header);
    header.bV5Width = 32;
    header.bV5Height = -32;
    header.bV5Planes = 1;
    header.bV5BitCount = 32;
    header.bV5Compression = BI_BITFIELDS;
    header.bV5RedMask = 0x00FF0000;
    header.bV5GreenMask = 0x0000FF00;
    header.bV5BlueMask = 0x000000FF;
    header.bV5AlphaMask = 0xFF000000;

    HDC dc = GetDC(nullptr);
    void* bits = nullptr;
    HBITMAP color = CreateDIBSection(dc, reinterpret_cast<BITMAPINFO*>(&header), DIB_RGB_COLORS, &bits, nullptr, 0);
    ReleaseDC(nullptr, dc);

    HBITMAP mask = CreateBitmap(32, 32, 1, 1, nullptr);

    if (color && mask && bits) {
        memcpy(bits, this->m_cursor, sizeof(this->m_cursor));

        ICONINFO info;
        info.fIcon = FALSE;
        info.xHotspot = this->m_cursorHotspotX;
        info.yHotspot = this->m_cursorHotspotY;
        info.hbmMask = mask;
        info.hbmColor = color;
        this->m_cursorHandle = CreateIconIndirect(&info);
    }

    if (color) {
        DeleteObject(color);
    }

    if (mask) {
        DeleteObject(mask);
    }
}

void CGxDeviceD3d11::ICursorCreate(const CGxFormat& format) {
    CGxDevice::ICursorCreate(format);
    this->ICursorBuild();
}

void CGxDeviceD3d11::ICursorDestroy() {
    CGxDevice::ICursorDestroy();

    if (this->m_cursorHandle) {
        DestroyCursor(this->m_cursorHandle);
        this->m_cursorHandle = nullptr;
    }
}

void CGxDeviceD3d11::ICursorDraw() {
    CGxDevice::ICursorDraw();
}

void CGxDeviceD3d11::CursorUnlock(uint32_t x, uint32_t y) {
    CGxDevice::CursorUnlock(x, y);
    this->ICursorBuild();

    if (this->m_hwCursor && this->m_cursorVisible) {
        POINT point;
        GetCursorPos(&point);
        ScreenToClient(this->m_hwnd, &point);

        RECT rect;
        GetClientRect(this->m_hwnd, &rect);

        if (point.x >= rect.left && point.x < rect.right && point.y >= rect.top && point.y < rect.bottom) {
            SetCursor(this->m_cursorHandle);
        }
    }
}

void CGxDeviceD3d11::QueryCreate(CGxQuery*& query, EGxQueryType type) {
    CGxDevice::QueryCreate(query, type);

    if (query->m_apiSpecific || !this->m_d3dDevice) {
        return;
    }

    D3D11_QUERY_DESC desc;
    desc.Query = D3D11_QUERY_OCCLUSION;
    desc.MiscFlags = 0;

    ID3D11Query* d3dQuery;

    if (SUCCEEDED(this->m_d3dDevice->CreateQuery(&desc, &d3dQuery))) {
        query->m_apiSpecific = d3dQuery;
    }
}

void CGxDeviceD3d11::QueryDestroy(CGxQuery*& query) {
    if (query->m_apiSpecific) {
        static_cast<ID3D11Query*>(query->m_apiSpecific)->Release();
    }

    query->m_apiSpecific = nullptr;
    CGxDevice::QueryDestroy(query);
}

bool CGxDeviceD3d11::QueryBegin(CGxQuery* query) {
    if (!this->m_context) {
        return false;
    }

    if (!query->m_apiSpecific) {
        CGxQuery* created = query;
        this->QueryCreateApi(created);

        if (!query->m_apiSpecific) {
            return false;
        }
    }

    if (query->m_type == GxQueryType_Occlusion) {
        this->m_d3dContext->Begin(static_cast<ID3D11Query*>(query->m_apiSpecific));
    }

    return true;
}

void CGxDeviceD3d11::QueryCreateApi(CGxQuery* query) {
    D3D11_QUERY_DESC desc;
    desc.Query = D3D11_QUERY_OCCLUSION;
    desc.MiscFlags = 0;

    ID3D11Query* d3dQuery;

    if (SUCCEEDED(this->m_d3dDevice->CreateQuery(&desc, &d3dQuery))) {
        query->m_apiSpecific = d3dQuery;
    }
}

bool CGxDeviceD3d11::QueryEnd(CGxQuery* query) {
    if (!this->m_context || !query->m_apiSpecific) {
        return false;
    }

    if (query->m_type == GxQueryType_Occlusion) {
        this->m_d3dContext->End(static_cast<ID3D11Query*>(query->m_apiSpecific));
    }

    return true;
}

bool CGxDeviceD3d11::QueryGetParam(CGxQuery* query, EGxQueryParam param, uint32_t& data) {
    if (!this->m_context) {
        return false;
    }

    if (!query->m_apiSpecific) {
        this->QueryCreateApi(query);

        if (!query->m_apiSpecific) {
            return false;
        }
    }

    ID3D11Query* d3dQuery = static_cast<ID3D11Query*>(query->m_apiSpecific);

    switch (param) {
        case GxQueryParam_Unk0:
            data = d3dQuery->GetDataSize();
            return true;

        case GxQueryParam_Unk1:
            data = this->m_d3dContext->GetData(d3dQuery, nullptr, 0, 0) == S_OK;
            return false;

        default:
            return false;
    }
}

bool CGxDeviceD3d11::QueryGetData(CGxQuery* query, uint32_t& data) {
    if (!this->m_context || !query->m_apiSpecific) {
        return false;
    }

    ID3D11Query* d3dQuery = static_cast<ID3D11Query*>(query->m_apiSpecific);
    UINT64 samples = 0;
    HRESULT result;
    uint32_t start = GetTickCount();

    do {
        result = this->m_d3dContext->GetData(d3dQuery, &samples, sizeof(samples), 0);
    } while (result == S_FALSE && GetTickCount() - start < 500);

    data = static_cast<uint32_t>(samples);

    return result == S_OK;
}

void CGxDeviceD3d11::IReleaseD3dQueries() {
    for (CGxQuery* query = this->m_queryList.Head(); query; query = this->m_queryList.Next(query)) {
        if (query->m_apiSpecific) {
            static_cast<ID3D11Query*>(query->m_apiSpecific)->Release();
        }

        query->m_apiSpecific = nullptr;
    }
}
