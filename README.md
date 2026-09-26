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
| **Phase M / M+: Dark Kahn** | **PARTIALLY VERIFIED** | Current captures show Dark Kahn selected with matching large art, fighting as Player 1 in Arcade and Player 2 in Practice, and responding to combat input. Voice/SFX/foley package redirects ran; direct announcer playback and a Dark Kahn particle emitter remain unverified. |
| **Audit remediation** | **IN PROGRESS** | SDK Debug/Release and project Release builds pass; full CTest passes 1,677 tests per SDK configuration (1,673 pass and four existing conditional skips). Fresh codegen, 36 hook checks, Player 1 Arcade and Player 2 Practice runs, both mod-check modes, and clean deployment staging pass. Older-milestone regressions, direct audio proof, and FX attachment remain open. |

Phases G–K summarize earlier milestone evidence. The current audit reran
fresh Release codegen, the complete SDK test suite in Debug and Release, an
Arcade menu-to-combat run, a Player 2 Practice run, and clean deployment
staging checks against the current app build. See the current capture sequences
under `scratch/phase_m/audit_20260926_sdk_xex_fix/` and
`scratch/practice_runs/audit_20260926_release_p2_smoke2/`.

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
| **M/M+ — Dark Kahn mod** | Added selection aliases and package/mesh/audio/effect hooks. Current captures show Dark Kahn selected with matching large art, fighting as Player 1 in Arcade and Player 2 in Practice, and responding to combat input. Announcer playback and particle attachment remain open. | Partially verified; active work. |
| **N — Stock compatibility** | A prior full-combat stock regression run is recorded as passing. | Historical verification; rerun the affected paths after runtime changes. |
| **O — Multiplayer/netplay** | Identified networking stubs and recorded a possible foundation. | Proposal only; not implemented. |

### Audit Remediation Stages

| Audit stage | Changes made | Evidence / remaining limits |
| :--- | :--- | :--- |
| **0 — Preserve baseline** | Preserved existing project and SDK edits; confirmed the pinned ReXGlue base and captured current state. | Complete. No existing working-tree changes were discarded. |
| **1 — Reproducible SDK and code generation** | Added `patches/rexglue-sdk.patch` and its guarded apply script; tracked generated callback edits and added a verifier/post-codegen script; documented the configure/codegen/reconfigure sequence; staged the Xenos GPU plugin and required SSSE3/ImGui build settings. | Clean pinned-SDK application and idempotent reapplication passed. Fresh codegen verified 36 callback insertions and built the executable, runtime, and GPU plugin. Generated game code and proprietary assets stay untracked. |
| **2 — Runtime safety and configuration correctness** | Made ModManager reads return synchronized snapshots; made config writes use a temporary file and report failures; preserved runtime `config/mods.toml` during post-build deployment; retained preferences for temporarily undiscovered mods; range-checked TOML priorities and quoted saved mod IDs; aligned manifest metadata defaults with the guide; rejected linked overlay roots, skipped symlinks and Windows reparse points in host indexing, lazy lookup, and overlays, and capped VFS traversal at 128 directory levels; scoped mesh fallback to the current thread/load; serialized shared package/audio guest scratch use and ordered boss-select reset with enqueue; initialized immutable mesh/HUD/player-data strings, fixed voice/SFX/foley package names, and the synthetic-costume record once in `ApplyHooks`; respected explicit false flags; documented restart-required overlay changes; rejected VFS file/directory collisions; fixed auto-continue, exact test-button parsing, and present-interval clamping; initialized D3D12 texture-copy layout values, fixed a dangling XAM launch-path view, fixed signed scissor bounds and `RtlCompareStringN`'s `0xFFFF` length handling; bounded XEX image/header/import/PE ranges; and prevalidated complete LZX delta streams with separate source and destination bounds. | Debug and Release SDK builds/install and CTest pass 1,677 tests each: 1,673 pass and four existing conditional skips (219 unit cases, 1,458 PPC cases). Fresh codegen loads the real `default.xex`, the module is up to date, and all 36 generated hook calls verify. The project Release build passes and deployed/installed runtime DLL hashes match. Deployment fixtures, the clean-stage runtime smoke, and both automatic/explicit `tests/test_modding.py` checks pass. Player 1 Arcade and Player 2 Practice runs reach combat; the Arcade run also reaches pause. Five `\Device` VFS probes are non-fatal. Announcer playback, FX attachment, package fallback, dynamic cue-buffer lifetime, longer-term package-object lifetime, and full installer/archive validation remain open. |
| **3 — Tooling, regression checks, and documentation** | Made `tests/test_modding.py` use checkout-relative paths, select the newest build logs by default (or honor `MKVDC_BUILD_DIR`), load the effective config/manifests used by that run, and restrict telemetry checks to the latest startup; made vtable harvesting emit a separate candidate file by default, refuse overwriting the canonical config, and added a conflict-checking merge tool; made the Phase M harness select the current audit build by default, accept build overrides, and refuse to overwrite prior evidence; added Windows SDK tests for linked roots, overlay descendants, and lazy base-path lookups, plus overflow boundary tests for XEX byte ranges; fixed the SDK unit-test include path and depfile test's invalid C++ escape; made the SDK patch verifier capture added files through a temporary index; cleaned unsafe pointer-bearing texture resets and SDK initialization-order warnings; made sentinel comparisons explicit; refreshed the SDK patch; and reconciled current status across the docs and mod/config path precedence in the guide. | Both automatic and explicit-build mod checks pass against the latest interactive startup logs. Synthetic enabled/disabled config fixtures and stale-telemetry checks passed. The vtable scan found 0 unregistered functions; synthetic merge/conflict checks passed without changing the canonical config. Debug and Release full CTest each passed 1,677 tests with four existing conditional skips; a synthetic clean-apply check confirmed the patch verifier detects untracked additions without changing the repository index. The new harness captured the selected Dark Kahn state, Player 1 combat, and pause. Direct announcer audio and emitter attachment remain open. |
| **4 — Phase M+ package investigation** | Inspected UI, character, audio, and FX package exports; distinguished the default P2 preview from the selected Dark Kahn screen, versus screen, and combat mesh; removed ineffective body-art and cue rewrite attempts; recorded candidate sockets and effects. | The latest capture shows the `DARK KAHN` selection label and matching large art after the boss-slot toggle; the following fight selects Dark Kahn as Player 1. The mesh redirect succeeds. Announcer playback and emitter attachment are not verified. No package repacker or proprietary package is included. |
| **5 — Regression and distribution gate** | Rebuilt after fresh codegen, ran the full SDK Debug/Release CTest suites, rebuilt the project Release target, ran automatic and explicit mod checks, captured Player 1 Arcade and Player 2 Practice combat, and exercised clean deployment staging plus a 40-second startup smoke. | Selected-state art, both player slots, test-button combat input, and pause are verified. A clean staging fixture seeds `test_mod=false`; an existing user override survives; the staged app starts without mounting the test marker. Startup logos were captured, but Story Mode playback, cold-restart save persistence, a confirmed keyboard/mouse response, and measured frame pacing remain unverified. The project Release build has zero registered CTest tests; `tests/test_modding.py` is run directly. No installer/archive target exists. Direct announcer audio and FX attachment remain open. |

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
generated C++ translation units in the main checkout. The earlier Player 2 Practice
capture and paired logs are in the local, untracked evidence directory
`scratch/practice_runs/audit_20260926_phase_m_asset/`.
The screenshot sequence shows the default Batman preview in
`4_practice_p2_select.png`, then Dark Kahn on the later versus screen in
`5_practice_p2_boss.png`, and Dark Kahn responding to a hit in Practice. The
sequence did not capture the select screen after choosing Dark Kahn; the newer
current-build selection capture is recorded below. Logs show
the head-name alias hook and Dark Kahn voice/SFX package redirects; they also
show the FX package being linked. Those events do not by themselves prove the
selected-state art, audio playback, or particle attachment.

After that capture, the runtime audit moved immutable mesh, HUD, empty-string,
and player-data guest strings plus the synthetic costume pointer record into
one-time initialization in `ModManager::ApplyHooks`. The application rebuilt,
and an earlier 35-second hidden startup smoke reached UI-package requests; both
automatic and explicit-build mod checks passed against its paired logs at that
checkpoint. The latest interactive run is recorded below. Per-call
voice-name ring buffers and the captured package-object pointer lifetime
remain open review items.

### Latest Audit Checkpoint — 2026-09-26

The XEX/LZX audit now bounds compressed headers, blocks, chunks, decompressed
output, import tables, PE headers, and patch source/destination extents. Delta
records are validated as a complete stream before applying writes. The first
fresh Release codegen run caught an over-strict PE section check against the
aggregate XEX page count; section and exception-directory address checks now
use 64-bit guest-heap bounds while PE header/table reads remain image-bounded.
The real `default.xex` loads and codegen reports the module up to date.

The SDK patch was refreshed and both its exact-match verifier and reverse-apply
check pass. Debug and Release builds/install pass, and full CTest passes 1,677
tests per configuration: 1,673 pass and four existing conditional BitStream
cases skip. Each run includes 219 unit cases (215 pass, four skip) and 1,458 PPC
cases. The project Release target rebuilds, all 36 generated hook sites verify,
and installed/deployed `rexruntime.dll` hashes match.

The fresh capture at
`scratch/phase_m/audit_20260926_sdk_xex_fix/` shows Dark Kahn's selected art and
name after the boss-slot toggle, then Player 1 Dark Kahn in combat and pause.
The second current-build run at
`scratch/practice_runs/audit_20260926_release_p2_smoke2/` shows Player 2 Dark
Kahn in Practice responding to a hit. `tests/test_modding.py` passes with
automatic build selection and an explicit `MKVDC_BUILD_DIR`. Sound-package
redirects ran, but there is no direct announcer recording; `FX_DarkKahn` linked,
but no emitter was attached.

The current Release startup capture under
`scratch/phase_gj_current_20260926_startup/` records six frames from 2 through
40 seconds, including game and studio logo animation. The app remained alive
until the capture harness terminated it. A focused Enter/Space attempt during
the intro did not produce a confirmed menu transition; keyboard behavior stays
open. Current Release `ctest -N` lists zero project tests, so the app-level
configuration check is the direct `tests/test_modding.py` run.

The Phase M and Practice capture harnesses now default to the audit Release
build, accept build/executable/capture overrides, and refuse to overwrite
existing evidence. Clean deployment fixtures verified that `test_mod=false`
is seeded while a preexisting user override remains unchanged. A 40-second
staged-app startup stayed alive, and the log checker confirmed the marker was
not mounted. Remaining work is direct selection/victory audio verification, FX
binding, cue scratch/package lifetime review, Story Mode movie playback,
cold-restart save persistence, confirmed keyboard/mouse response, and measured
frame pacing. There is currently no installer/archive target to validate.

Focused SDK tests now exercise rejection of a linked overlay root, skipping
linked overlay descendants, and refusal of linked descendants in the base tree
and its lazy lookup path. On Windows they create junctions first to exercise
the reparse-point guard, then fall back to directory symlinks if junction
creation fails. The latest SDK unit suite reports 219 cases in each
configuration; 215 passed and four existing bitstream cases were skipped by
their test conditions, with no failures. The suite also exposed and fixed an
invalid escape in an existing depfile assertion. A focused range-helper test
covers exact, empty, out-of-bounds, and overflow boundaries for XEX byte
ranges. The refreshed SDK patch matches the pinned SDK checkout exactly. Its
apply verifier uses a temporary index, preserving the SDK checkout's real
index while comparing added files.

The follow-up Release warning review found a real lifetime error in
`XamLoaderLaunchTitle`: a temporary joined path had been assigned to a
`std::string_view`. The path now uses owned storage before launch state is
updated. It also initialized the D3D12 base texture footprint and size, changed
texture-binding reset to assign null pointers explicitly, aligned render-target
member initialization, and clarified heap defaults. Debug and Release SDK
install builds and the 219-case suites pass after the fixes, and the project
Release target was rebuilt. The broad SDK build still reports unrelated
warnings in emulation stubs and aggregate initialization; they are recorded in
the audit report rather than hidden by blanket warning suppression. The
signed/unsigned pass removed a negative-scissor conversion edge case and made
sentinel and member initialization order explicit. Follow-up parsing review
fixed `RtlCompareStringN`'s `0xFFFF` length handling and bounded XEX compressed
header, block, chunk, and output ranges. The current XEX paths compile and pass
the SDK suite, but malformed-XEX integration fixtures are not yet present.

The next config review found unchecked 64-bit TOML priorities being narrowed to
`int`, and config saves dropping preferences for mods that are temporarily not
installed. Priority values are now range-checked and invalid values are logged
and ignored; absent-mod enable/priority settings are retained across saves.
Saved mod IDs are emitted as escaped TOML quoted keys so punctuation and control
characters cannot break the configuration file. A later hidden startup smoke
and the fresh interactive run exercised config loading; automatic and explicit
mod checks pass against the fresh run's paired logs.

The audio package hook now selects one of three fixed, immutable guest strings
for Dark Kahn voice, SFX, and cape-foley packages instead of mutating a reused
four-slot ring. Variable announcer cue names still use per-call scratch slots
and need runtime lifetime review.

The VFS audit found that recursive asset mounting followed linked paths and
could recurse deeply enough to exhaust the stack. The pinned SDK now rejects a
linked `assets/` root, skips symlinks and Windows reparse points beneath it, and
stops directory traversal beyond 128 levels. The SDK patch was regenerated and
its apply verifier confirmed an exact match; Debug and Release SDK build/install
steps passed. Review also found that
the directory named `audit-win-amd64-release` was configured as Debug; it is now
reconfigured as true Release, with all 219 generated translation units, the app,
and deployment step rebuilt. The mod checker and Python syntax checks passed
against captured logs available at that earlier checkpoint. The later 35-second
hidden startup smoke reached UI-package requests, and automatic plus explicit
mod checks pass against its paired latest logs. A fresh menu-to-combat run now
exercises the current config and overlay paths; it did not include linked
overlay assets.

Post-build deployment now refreshes static config and bundled mod assets while
preserving an existing runtime `config/mods.toml`; the default file is copied
only when missing. The tracked default disables the development `test_mod`
marker. Deployment fixtures verified preservation of an explicit user opt-in,
first-run seeding with the marker disabled, and continued copying of the marker
asset. The log checker also passed against synthetic disabled-mod startup logs.
Fresh codegen/build and both real-log mod-check invocations passed. The latest
menu-to-combat run confirms Dark Kahn selection and combat; clean distribution
staging and the affected movie/input/save/graphics regressions remain pending.

The modding guide now documents the verified path order and duplicate-ID rule:
working-directory mods, executable-directory mods, then Windows per-user mods;
later copies replace earlier copies with the same ID. Config lookup uses the
working directory first and the executable directory as a Windows fallback.

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
- [`docs/AUDIT_REMEDIATION.md`](docs/AUDIT_REMEDIATION.md): Phased codebase audit, fixes, verification evidence, and remaining work.
- [`docs/MODDING_GUIDE.md`](docs/MODDING_GUIDE.md): Mod layout, manifests, configuration, VFS priority, and ModManager API.
- [`docs/PHASE_K3_DARK_KAHN_INVESTIGATION.md`](docs/PHASE_K3_DARK_KAHN_INVESTIGATION.md): Dark Kahn character-loading investigation and parked replacement approach.
- [`docs/PHASE_M_ASSET_AUDIT.md`](docs/PHASE_M_ASSET_AUDIT.md): Current Dark Kahn UI, character, audio, and effects package inspection.
- [`docs/wmv_status.md`](docs/wmv_status.md): Complete audit and status matrix for all 119 WMV video assets.
- [`docs/wmv_ledger.json`](docs/wmv_ledger.json): Machine-readable catalog containing hashes, dimensions, durations, and bitrates of all game cutscenes.

---

## Disclaimer & Copyright

**MKVSDC: Resurrektion** does **NOT** distribute any proprietary or copyrighted game assets. No textures, 3D models, audio files, game scripts, pre-rendered movies (`.wmv`), or original executable binaries (`.xex`, `.xxx`) are included in this repository.

To build and run the game, users must provide their own legitimately obtained copy of *Mortal Kombat vs. DC Universe* for the Xbox 360. All trademarks, character names, logos, and game assets are property of Warner Bros. Interactive Entertainment, NetherRealm Studios, DC Comics, and Midway Games.
