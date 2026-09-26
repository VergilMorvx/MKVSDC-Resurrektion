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
| **Phase L: Mod Framework** | **IMPLEMENTED** | Mod discovery, central TOML overrides, priority overlays, and runtime hooks. Asset overlays are applied at startup and need a restart after changing. |
| **Phase M / M+: Dark Kahn** | **PARTIALLY VERIFIED** | The current captures show Dark Kahn on the Player 2 versus screen and in Practice combat, responding to a hit. They do not capture the select screen after Dark Kahn is chosen, so selected-state artwork remains unverified; announcer playback and particle attachment are also open. |
| **Audit remediation** | **IN PROGRESS** | The pinned SDK patch applies to a clean SDK worktree; a separate project worktree completed fresh codegen, 36 hook insertions, and a source-SDK build. The regenerated main build passed a Practice and mod startup check. Remaining runtime safety and Phase M+ paths are listed below. |

Phases G–K summarize earlier milestone evidence; the current audit reran the
source build, mod startup, and Player 2 Dark Kahn Practice path.

## Project Stages, Audit Work, and Change Record

This section consolidates the project roadmap and the work recorded in the
engineering log. It is a status index, not a claim that every historic milestone
was rerun during the 2026-09-26 audit. Detailed run notes and reverse-engineering
evidence are linked in [Documentation](#documentation).

### Full Stage History

| Stage | Scope and result | Current status |
| :--- | :--- | :--- |
| **A–D — Toolchain and translation foundation** | Set up the Windows/LLVM build, pinned ReXGlue, inspected the Xbox 360 XEX, and established strict PowerPC-to-C++ code generation. | Complete; historical verification. |
| **E–F — Runtime and subsystem bring-up** | Initialized guest memory, CRT/kernel services, VFS mounts, D3D12/Xenos graphics, SDL audio, and package loading. Iteratively resolved startup panics caused by missing indirect-call targets. | Complete; historical verification. |
| **G — First visual frame** | Brought the game to a visible, non-black Unreal Engine 3 frame in the host window. | Complete; historical verification. |
| **H — Attract mode and 3D battles** | Reached rendered in-engine matches with lighting, skeletal animation, effects, and multichannel audio. | Complete; historical verification. |
| **I — Combat, graphics performance, and input** | Isolated first-hit stutter to D3D12 PSO compilation, persisted and warmed the pipeline cache, selected the high-performance GPU, added presentation pacing, and enabled keyboard/mouse input. | Complete; historical verification. |
| **J1/J2 — Movies and story cinematics** | Cataloged 119 WMV assets and verified in-engine logo playback plus the `MK001.wmv` and `dc001.wmv` Story Mode introductions using the Xenon movie path. | Complete; historical verification. |
| **K — Save and profile persistence** | Implemented local content-container handling under `./savedata/` and verified settings/profile restoration after a cold restart. | Complete; historical verification. |
| **K3 — Dark Kahn investigation** | Traced selection, player setup, script/move data, and mesh/material loading. Documented why a drop-in package replacement was unsafe and parked that route. | Investigation complete; replacement route parked. |
| **L — Mod framework** | Added non-destructive VFS overlays, mod discovery and priority, TOML configuration, runtime hooks, and post-build deployment. | Implemented; startup and mod checks rerun during this audit. |
| **M/M+ — Dark Kahn mod** | Added selection aliases and package/mesh/audio/effect hooks. Current captures show Dark Kahn on the versus screen and as the Player 2 combat mesh; they do not show the select screen after Dark Kahn is chosen. Announcer playback and particle attachment remain open. | Partially verified; active work. |
| **N — Stock compatibility** | A prior full-combat stock regression run is recorded as passing. | Historical verification; rerun the affected paths after runtime changes. |
| **O — Multiplayer/netplay** | Identified networking stubs and recorded a possible foundation. | Proposal only; not implemented. |

### Audit Remediation Stages

| Audit stage | Changes made | Evidence / remaining limits |
| :--- | :--- | :--- |
| **0 — Preserve baseline** | Preserved existing project and SDK edits; confirmed the pinned ReXGlue base and captured current state. | Complete. No existing working-tree changes were discarded. |
| **1 — Reproducible SDK and code generation** | Added `patches/rexglue-sdk.patch` and its guarded apply script; tracked generated callback edits and added a verifier/post-codegen script; documented the configure/codegen/reconfigure sequence; staged the Xenos GPU plugin and required SSSE3/ImGui build settings. | Clean pinned-SDK application and idempotent reapplication passed. Fresh codegen verified 36 callback insertions and built the executable, runtime, and GPU plugin. Generated game code and proprietary assets stay untracked. |
| **2 — Runtime safety and configuration correctness** | Made ModManager reads return synchronized snapshots; made config writes use a temporary file and report failures; preserved runtime `config/mods.toml` during post-build deployment; scoped mesh fallback to the current thread/load; serialized shared package/audio guest scratch use and ordered boss-select reset with enqueue; initialized immutable mesh/HUD/player-data strings and the synthetic-costume record once in `ApplyHooks`; respected explicit false flags; documented restart-required overlay changes; rejected VFS file/directory collisions; fixed auto-continue, exact test-button parsing, and present-interval clamping. | The latest full codegen/build passed after the scratch initialization and deployment changes. The deployment fixture confirmed user config is preserved and missing config is seeded. The mod check passed with automatic and explicit build selection against available startup logs, but no new Practice run followed these final changes. Package fallback was not reached; variable voice-name ring reuse and package-object lifetime still need runtime review. |
| **3 — Tooling, regression checks, and documentation** | Made `tests/test_modding.py` use checkout-relative paths, select the newest build logs by default (or honor `MKVDC_BUILD_DIR`), load the effective config/manifests used by that run, and restrict telemetry checks to the latest startup; made vtable harvesting emit a separate candidate file by default and added a conflict-checking merge tool; reconciled current status across the docs. | The mod check passed after the latest build with automatic log selection and with an explicit audit-build override. A stale-telemetry check passed. The vtable merge utility has not been run. |
| **4 — Phase M+ package investigation** | Inspected UI, character, audio, and FX package exports; distinguished the default P2 select preview, the later versus screen, and the combat mesh; removed ineffective body-art and cue rewrite attempts; recorded candidate sockets and effects. | Logs confirm the head-name alias hook ran, and captures show Dark Kahn on the versus screen and in combat. The selected-state select-screen art, announcer playback, and emitter attachment are not verified. No package repacker or proprietary package is included. |
| **5 — Regression and distribution gate** | Rebuilt after fresh codegen, ran Practice and the mod startup check, and recorded remaining checks. | Latest fresh codegen, hook verification, full build, deployment fixture, and automatic/explicit mod checks passed. Practice evidence predates the last scratch and deployment edits, so a new gameplay run is still needed. Stock movie/input/save/graphics regression paths, full Dark Kahn presentation/audio/FX, and a release config without the development marker remain open. |

### 2026-09-25–26 Audit Change Summary

The audit changed project build and reproducibility files (`CMakeLists.txt`,
presets, generated-hook tooling, and the tracked SDK patch); runtime and mod
management code (`src/mkvdc_app.h`, `src/mod_manager.*`, mod configuration and
manifest); and the modding test, vtable tools, README, and audit documentation.
The SDK patch is applied to a separate pinned SDK checkout. Generated C++ and
local game inputs are build inputs, not source files committed by this project.

The regenerated project build completed in
`out/build/audit-win-amd64-release/`, producing `mkvdc.exe`, `rexruntime.dll`,
and `rexgpu-xenos.dll`. Fresh code generation was byte-identical across 219
generated C++ translation units in the main checkout. The current Practice
capture and paired logs are in the local, untracked evidence directory
`scratch/practice_runs/audit_20260926_phase_m_asset/`.
The screenshot sequence shows the default Batman preview in
`4_practice_p2_select.png`, then Dark Kahn on the later versus screen in
`5_practice_p2_boss.png`, and Dark Kahn responding to a hit in Practice. The
sequence does not capture the select screen after choosing Dark Kahn. Logs show
the head-name alias hook and Dark Kahn voice/SFX package redirects; they also
show the FX package being linked. Those events do not by themselves prove the
selected-state art, audio playback, or particle attachment.

After that capture, the runtime audit moved immutable mesh, HUD, empty-string,
and player-data guest strings plus the synthetic costume pointer record into
one-time initialization in `ModManager::ApplyHooks`. The application rebuilt
and the log-based mod check passed; an interactive Practice run after this
last change is still needed. Per-call voice-name ring buffers and the captured
package-object pointer lifetime remain open review items.

Post-build deployment now refreshes static config and bundled mod assets while
preserving an existing runtime `config/mods.toml`; the default file is copied
only when missing. A focused deployment fixture verified both preservation and
first-run seeding. Fresh codegen/build and both mod-check invocations passed
after this deployment change.

Earlier command history and reverse-engineering evidence are maintained in
[`docs/devlog.md`](docs/devlog.md) and
[`docs/analysis-ledger.md`](docs/analysis-ledger.md). Dark Kahn's original
character-injection investigation is in
[`docs/PHASE_K3_DARK_KAHN_INVESTIGATION.md`](docs/PHASE_K3_DARK_KAHN_INVESTIGATION.md).

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
- **Toolchain**: LLVM Clang 22.1.8 targeting `x86_64-pc-windows-msvc`, with the MSVC-compatible runtime and Windows SDK. The current audit build was verified in Visual Studio 2026 with MSVC 14.51.
- **Build System**: CMake (>= 3.25) and Ninja. The current audit used CMake 4.3.1 and Ninja 1.13.2.
- **Python**: Python 3.11+ (or Python 3.10 with `tomli`; `capstone` is required for vtable harvesting).
- **SDK**: ReXGlue SDK (pinned commit `c94f5eb`).

### Build Steps

```powershell
# 1. Clone the project and pinned SDK beside it
git clone https://github.com/VergilMorvx/MKVSDC-Resurrektion.git
cd MKVSDC-Resurrektion
git clone https://github.com/rexglue/rexglue-sdk.git ..\rexglue-sdk
git -C ..\rexglue-sdk checkout c94f5ebdcb3c9d1a460ca48e04f9758448f8d518
git -C ..\rexglue-sdk submodule update --init --recursive

# Apply the complete, project-owned SDK change set. This refuses unexpected local edits.
python scripts\apply_rexglue_sdk_patch.py --sdk-dir ..\rexglue-sdk

# Point mkvdc_manifest.toml at your own extracted game directory and default.xex.

# 2. Configure and run codegen once so CMake can discover generated sources
cmake --preset win-amd64-release
cmake --build --preset win-amd64-release --target mkvdc_codegen

# Reconfigure after codegen, then build the executable
cmake --preset win-amd64-release
cmake --build --preset win-amd64-release

```

The preset uses `../rexglue-sdk` by default. Set `REXSDK_DIR` in the CMake cache to use another checkout. The first configure and codegen pass requires the game paths in `mkvdc_manifest.toml` to point to files on your machine. CMake reapplies the tracked callback patch after codegen; running `rexglue codegen` manually requires `python scripts/apply_generated_hooks.py` afterward.

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
- [`docs/MODDING_GUIDE.md`](docs/MODDING_GUIDE.md): Mod layout, manifests, configuration, VFS priority, and ModManager API.
- [`docs/PHASE_K3_DARK_KAHN_INVESTIGATION.md`](docs/PHASE_K3_DARK_KAHN_INVESTIGATION.md): Dark Kahn character-loading investigation and parked replacement approach.
- [`docs/wmv_status.md`](docs/wmv_status.md): Complete audit and status matrix for all 119 WMV video assets.
- [`docs/wmv_ledger.json`](docs/wmv_ledger.json): Machine-readable catalog containing hashes, dimensions, durations, and bitrates of all game cutscenes.

---

## Disclaimer & Copyright

**MKVSDC: Resurrektion** does **NOT** distribute any proprietary or copyrighted game assets. No textures, 3D models, audio files, game scripts, pre-rendered movies (`.wmv`), or original executable binaries (`.xex`, `.xxx`) are included in this repository.

To build and run the game, users must provide their own legitimately obtained copy of *Mortal Kombat vs. DC Universe* for the Xbox 360. All trademarks, character names, logos, and game assets are property of Warner Bros. Interactive Entertainment, NetherRealm Studios, DC Comics, and Midway Games.
