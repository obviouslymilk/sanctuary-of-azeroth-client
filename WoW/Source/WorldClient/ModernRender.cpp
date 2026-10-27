#include "ModernRender.h"
#include <cstdint>
#include "Engine/Source/GxModern/GxModern.h"
#include "Engine/Source/Model2/M2Light.h"
#include "Engine/Source/Tempest/CImVector.h"
#include "Engine/Source/Tempest/CRect.h"
#include "WoW/Source/WorldClient/DayNight__DNInfo.h"
#include "WoW/Source/WorldClient/Map.h"
#include "WoW/Source/WorldClient/MapLight.h"
#include "WoW/Source/WorldClient/World.h"

void ModernRender::Begin() {
    GxModern::BeginWorld();
}

void ModernRender::AfterOpaque() {
    DayNight::DNInfo* info = DayNight::GetInfo();

    if (!info || !CMap::s_mapLight) {
        return;
    }

    GxModern::WorldParams params;
    params.fogColor[0] = info->m_fog.m_color.r / 255.0f;
    params.fogColor[1] = info->m_fog.m_color.g / 255.0f;
    params.fogColor[2] = info->m_fog.m_color.b / 255.0f;
    params.fogStart = info->m_fog.m_start;
    params.fogEnd = info->m_fog.m_end;
    params.interiorBlend = info->m_interiorFogBlend;
    params.underwater = World::SceneCamLiquidStatus(nullptr) != 0;

    const C3Vector& dir = CMap::s_mapLight->m_light.m_dir;
    params.sunDir[0] = dir.x;
    params.sunDir[1] = dir.y;
    params.sunDir[2] = dir.z;

    params.sunColor[0] = info->m_light0.m_diffuse.r / 255.0f;
    params.sunColor[1] = info->m_light0.m_diffuse.g / 255.0f;
    params.sunColor[2] = info->m_light0.m_diffuse.b / 255.0f;

    params.farClip = World::s_farClip;
    params.cameraPos[0] = info->m_cameraPos.x;
    params.cameraPos[1] = info->m_cameraPos.y;
    params.cameraPos[2] = info->m_cameraPos.z;
    params.playerZ = info->m_playerPos.z;
    params.deltaSec = info->m_deltaSec;
    params.time = info->m_timeSec;

    GxModern::AfterOpaque(params);
}

void ModernRender::End(const CRect& viewportNDC) {
    GxModern::EndWorld(viewportNDC.minX, viewportNDC.minY, viewportNDC.maxX, viewportNDC.maxY);
}
