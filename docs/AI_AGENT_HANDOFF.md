# MKVSDC: Resurrektion — Engineering Handoff & Technical Specification

> **Canonical Engineering Handoff Document**  
> **Status:** Active Reference & Next-Agent Specification  
> **Repository:** `D:\mkvsdc\mkvsdc-recomp`  
> **Current Git Commit:** `12b4f36`  
> **Branch:** `master`  
> **Target Executable:** `out/build/win-amd64-release/mkvdc.exe`  

---

## 1. Project Identity

* **Project Name:** MKVSDC: Resurrektion
* **Original Title:** *Mortal Kombat vs. DC Universe* (Xbox 360, 2008)
* **Title ID:** `4D5707E9` | **Media ID:** `6153914C` | **Executable:** `default.xex`
* **Original Platform:** Xbox 360 (PowerPC Xenon, AMD Xenos GPU, Unreal Engine 3 modified)
* **Target Platform:** Windows 10/11 x86-64 (Direct3D 12, SDL 3.5.0, Windows SDK 10)
* **Core Architecture:** Static Ahead-of-Time (AOT) binary recompilation via **ReXGlue SDK** (v0.10.0.2-dev), translating Xenon PPC machine code into native C++23 translation units executed directly on the host CPU and GPU without emulation overhead.

---

## 2. Current Verified State

### Verified Working Subsystems (Empirically Validated)
* **CPU Execution & CRT Initialization:** Clean translation of all 66,546 original functions; all 6 strict analysis failures and 42 unmapped virtual leaf accessors/thunks resolved.
* **Rendering & Direct3D 12:** Flawless in-engine real-time 3D combat rendering on NVIDIA RTX 4060 Laptop GPU (and Intel Iris Xe fallback) at 1280x720 / 60 FPS presentation lock with full dynamic lighting, specular shaders, blood particle effects, arena physics, and cloth simulation.
* **Audio Streaming:** Multi-channel 5.1 surround sound streaming via SDL 3.5.0 and host audio endpoints.
* **Input Emulation:** Seamless keyboard/mouse controller emulation (`mnk_mode = true`) mapped to Player 1, alongside automated headless button injection (`test_buttons.txt`).
* **Cinematic Video Subsystem (Phase J):** In-engine WMV playback verified bit-for-bit for studio logos (`midway_logo.wmv`, `WB_Logo.wmv`, `DC_Logo.wmv`) and high-bandwidth story mode cutscenes (`MK001.wmv`, `dc001.wmv`) using Xenon DXVA hardware decode shaders.
* **In-Game Save Persistence (Phase K):** STFS container creation (`CREATE_ALWAYS`), header serialization, and persistent reload (`OPEN_EXISTING`) into `./savedata` verified across process restarts (options changes persist).
* **Boss Unlocking (Phase K3/L):** Shao Kahn and Darkseid are unlocked and fully playable on the roster via non-destructive hooks in `src/mod_manager.cpp`. Matches load cleanly with zero crashes.
* **Modding Subsystem (Phase L):** Complete non-destructive VFS overlay system with multi-directory discovery, priority-based conflict resolution, TOML manifests, central overrides (`config/mods.toml`), runtime safe guards, and automated test suite (`tests/test_modding.py`).

### What Does NOT Work / Is Parked / Unverified
* **Dark Kahn Custom Mesh (Parked):** Standalone `CHAR_DarkKahn.xxx` replacement failed because `CHAR_DarkKahn.xxx` lacks player script objects (it is an NPC boss package), causing a NULL pointer dereference in character initialization. Visually rendered boss currently uses stock Shao Kahn's model until Phase M mesh injection is completed.
* **Network Multiplayer (Phase O - Untested):** Online Xbox Live / GameSpy multiplayer is currently unmapped/untested. ReXGlue has socket stubs in `xam_net.cpp`, but netplay requires investigation and recreation.
* **Local 2-Player (Partially Untested):** Player 1 is fully operational via MnK/XInput; Player 2 controller mapping and versus flow have not been exhaustively tested.

---

## 3. Build & Development Environment

### Host Machine Specifications
* **Operating System:** Windows 11 (AMD64)
* **Compiler:** Clang 22.1.8 (LLVM) with C++23 standard support (`-std=c++23`)
* **Build System:** CMake 3.25+ with Ninja 1.13.2 generator
* **SDK Path:** `D:\mkvsdc\rexglue-sdk\out\install\win-amd64`
* **Game Assets Root:** `D:\mkvsdc\MKvDC_Extracted\Mortal Kombat vs. DC Universe (World) (En,Fr,De,Es,It)`
* **Decrypted Mapped Binary:** `D:\mkvsdc\default_mapped.bin` (Virtual base: `0x82000000`, size: `0x1130000`)

### Build Commands
To build the project in the established environment, run:

```bat
cmd.exe /c "D:\mkvsdc\run_in_env.bat ninja -C out\build\win-amd64-release"
```

* **Build Output Directory:** `D:\mkvsdc\mkvsdc-recomp\out\build\win-amd64-release`
* **Primary Executable:** `out\build\win-amd64-release\mkvdc.exe`
* **Automated Post-Build:** `CMakeLists.txt` automatically syncs `config/` and `mods/` directories to the build directory upon successful link.

> [!WARNING]
> **Windows Command Rules (Mandatory):**
> * NEVER invoke `Remove-Item` directly as a standalone executable in commands.
> * For deleting files, always use: `cmd.exe /d /c del /f /q "PATH"`
> * For deleting directories, always use: `cmd.exe /d /c rmdir /s /q "PATH"`
> * NEVER commit proprietary game files (`.xxx`, `.bin`, `.wmv`, `.xex`) to Git.

---

## 4. Architecture Overview

```
+---------------------------------------------------------------+
|                       Host Executable                         |
|                 (mkvdc.exe, Windows x86-64)                   |
+-------------------------------+-------------------------------+
                                |
        +-----------------------+-----------------------+
        |                                               |
+-------v-----------------------+       +---------------v---------------+
|     ReXGlue Recomp Runtime    |       |      MKVSDC Mod Subsystem     |
| - PPC Memory Arenas           |       | - ModManager (Singleton)      |
|   (0x100000000 / 0x200000000) |       | - Multi-Directory Scanner     |
| - FunctionDispatcher          |       | - VFS Overlay Shadowing       |
| - VirtualFileSystem (VFS)     |       | - Hook Interceptors           |
| - D3D12/DXGI Pipeline         |       | - config/mods.toml Override   |
| - SDL 3.5.0 Audio             |       +-------------------------------+
+-------------------------------+
```

### 1. PowerPC to C++ Translation
* The game's PPC binary (`default.xex`) was disassembled and recompiled into 443 native C++ translation units (`mkvdc_recomp.*.cpp`) registered via `PPCFuncMappings` in `generated/default/mkvdc_init.cpp`.
* Functions are called via `PPCContext` register state (`ctx.r3`, `ctx.r4`, `ctx.lr`, etc.) over a mapped 4 GB guest memory base (`uint8_t* base`).

### 2. Runtime Guest Hooks & Dispatcher
* `rex::runtime::FunctionDispatcher` manages the function lookup table.
* Native C++ hooks intercept critical guest addresses at startup:
  * `0x826AC700`: Move table bound safety guard (prevents crashes from invalid move array indices).
  * `0x82634318`: Boss selection lock check (`r3 = 0` unlocks Shao Kahn and Darkseid).
  * `0x82680EB0`: Character availability query override.

### 3. Virtual File System (VFS) Overlays
* ReXGlue mounts the extracted game files under `\Device\Harddisk0\Partition1`.
* `HostPathDevice::AddOverlay(path)` dynamically shadows entries in the VFS tree. When a mod provides an asset matching a stock relative path (e.g. `Config/Coalesced.ini`), the VFS updates its internal `host_path_` pointer without touching the base game files.

---

## 5. Repository Map

```
d:/mkvsdc/mkvsdc-recomp/
├── CMakeLists.txt              # Build definitions, C++23 setup, post-build mod sync
├── mkvdc_manifest.toml         # ReXGlue recompilation manifest and function configs
├── config/
│   ├── functions.toml          # Manual function boundaries and gapfill overrides
│   ├── gnatives.toml           # UnrealScript native bytecode page dispatchers
│   ├── vtables.toml            # Harvested class virtual method table entries
│   ├── dispatch_stubs.toml     # Virtual method forwarders and COM dispatch stubs
│   └── mods.toml               # Central mod toggles and load priority overrides
├── docs/
│   ├── AI_AGENT_HANDOFF.md     # THIS CANONICAL HANDOFF SPECIFICATION
│   ├── MODDING_GUIDE.md        # Comprehensive mod authoring guide & API specification
│   ├── PHASE_K3_DARK_KAHN_INVESTIGATION.md # Deep root cause analysis of Dark Kahn crash
│   ├── devlog.md               # Historical engineering log of all milestones
│   └── wmv_status.md           # 119-movie inventory and verification matrix
├── mods/
│   ├── BossUnlock/             # Stock boss character unlock mod
│   │   └── mod.toml
│   ├── DarkKahn/               # Dark Kahn prototype mod (Parked for Phase M)
│   │   ├── mod.toml
│   │   └── assets/Config/
│   └── TestMod/                # Lightweight test mod verifying VFS overlay functionality
│       ├── mod.toml
│       └── assets/Config/test_mod_marker.txt
├── src/
│   ├── main.cpp                # Host application entry point & GPU power exports
│   ├── mkvdc_app.h             # App lifecycle, telemetry, VFS paths, guest page permissions
│   ├── mod_manager.h           # ModManager public interface & manifest structures
│   └── mod_manager.cpp         # Mod discovery, priority sorting, hooks, VFS overlays
└── tests/
    └── test_modding.py         # Automated verification test suite for Phase L
```

---

## 6. Phase History & Status Ledger

| Phase | Description | Status | Evidence / Commits |
|---|---|---|---|
| **Phase A–D** | Toolchain, ReXGlue SDK build, XEX analysis, codegen | **VERIFIED** | Commit `03f916f` |
| **Phase E–F** | Subsystem bringup: CRT, D3D12, SDL audio, VTables | **VERIFIED** | Commit `a3d84ca` |
| **Phase G–H** | Real-time 3D combat rendering, 60 FPS lock, RTX 4060 | **VERIFIED** | Commit `3b85aff` |
| **Phase J1** | In-engine WMV logo movie playback (Midway, WB, DC) | **VERIFIED** | Commit `afa12bc` |
| **Phase J2** | In-engine Story Mode WMV cutscenes (`MK001`, `dc001`) | **VERIFIED** | Commit `1f7379b` |
| **Phase K** | Local save persistence, STFS containers (`./savedata`) | **VERIFIED** | Commit `1275ba8` |
| **Phase K3** | Dark Kahn character investigation & root cause discovery | **VERIFIED / PARKED** | `docs/PHASE_K3_DARK_KAHN_INVESTIGATION.md` |
| **Phase L** | Non-destructive VFS Modding Architecture & ModManager | **VERIFIED** | Commits `b635526`, `12b4f36` |
| **Phase M** | Custom Character / Skeletal Mesh Injection (Dark Kahn) | **PENDING** | *Next Recommended Action* |
| **Phase N** | Mod Compatibility & Stock Regression Verification | **VERIFIED** | Full combat match test passed (Run 139) |
| **Phase O** | Multiplayer / Netplay Foundation | **PROPOSAL** | Sockets stubs identified in `xam_net.cpp` |

---

## 7. Deep Dive: Dark Kahn & Phase K3 Investigation

### The Objective
Make Dark Kahn playable on the character roster.

### What Was Attempted
* Unlocked the boss selection slot via `sub_82634318` returning `r3 = 0`. This allowed selecting Shao Kahn / Darkseid on the character selection grid.
* Attempted drop-in package replacement by creating `mods/DarkKahn/assets/Asset/CHAR_ShaoKahn.xxx` as a direct copy of `CHAR_DarkKahn.xxx`.

### Why Drop-In Replacement Failed (Root Cause Discovery)
1. **Package Content Difference:**
   * Playable characters (`CHAR_ShaoKahn.xxx`, `CHAR_SubZero.xxx`) contain **70 to 95 script/player data objects**, including `char_shaokahn_player_data`.
   * `CHAR_DarkKahn.xxx` is strictly an NPC Story Mode boss package. It contains **0 player script objects**.
2. **The Exact Crash:**
   * During match loading, character initialization function `sub_826A46E0` looks up the character's player data table inside the loaded package.
   * Because `CHAR_DarkKahn.xxx` contains no player data, `sub_826A46E0` returned `NULL` (`0x00000000`).
   * UE3 character pawn initialization subsequently attempted to dereference `*(pstruct + 128)` (CostumeData pointer), resulting in an immediate guest memory read violation (`0x7ff63d102a65` dereferencing address `0x00000080`).
3. **Skeletal Rig Compatibility:**
   * Reverse engineering revealed that both Shao Kahn and Dark Kahn share the **identical skeletal rig**:
     `Characters\BaseMale\Meshes\BaseMale_Medium_Muscles.dkf`
   * This proves Dark Kahn's mesh is 100% rigged to standard male bones and animations.

### Current State of Dark Kahn
* **Dark Kahn is NOT visibly rendered yet.**
* Stock Shao Kahn is unlocked and 100% playable with full animations, cloth physics, attacks, and audio without crashing.
* `mods/DarkKahn/mod.toml` has been set to `enabled = false` and parked.
* The mid-loading crash was permanently eliminated by removing the broken `.xxx` file copies.

---

## 8. Modding Subsystem Specification (Phase L)

### Architecture & Discovery
`mkvsdc::ModManager` is a thread-safe singleton managing mod discovery and VFS mounting:
1. **Search Paths:** Automatically scans:
   * `<InstallDir>/mods/`
   * `<ExecutableDir>/mods/`
   * `%APPDATA%/MKVSDC/mods/`
2. **Conflict Resolution:** Mods are sorted by `priority` in **ascending order** (`10 < 50 < 100`). Lower-priority overlays mount first; higher-priority overlays mount subsequently and overwrite VFS pointers. **Highest priority wins all file conflicts.**
3. **Central Configuration (`config/mods.toml`):**
   ```toml
   [mods]
   enabled = true

   [mods.boss_unlock]
   enabled = true
   priority = 50

   [mods.darkkahn]
   enabled = false
   priority = 100
   ```
4. **Automated Verification:**
   * Run `python tests/test_modding.py` to verify manifest parsing, priority sorting, and live engine mounting.

---

## 9. Save & Persistence Subsystem

* **Save Root:** Redirected unconditionally to `./savedata` via `OnConfigurePaths` in `src/mkvdc_app.h`.
* **Container Location:** `./savedata/B13EBABEBABEBABE/4D5707E9/00000001/MK vs. DCU Game Settings/`
* **Verified Behavior:** In-game gameplay settings (`ROUNDS TO WIN = 3`, `KOMBAT CPU = HARD`) persist across process shutdown and reload.

---

## 10. Important Reverse-Engineering Discoveries

* `0x82634318`: Boss character lock check. Returning `r3 = 0` unlocks Shao Kahn and Darkseid.
* `0x826AC700`: Virtual method move table dispatch. Guarded against out-of-bounds array indices in `src/mod_manager.cpp`.
* `0x826A46E0`: Character player data query in package. Expects `char_<character>_player_data`.
* `0x82254E50`: UE3 string assignment (`FString::operator=`). Guarded against dangling pointers.
* `0x82670208`: Pawn mesh initialization step.
* `0x820D3218`: String constant `CHAR_darkkahn` in `.rdata`.
* `0x82F4E9EC`: UE3 `GNatives` bytecode dispatch table (2,048 entries).

---

## 11. Known Bugs & Blockers

1. **Dark Kahn Mid-Loading Crash (Resolved for Stock; Blocker for Simple Overlay):**
   * *Cause:* `CHAR_DarkKahn.xxx` has 0 player scripts.
   * *Status:* Bypassed for stock Shao Kahn; requires Phase M mesh injection.
2. **First-Hit D3D12 Shader Stutter (Mitigated):**
   * *Cause:* First-encounter D3D12 PSO compilation in driver.
   * *Status:* Permanent fix is distributing the pre-warmed `4D5707E9.rtv.d3d12.xpso` cache.
3. **Window Focus Input Suppression (Resolved):**
   * *Status:* Resolved via `mnk_ignore_focus = true`.

---

## 12. Failed Approaches (DO NOT REPEAT)

1. **DO NOT replace `CHAR_ShaoKahn.xxx` directly with `CHAR_DarkKahn.xxx`:**
   * Breaks character initialization due to missing player scripts.
2. **DO NOT add speculative null checks to PPC translation units:**
   * Over 400 translation units are generated. Adding guest memory null checks to ephemeral generated files violates ReXGlue architecture and will be overwritten on next codegen.
3. **DO NOT invoke PowerShell cmdlets (`Remove-Item`) directly:**
   * Always use `cmd.exe /d /c del /f /q`.

---

## 13. Testing Framework

### Automated Tests
* **Modding Test Suite:**
  ```bat
  python tests/test_modding.py
  ```
  *Expected Result:* `ALL PHASE L MODDING TESTS PASSED!`

### Manual / Regression Gameplay Test
* Run the automated match flow script:
  ```bat
  python C:\Users\HP\.gemini\antigravity\brain\28940de3-7a11-4523-8b60-85eb3fa23b4b\scratch\test_dark_kahn_flow.py
  ```
  *Expected Result:* Boots game, navigates menus, selects Arcade, unlocks Shao Kahn via RB, loads match, completes Round 1 combat, opens Pause menu, exits with code 0.

---

## 14. Git State & History

* **Branch:** `master`
* **HEAD Commit:** `12b4f36` (`Enhance mod search path resolution and automated post-build directory deployment`)
* **Previous Commit:** `b635526` (`Implement Phase L: Non-destructive VFS modding architecture and ModManager`)
* **Working Tree:** Clean (0 uncommitted files).
* **Ignored Files:** All `.xxx`, `.bin`, `.wmv`, `.xex`, `out/`, `build/`, `logs/`, and `scratch/` are ignored.

---

## 15. Current Roadmap

1. **Phase L: Modding System Completion** — **COMPLETED & COMMITTED**
2. **Phase N: Stock Compatibility & Regression Check** — **COMPLETED & VERIFIED**
3. **Phase M: Custom Character / Mesh Injection (Dark Kahn)** — **READY TO START**
4. **Phase O: Multiplayer / Netplay Foundation** — **FUTURE WORK**
5. **Phase P: Unified Game & Mod Launcher GUI** — **FUTURE WORK**

---

## 16. Recommended Next Action for the Next Agent

### Start Phase M: Custom Character / Skeletal Mesh Injection — Dark Kahn
Instead of replacing the whole package, inject Dark Kahn's visual mesh while keeping Shao Kahn's player data:
1. **Target Function:** Inspect `sub_82670208` and `sub_826A46E0` during character setup.
2. **Mechanism:** Intercept the `USkeletalMeshComponent` assignment in `OnPreLoadCharacterMesh` / `Hook_sub_82670208`.
3. **Asset Target:** Point the mesh pointer to Dark Kahn's skeletal mesh (`SK_DarkKahn` or `CHAR_DarkKahn` export) while retaining `CHAR_ShaoKahn.xxx`'s player scripts.

---

## 17. Rules for Handoff Compliance

1. **Do not redo completed work:** Phases A through L, and N are complete and verified. Do not rewrite ModManager or codegen configs.
2. **Verify claims against evidence:** Always test by building Release x64 and inspecting live logs and framebuffers.
3. **Preserve working functionality:** Ensure stock Shao Kahn and standard roster fighters never crash.

---

## 18. Quick Start for the Next Agent

```bat
:: 1. Verify working directory
cd D:\mkvsdc\mkvsdc-recomp

:: 2. Check git status
git status

:: 3. Run automated modding tests
python tests/test_modding.py

:: 4. Build host executable with Ninja
cmd.exe /c "D:\mkvsdc\run_in_env.bat ninja -C out\build\win-amd64-release"

:: 5. Begin Phase M (Mesh Injection)
:: Inspect src/mod_manager.cpp around line 425 (OnPreLoadCharacterMesh)
```
