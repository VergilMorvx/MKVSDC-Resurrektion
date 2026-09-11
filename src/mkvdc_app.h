#pragma once

#include <rex/rex_app.h>

#if defined(_WIN32)
#include <windows.h>
#include <dxgi.h>
#endif

class MkvdcApp : public rex::ReXApp {
 public:
  using rex::ReXApp::ReXApp;

  static std::unique_ptr<rex::ui::WindowedApp> Create(
      rex::ui::WindowedAppContext& ctx) {
    return std::unique_ptr<MkvdcApp>(new MkvdcApp(ctx, "mkvdc",
        PPCImageConfig));
  }

  void OnConfigurePaths(rex::PathConfig& paths) override {
    if (paths.game_data_root.empty()) {
      std::filesystem::path default_root =
          "D:/mkvsdc/MKvDC_Extracted/Mortal Kombat vs. DC Universe (World) (En,Fr,De,Es,It)";
      if (std::filesystem::is_directory(default_root)) {
        paths.game_data_root = default_root;
      }
    }
  }

  void OnPreSetup(rex::RuntimeConfig& config) override {
    if (config.gpu_plugin.empty()) {
      config.gpu_plugin = "xenos";
    }

#if defined(_WIN32)
    // Auto-detect high-performance discrete GPU (e.g. NVIDIA RTX 4060)
    // If d3d12_adapter is default (-1), select the adapter with the highest dedicated VRAM.
    std::string current_adapter = rex::cvar::GetFlagByName("d3d12_adapter");
    if (current_adapter.empty() || current_adapter == "-1") {
      HMODULE dxgi_lib = LoadLibraryW(L"dxgi.dll");
      if (dxgi_lib) {
        typedef HRESULT(WINAPI * PFN_CreateDXGIFactory1)(REFIID, void**);
        auto pCreateDXGIFactory1 = reinterpret_cast<PFN_CreateDXGIFactory1>(
            GetProcAddress(dxgi_lib, "CreateDXGIFactory1"));
        if (pCreateDXGIFactory1) {
          IDXGIFactory1* factory = nullptr;
          if (SUCCEEDED(pCreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
            int best_index = -1;
            SIZE_T max_vram = 0;
            UINT idx = 0;
            IDXGIAdapter1* adapter = nullptr;
            while (factory->EnumAdapters1(idx, &adapter) == S_OK) {
              DXGI_ADAPTER_DESC1 desc;
              if (SUCCEEDED(adapter->GetDesc1(&desc))) {
                if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
                  if (desc.DedicatedVideoMemory > max_vram) {
                    max_vram = desc.DedicatedVideoMemory;
                    best_index = static_cast<int>(idx);
                  }
                }
              }
              adapter->Release();
              ++idx;
            }
            factory->Release();
            if (best_index >= 0) {
              rex::cvar::SetFlagByName("d3d12_adapter", std::to_string(best_index));
            }
          }
        }
        FreeLibrary(dxgi_lib);
      }
    }
#endif
  }
};

