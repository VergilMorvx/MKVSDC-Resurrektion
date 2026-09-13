# MKVSDC: Resurrektion

**MKVSDC: Resurrektion** is an advanced static recompilation of *Mortal Kombat vs. DC Universe* (Xbox 360 Title ID `0x4D5707E9`, Media ID `6153914C`) for modern 64-bit Windows platforms, powered by the [ReXGlue SDK](https://github.com/rexglue/rexglue).

By translating PowerPC machine instructions into native C++ ahead-of-time (AOT) and mapping Xbox 360 kernel/hardware subsystems to modern APIs (Direct3D 12, SDL3 audio, VFS), *Resurrektion* enables the game to run natively on PC hardware with high framerates, low latency, and modern GPU features.

---

## Current Status & Milestones

| Milestone | Status | Description |
| :--- | :---: | :--- |
| **Phase A–F: Foundation** | **COMPLETE** | Executable decoding, function boundary analysis, strict codegen, CRT initialization, kernel hooks, VFS mounting. |
| **Phase G: First Visual Frame** | **COMPLETE** | Real 3D Unreal Engine 3 graphics rendered directly to the host window with verified non-black frames. |
| **Phase H: 3D Attract & In-Game** | **COMPLETE** | In-engine matches rendering with dynamic lighting, skeletal animation, particle systems, and 5.1 surround audio. |
| **Phase I: Interactive Combat & Optimization** | **COMPLETE** | Interactive gameplay confirmed. First-encounter combat stutter resolved via persistent D3D12 PSO caching. Frame presentation locked to rock-solid 60 FPS (`SyncInterval = 1`). Discrete GPU auto-selection (NVIDIA RTX 4060). Keyboard/mouse emulation enabled. |
| **Phase J: Cinematic & Movie Subsystem** | **COMPLETE** | 119 WMV files audited. In-engine Xenon DXVA decoder shaders verified active: bit-for-bit verified playback of Studio Logo movies (`midway_logo.wmv`, `WB_Logo.wmv`, `DC_Logo.wmv`) and full in-engine Story Mode intros (`MK001.wmv` & `dc001.wmv`). |
| **Phase K: Save System & Profile Persistence** | **COMPLETE** | Local storage container mounting (`XamContentCreate`, `XamContentOpen`, `XamContentClose`) redirected to `./savedata/`, automated headless profile registration, container persistence (`MK vs. DCU Game Settings` & `MK vs DCU`), and verified cold-restart state restoration. |

---

## Technical Architecture

### 1. Ahead-of-Time (AOT) PowerPC Static Translation
- Decompiles the original `default.xex` executable into modular C++ translation units.
- Eliminates function slicing and invalid basic block splits through custom boundary manifests:
  - `config/vtables.toml`: 116 validated virtual method entry points and constructor leaves.
  - `config/gnatives.toml`: 24 UnrealScript bytecode VM native dispatchers and operator jump tables.
  - `config/dispatch_stubs.toml`: 349 COM and virtual interface dispatch forwarders.
  - `config/functions.toml`: Strict function boundary overrides, including the CRT `qsort` comparator (`0x82C075A8`).

### 2. Direct3D 12 Rendering & Pipeline State (PSO) Caching
- **GPU Path**: Modern discrete GPUs (such as NVIDIA GeForce RTX 4060) utilize the high-performance Host Render Targets path (`Path::kHostRenderTargets`), utilizing standard hardware RTV/DSVs.
- **PSO Stutter Elimination**: First-encounter combat hitches caused by runtime host driver PSO compilation are eliminated using persistent pre-compiled pipeline caches (`4D5707E9.rtv.d3d12.xpso`). 597+ verified graphics pipelines deserialize in ~130 milliseconds on launch.
- **60 FPS Frame Pacing**: Features a dedicated `d3d12_present_interval` presentation lock (`SyncInterval = 1`) ensuring tear-free, cadence-matched 60 FPS delivery synchronized with the engine's 60 Hz tick rate.
- **High-Performance GPU Auto-Selection**: Automatically enumerates DXGI adapters and binds the discrete GPU with the largest dedicated VRAM pool (`NvOptimusEnablement` and `AmdPowerXpressRequestHighPerformance` enabled).

### 3. Audio Subsystem
- Multi-channel 5.1 surround sound audio pipeline decoded from XMA streams and presented via SDL audio sinks.

### 4. Cinematic & Video Playback
- Reverse-engineered in-engine Microsoft Xenon WMV DXVA decoder (`video\wmv\xplat\decoder_c9e\dxva_pk\xenon`).
- Leverages recompiled Xenos vertex/pixel shaders (`Shader_DetileY`, `Shader_DetileUV`, `cYUV`) for hardware-accelerated motion compensation and YUV-to-RGB detiling.
- 119 cataloged cutscenes covering story cinematics, character endings, and tournament intros.
- Live zero-overhead file access and pipeline tracing via `DiagnosticTelemetrySink`.

---

## Building from Source

### Prerequisites
- **Operating System**: Windows 10/11 (64-bit).
- **Toolchain**: Visual Studio 2022 / Build Tools with Clang/LLVM and MSVC v143.
- **Build System**: CMake (>= 3.28) and Ninja.
- **Python**: Python 3.10+ (for helper scripts and verification).
- **SDK**: ReXGlue SDK (pinned commit `c94f5eb`).

### Build Steps

```powershell
# 1. Clone repository
git clone https://github.com/VergilMorvx/MKVSDC-Resurrektion.git
cd MKVSDC-Resurrektion

# 2. Configure build preset (Release x64)
cmake --preset win-amd64-release

# 3. Compile host executable
cmake --build --preset win-amd64-release
```

The output executable `mkvdc.exe` will be located in `out/build/win-amd64-release/`.

---

## Running the Game

Run the executable and provide the path to your extracted Xbox 360 game data:

```powershell
.\out\build\win-amd64-release\mkvdc.exe --game_data_root="D:\Games\MKvDC_Extracted"
```

### Useful Command-Line Flags
- `--d3d12_present_interval=1`: Enforces locked 60 FPS VSync presentation (default). Set to `0` for uncapped framerates.
- `--log_level=info`: Sets standard engine logging verbosity (options: `trace`, `debug`, `info`, `warning`, `error`).
- Telemetry logs are automatically recorded to `logs/diagnostic_telemetry.log`.

---

## Documentation

Comprehensive reverse-engineering reports and implementation notes are available in the [`docs/`](docs/) directory:
- [`docs/devlog.md`](docs/devlog.md): Chronological engineering log covering passes 1 through 9, bug fixes, and profiling experiments.
- [`docs/analysis-ledger.md`](docs/analysis-ledger.md): Detailed reverse engineering ledger of all identified assembly routines, vtable gaps, and dispatchers.
- [`docs/blockers.md`](docs/blockers.md): Milestone tracker, open investigations, and active goals.
- [`docs/wmv_status.md`](docs/wmv_status.md): Complete audit and status matrix for all 119 WMV video assets.
- [`docs/wmv_ledger.json`](docs/wmv_ledger.json): Machine-readable catalog containing hashes, dimensions, durations, and bitrates of all game cutscenes.

---

## Disclaimer & Copyright

**MKVSDC: Resurrektion** does **NOT** distribute any proprietary or copyrighted game assets. No textures, 3D models, audio files, game scripts, pre-rendered movies (`.wmv`), or original executable binaries (`.xex`, `.xxx`) are included in this repository.

To build and run the game, users must provide their own legitimately obtained copy of *Mortal Kombat vs. DC Universe* for the Xbox 360. All trademarks, character names, logos, and game assets are property of Warner Bros. Interactive Entertainment, NetherRealm Studios, DC Comics, and Midway Games.
