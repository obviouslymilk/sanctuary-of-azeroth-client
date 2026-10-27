#ifndef ENGINE_SOURCE_GXMODERN_GXMODERN_H
#define ENGINE_SOURCE_GXMODERN_GXMODERN_H

#include <cstdint>
#include <windows.h>

namespace GxModern {

struct WorldParams {
    float fogColor[3];
    float fogStart;
    float fogEnd;
    float interiorBlend;
    bool underwater;
    float sunDir[3];
    float sunColor[3];
    float farClip;
    float cameraPos[3];
    float playerZ;
    float deltaSec;
    float time;
};

void RegisterCVars();
bool WantsPipelineAtDeviceCreate();
bool Active();
void ReleaseResources();
void Shutdown();

void BeginWorld();
void AfterSolid(const WorldParams& params);
void AfterOpaque(const WorldParams& params);
void EndWorld(float minX, float minY, float maxX, float maxY);

bool PanelWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
void PanelRender();

}

#endif
