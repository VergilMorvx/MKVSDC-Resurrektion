#pragma once

#include <rex/rex_app.h>
#include <spdlog/sinks/base_sink.h>

#include <chrono>
#include <fstream>
#include <mutex>
#include <string_view>

#if defined(_WIN32)
#include <windows.h>
#include <dxgi.h>
#endif

// Lightweight, zero-overhead diagnostic telemetry sink
// Captures file streaming (.xxx), video playback (.wmv), and shader/pipeline creation
class DiagnosticTelemetrySink : public spdlog::sinks::base_sink<std::mutex> {
 public:
  explicit DiagnosticTelemetrySink(const std::filesystem::path& log_path)
      : out_(log_path, std::ios::out | std::ios::app) {}

 protected:
  void sink_it_(const spdlog::details::log_msg& msg) override {
    std::string_view payload(msg.payload.data(), msg.payload.size());
    bool is_movie = (payload.find(".wmv") != std::string_view::npos ||
                     payload.find(".WMV") != std::string_view::npos ||
                     payload.find("Movies") != std::string_view::npos ||
                     payload.find("Movie") != std::string_view::npos);
    bool is_shader = (payload.find("shader") != std::string_view::npos ||
                      payload.find("Shader") != std::string_view::npos ||
                      payload.find("pipeline") != std::string_view::npos ||
                      payload.find("Pipeline") != std::string_view::npos);
    bool is_pkg = (payload.find(".xxx") != std::string_view::npos ||
                   payload.find("Asset") != std::string_view::npos);

    bool is_content = (payload.find("XamContent") != std::string_view::npos ||
                       payload.find("Content") != std::string_view::npos ||
                       payload.find("profile") != std::string_view::npos ||
                       payload.find("Save") != std::string_view::npos ||
                       payload.find("save") != std::string_view::npos);

    if (payload.find("k_1_REVERSE") != std::string_view::npos) {
      return;
    }

    if (is_movie || is_shader || is_pkg || is_content || msg.level >= spdlog::level::warn) {
      auto now = std::chrono::steady_clock::now();
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
      if (out_.is_open()) {
        out_ << "[" << ms << " ms] [" << msg.logger_name.data() << "] "
             << "[" << spdlog::level::to_string_view(msg.level).data() << "] "
             << payload << "\n";
        out_.flush();
      }
    }
  }

  void flush_() override {
    if (out_.is_open()) {
      out_.flush();
    }
  }

 private:
  std::ofstream out_;
};

class MkvdcApp : public rex::ReXApp {

 public:
  using rex::ReXApp::ReXApp;

  static std::unique_ptr<rex::ui::WindowedApp> Create(
      rex::ui::WindowedAppContext& ctx) {
    return std::unique_ptr<MkvdcApp>(new MkvdcApp(ctx, "mkvdc",
        PPCImageConfig));
  }

  void OnPostInitLogging() override {
    std::filesystem::path diag_dir = "logs";
    std::error_code ec;
    std::filesystem::create_directories(diag_dir, ec);
    auto diag_sink = std::make_shared<DiagnosticTelemetrySink>(diag_dir / "diagnostic_telemetry.log");
    rex::AddSink(diag_sink);

    // Target GPU and FileSystem specifically for shader compilation, PSO, and movie/package tracing
    if (auto cat_gpu = rex::FindCategory("gpu")) {
      rex::SetCategoryLevel(*cat_gpu, spdlog::level::debug);
    }
    if (auto cat_fs = rex::FindCategory("fs")) {
      rex::SetCategoryLevel(*cat_fs, spdlog::level::debug);
    }
    if (auto cat_krnl = rex::FindCategory("krnl")) {
      rex::SetCategoryLevel(*cat_krnl, spdlog::level::trace);
    }
  }


  void OnConfigurePaths(rex::PathConfig& paths) override {

    if (paths.game_data_root.empty()) {
      std::filesystem::path default_root =
          "D:/mkvsdc/MKvDC_Extracted/Mortal Kombat vs. DC Universe (World) (En,Fr,De,Es,It)";
      if (std::filesystem::is_directory(default_root)) {
        paths.game_data_root = default_root;
      }
    }

    if (paths.user_data_root.empty()) {
      paths.user_data_root = std::filesystem::current_path() / "savedata";
    }
    std::error_code ec;
    std::filesystem::create_directories(paths.user_data_root, ec);
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

    // Enforce 60 FPS frame pacing baseline (SyncInterval = 1) unless explicitly overridden
    std::string current_interval = rex::cvar::GetFlagByName("d3d12_present_interval");
    if (current_interval.empty()) {
      rex::cvar::SetFlagByName("d3d12_present_interval", "1");
    }

    // Enable keyboard/mouse controller emulation by default (Enter=Start, Space=A, Backspace=B, WASD)
    std::string current_mnk = rex::cvar::GetFlagByName("mnk_mode");
    if (current_mnk.empty() || current_mnk == "false") {
      rex::cvar::SetFlagByName("mnk_mode", "true");
    }

    // Permit background/unfocused input processing for automation and seamless window switching
    std::string current_mnk_focus = rex::cvar::GetFlagByName("mnk_ignore_focus");
    if (current_mnk_focus.empty() || current_mnk_focus == "false") {
      rex::cvar::SetFlagByName("mnk_ignore_focus", "true");
    }
  }
};

