#include <cstdint>
#include <cstring>
#include "Engine/Source/Gx/CGxDeviceD3d11/CGxDeviceD3d11.h"
#include "Engine/Source/Gx/CGxBuf.h"
#include "Engine/Source/Gx/CGxPool.h"
#include "Engine/Source/Storm/H/storm.h"

// OFFSET-15595: 0x008526D0/0x140582160
int32_t CGxDeviceD3d11::ICreatePoolApi(CGxPool* pool) {
    if (!this->m_d3dDevice || pool->m_size <= 0) {
        return 0;
    }

    D3D11_BUFFER_DESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.ByteWidth = pool->m_size;
    desc.Usage = pool->m_usage == GxPoolUsage_Static ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = pool->m_usage == GxPoolUsage_Static ? 0 : D3D11_CPU_ACCESS_WRITE;
    desc.BindFlags = pool->m_target == GxPoolTarget_Vertex ? D3D11_BIND_VERTEX_BUFFER : D3D11_BIND_INDEX_BUFFER;

    ID3D11Buffer* buffer = nullptr;

    if (FAILED(this->m_d3dDevice->CreateBuffer(&desc, nullptr, &buffer))) {
        return 0;
    }

    pool->m_apiSpecific = buffer;

    return 1;
}

void CGxDeviceD3d11::IReleasePool(CGxPool* pool) {
    if (pool->m_usage == GxPoolUsage_Stream) {
        pool->m_offset = 0;
    }

    pool->Invalidate();

    if (!pool->m_apiSpecific) {
        return;
    }

    ID3D11Buffer* buffer = static_cast<ID3D11Buffer*>(pool->m_apiSpecific);

    // a new buffer can get the same address, so drop the cached bindings
    if (this->m_curIndexBuffer == buffer) {
        this->m_curIndexBuffer = nullptr;
    }

    for (uint32_t i = 0; i < GxVAs_Last; i++) {
        if (this->m_curVertexBuffers[i] == buffer) {
            this->m_curVertexBuffers[i] = nullptr;
        }
    }

    buffer->Release();
    pool->m_apiSpecific = nullptr;
}

// OFFSET-15595: 0x00852750/0x140582200
void CGxDeviceD3d11::PoolSizeSet(CGxPool* pool, uint32_t size) {
    this->IReleasePool(pool);
    pool->m_size = size;
    this->ICreatePoolApi(pool);
}

// OFFSET-15595: 0x008527A0/0x140582270
void CGxDeviceD3d11::PoolDestroy(CGxPool* pool) {
    if (pool) {
        this->IReleasePool(pool);
        CGxDevice::PoolDestroy(pool);
    }
}

void CGxDeviceD3d11::IReleaseD3dPools(int32_t freePools) {
    for (CGxPool* pool = this->m_poolList.Head(); pool; pool = this->m_poolList.Next(pool)) {
        if (freePools || pool->m_usage == GxPoolUsage_Stream || pool->m_usage == GxPoolUsage_Dynamic) {
            this->IReleasePool(pool);
        }
    }
}

// OFFSET-15595: 0x008527F0/0x1405822D0
char* CGxDeviceD3d11::IBufLock(CGxBuf* buf) {
    CGxPool* pool = buf->m_pool;

    if (!this->m_context) {
        return static_cast<char*>(this->m_emergencyMem[pool->m_target].Lock(buf->m_size));
    }

    if (!pool->m_apiSpecific) {
        this->ICreatePoolApi(pool);
    }

    ID3D11Buffer* buffer = static_cast<ID3D11Buffer*>(pool->m_apiSpecific);

    if (!buffer) {
        return static_cast<char*>(this->m_emergencyMem[pool->m_target].Lock(buf->m_size));
    }

    if (pool->m_usage == GxPoolUsage_Static) {
        return static_cast<char*>(this->m_staticEmergencyMem[pool->m_target].Lock(buf->m_size));
    }

    D3D11_MAP mapType = D3D11_MAP_WRITE_NO_OVERWRITE;

    if (pool->m_usage == GxPoolUsage_Stream) {
        uint32_t index = (buf->m_itemSize + pool->m_offset - 1) - (buf->m_itemSize + pool->m_offset - 1) % buf->m_itemSize;

        if (index + buf->m_size > static_cast<uint32_t>(pool->m_size)) {
            mapType = D3D11_MAP_WRITE_DISCARD;
            pool->Discard();
            buf->m_index = 0;
            pool->m_offset = buf->m_size;
        } else {
            buf->m_index = index;
            pool->m_offset = index + buf->m_size;
        }
    }

    D3D11_MAPPED_SUBRESOURCE mapped;

    if (FAILED(this->m_d3dContext->Map(buffer, 0, mapType, 0, &mapped))) {
        return static_cast<char*>(this->m_emergencyMem[pool->m_target].Lock(buf->m_size));
    }

    return static_cast<char*>(mapped.pData) + buf->m_index;
}

char* CGxDeviceD3d11::BufLock(CGxBuf* buf) {
    CGxDevice::BufLock(buf);
    return this->IBufLock(buf);
}

// OFFSET-15595: 0x00853D70/0x140583A30
void CGxDeviceD3d11::IBufUnlock(CGxBuf* buf, uint32_t size) {
    CGxPool* pool = buf->m_pool;
    EmergencyMem& emergencyMem = this->m_emergencyMem[pool->m_target];

    if (emergencyMem.m_lock) {
        emergencyMem.Unlock();
        buf->m_valid = 0;
        this->m_intF5C = 1;
        return;
    }

    ID3D11Buffer* buffer = static_cast<ID3D11Buffer*>(pool->m_apiSpecific);

    if (pool->m_usage == GxPoolUsage_Static) {
        EmergencyMem& staticMem = this->m_staticEmergencyMem[pool->m_target];
        staticMem.Unlock();

        if (!size) {
            size = buf->m_size;
        }

        D3D11_BOX box;
        box.left = buf->m_index;
        box.top = 0;
        box.front = 0;
        box.right = buf->m_index + size;
        box.bottom = 1;
        box.back = 1;
        this->m_d3dContext->UpdateSubresource(buffer, 0, &box, staticMem.m_data.Ptr(), 0, 0);
    } else {
        this->m_d3dContext->Unmap(buffer, 0);
    }

    buf->m_valid = 1;
}

int32_t CGxDeviceD3d11::BufUnlock(CGxBuf* buf, uint32_t size) {
    CGxDevice::BufUnlock(buf, size);
    this->IBufUnlock(buf, size);

    return 1;
}

// OFFSET-15595: 0x00853E90/0x140583BA0
void CGxDeviceD3d11::BufData(CGxBuf* buf, const void* data, uint32_t size, uint32_t offset) {
    CGxDevice::BufData(buf, data, size, offset);

    CGxPool* pool = buf->m_pool;

    if (this->m_context && pool->m_usage == GxPoolUsage_Static) {
        if (!pool->m_apiSpecific) {
            this->ICreatePoolApi(pool);
        }

        if (pool->m_apiSpecific) {
            D3D11_BOX box;
            box.left = buf->m_index + offset;
            box.top = 0;
            box.front = 0;
            box.right = buf->m_index + offset + size;
            box.bottom = 1;
            box.back = 1;
            this->m_d3dContext->UpdateSubresource(static_cast<ID3D11Buffer*>(pool->m_apiSpecific), 0, &box, data, 0, 0);
            buf->m_valid = 1;
            return;
        }
    }

    char* dest = this->IBufLock(buf);
    memcpy(dest + offset, data, size);
    this->IBufUnlock(buf, 0);
}
