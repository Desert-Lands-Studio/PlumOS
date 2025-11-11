#include "rhi.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <stdio.h>
#include <stdlib.h>


#ifdef __cplusplus
using Microsoft::WRL::ComPtr;
#else
typedef struct IUnknown* ComPtr;
#endif

typedef struct {
    PlumRhi base;
    ID3D12Device* device;
    ID3D12CommandQueue* command_queue;
    IDXGIFactory4* dxgi_factory;
    IDXGISwapChain* swap_chain;
    void* window_handle;
} D3D12Backend;

static bool is_d3d12_available(void) {
    HMODULE d3d12 = LoadLibraryA("d3d12.dll");
    if (!d3d12) return false;
    
    
    FARPROC create_device = GetProcAddress(d3d12, "D3D12CreateDevice");
    FreeLibrary(d3d12);
    
    return create_device != NULL;
}

static HRESULT create_d3d12_device(IDXGIFactory4* factory, ID3D12Device** device) {
    ComPtr<IDXGIAdapter1> adapter;
    
    
    for (UINT adapter_index = 0; 
         factory->EnumAdapters1(adapter_index, &adapter) != DXGI_ERROR_NOT_FOUND; 
         adapter_index++) {
        
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
        
        
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            continue;
        }
        
        
        HRESULT hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, 
                                      _uuidof(ID3D12Device), (void**)device);
        if (SUCCEEDED(hr)) {
            printf("✅ Using DirectX 12 adapter: %ls\n", desc.Description);
            return hr;
        }
    }
    
    return E_FAIL;
}

static PlumRhi* create_d3d12(const PlumRhiVersion* requested_version) {
    printf("Creating DirectX 12 backend for Windows\n");
    
    if (!is_d3d12_available()) {
        fprintf(stderr, "D3D12 not available on this system\n");
        return NULL;
    }
    
    D3D12Backend* backend = (D3D12Backend*)malloc(sizeof(D3D12Backend));
    if (!backend) return NULL;
    
    
    HRESULT hr = CreateDXGIFactory1(&IID_IDXGIFactory4, (void**)&backend->dxgi_factory);
    if (FAILED(hr)) {
        fprintf(stderr, "Failed to create DXGI factory: 0x%X\n", hr);
        free(backend);
        return NULL;
    }
    
    
    hr = create_d3d12_device(backend->dxgi_factory, &backend->device);
    if (FAILED(hr)) {
        fprintf(stderr, "Failed to create D3D12 device: 0x%X\n", hr);
        backend->dxgi_factory->Release();
        free(backend);
        return NULL;
    }
    
    
    D3D12_COMMAND_QUEUE_DESC queue_desc = {
        .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
        .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0
    };
    
    hr = backend->device->CreateCommandQueue(&queue_desc, &IID_ID3D12CommandQueue, 
                                            (void**)&backend->command_queue);
    if (FAILED(hr)) {
        fprintf(stderr, "Failed to create command queue: 0x%X\n", hr);
        backend->device->Release();
        backend->dxgi_factory->Release();
        free(backend);
        return NULL;
    }
    
    backend->swap_chain = NULL;
    backend->window_handle = NULL;
    
    backend->base.info = &d3d12_info;
    backend->base.version.major = 12;
    backend->base.version.minor = 0;
    
    printf("✅ DirectX 12 backend created successfully\n");
    return &backend->base;
}

static void destroy_d3d12(PlumRhi* rhi) {
    if (!rhi) return;
    D3D12Backend* backend = (D3D12Backend*)rhi;
    
    if (backend->swap_chain) {
        backend->swap_chain->Release();
    }
    if (backend->command_queue) {
        backend->command_queue->Release();
    }
    if (backend->device) {
        backend->device->Release();
    }
    if (backend->dxgi_factory) {
        backend->dxgi_factory->Release();
    }
    free(backend);
}


static bool d3d12_create_swap_chain(D3D12Backend* backend, void* window_handle, 
                                   int width, int height) {
    if (!backend || !window_handle) return false;
    
    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {
        .BufferDesc = {
            .Width = width,
            .Height = height,
            .RefreshRate = { 60, 1 }, 
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
            .Scaling = DXGI_MODE_SCALING_UNSPECIFIED
        },
        .SampleDesc = { .Count = 1, .Quality = 0 },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 2, 
        .OutputWindow = (HWND)window_handle,
        .Windowed = TRUE,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
    };
    
    HRESULT hr = backend->dxgi_factory->CreateSwapChain(
        backend->command_queue, &swap_chain_desc, &backend->swap_chain);
    
    if (FAILED(hr)) {
        fprintf(stderr, "Failed to create swap chain: 0x%X\n", hr);
        return false;
    }
    
    backend->window_handle = window_handle;
    return true;
}

static const PlumRhiInfo d3d12_info = {
    PLUM_RHI_D3D12,
    "D3D12",
    {12, 0},
    90,
    is_d3d12_available,
    create_d3d12,
    destroy_d3d12
};

__attribute__((constructor)) 
static void register_directx_backend(void) {
    if (is_d3d12_available()) {
        plum_rhi_register(&d3d12_info);
        printf("Registered DirectX 12 backend for Windows\n");
    }
}