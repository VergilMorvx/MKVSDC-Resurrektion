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
| **Phase I: Interactive Combat & Optimization** | **COMPLETE** | Interactive gameplay confirmed. First-encounter combat stutter resolved via persistent D3D12 PSO caching. The app requests VSync (`SyncInterval = 1`) by default after loading its GPU plugin and preserves explicit CVar overrides. The SDK suspends mouse-look while any ImGui drawer has an open dialog and restores it after the last dialog across all drawers closes; live cursor behavior is still awaiting interactive verification. Current frame cadence still needs an ETW measurement. |
| **Phase J: Cinematic & Movie Subsystem** | **COMPLETE (historical)** | 119 WMV files audited. Earlier in-engine Xenon DXVA runs verified Studio Logo playback and the `MK001.wmv` and `dc001.wmv` Story Mode intros. The current 2026-09-26 Story Mode re-entry did not establish the exact movie asset ID. |
| **Phase K: Save System & Profile Persistence** | **COMPLETE** | Local storage container mounting (`XamContentCreate`, `XamContentOpen`, `XamContentClose`) redirected to `./savedata/`, automated headless profile registration, container persistence (`MK vs. DCU Game Settings` & `MK vs DCU`), and verified cold-restart state restoration. |
| **Phase L: Mod Framework** | **IMPLEMENTED** | Mod discovery, central TOML overrides, priority overlays, and runtime hooks. Asset overlays are applied at startup and need a restart after changing. |
| **Phase M / M+: Dark Kahn** | **PARTIALLY VERIFIED** | Current captures show Dark Kahn selected with matching large art, fighting as Player 1 in Arcade and Player 2 in Practice, and responding to combat input. An isolated package probe confirms the selection-name FName redirect, and the target announcer FmodEvent exists in the retail package; the latest 110-second default-speaker loopback capture contains active audio during the run, correcting an earlier silence-only capture. Its log has no announcer cue-name hook call, so selection/victory playback and a Dark Kahn particle emitter remain unverified. |
| **Audit remediation** | **IN PROGRESS** | SDK Debug/Release full CTest pass 1,679 entries each (1,675 passed, four existing conditional skips); project CTest passes 3/3. Fresh codegen, 36 hook checks, Dark Kahn Arcade and Practice, stock Scorpion-versus-Deathstroke combat with both gameplay mods off, mod checks, clean deployment staging, isolated post-plugin CVar checks, and a clean portable ZIP launch pass. The mouse-look dialog gate now serializes cross-drawer transitions and has two regression cases, repeated 100 times in Release; live cursor behavior is still awaiting interactive verification. Cue-pool fallback behavior and Dark Kahn package alias gating have helper-level coverage; package strings and diagnostic structure reads are now bounded. A fresh match run exercised the package callbacks without invalid-name logs. Live cue-specific audio, live fallback dispatch, FX attachment, movie identity, and measured frame cadence remain open. |

Phases G–K summarize earlier milestone evidence. The current audit reran
fresh Release codegen, the complete SDK test suite in Debug and Release, an
Arcade menu-to-combat run, a Player 2 Practice run, a stock combat regression,
clean deployment staging checks, and isolated post-plugin D3D12 CVar launches against the current app build. See the current capture sequences
under `scratch/phase_m/audit_20260926_sdk_xex_fix/` and
`scratch/practice_runs/audit_20260926_release_p2_smoke2/`, plus the follow-up
captures under `scratch/phase_m/audit_20260926_phase_n_stock_skiprb/`. After
the GPU-plugin lifecycle fix, another stock run reached Scorpion-versus-Batman
combat and pause in `scratch/phase_n_stock_cvarfix_20260926/`.

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
| **I — Combat, graphics performance, and input** | Isolated first-hit stutter to D3D12 PSO compilation, persisted and warmed the pipeline cache, selected the high-performance GPU, added presentation pacing, and enabled keyboard/mouse input. | Historical gameplay/PSO verification. Current Release launch confirms post-plugin discrete-adapter selection and VSync default; config/environment interval 2 and CLI interval 0 are preserved. Actual frame timing remains unmeasured. |
| **J1/J2 — Movies and story cinematics** | Cataloged 119 WMV assets and verified in-engine logo playback plus the `MK001.wmv` and `dc001.wmv` Story Mode introductions using the Xenon movie path. | Complete; historical verification. |
| **K — Save and profile persistence** | Implemented local content-container handling under `./savedata/` and verified settings/profile restoration after a cold restart. | Complete; historical verification. |
| **K3 — Dark Kahn investigation** | Traced selection, player setup, script/move data, and mesh/material loading. Documented why a drop-in package replacement was unsafe and parked that route. | Investigation complete; replacement route parked. |
| **L — Mod framework** | Added non-destructive VFS overlays, mod discovery and priority, TOML configuration, runtime hooks, and post-build deployment. | Implemented; startup and mod checks rerun during this audit. |
| **M/M+ — Dark Kahn mod** | Added selection aliases and package/mesh/audio/effect hooks. Current captures show Dark Kahn selected with matching large art, fighting as Player 1 in Arcade and Player 2 in Practice, and responding to combat input. A local package probe exercises the selection cue-name redirect; a new system loopback capture records active audio during a current match run, while cue-specific playback and particle attachment remain open. | Partially verified; active work. |
| **N — Stock compatibility** | Rebuilt current Release code with `darkkahn`, `boss_unlock`, and `test_mod` disabled in isolated staging; Scorpion-versus-Deathstroke reached combat, accepted attacks, and opened pause. | Current stock regression passed 2026-09-26. |
| **O — Multiplayer/netplay** | Identified networking stubs and recorded a possible foundation. | Proposal only; not implemented. |

### Audit Remediation Stages

| Audit stage | Changes made | Evidence / remaining limits |
| :--- | :--- | :--- |
| **0 — Preserve baseline** | Preserved existing project and SDK edits; confirmed the pinned ReXGlue base and captured current state. | Complete. No existing working-tree changes were discarded. |
| **1 — Reproducible SDK and code generation** | Added `patches/rexglue-sdk.patch` and its guarded apply script; tracked generated callback edits and added a verifier/post-codegen script; documented the configure/codegen/reconfigure sequence; staged the Xenos GPU plugin and required SSSE3/ImGui build settings. | Clean pinned-SDK application and idempotent reapplication passed. Fresh codegen verified 36 callback insertions and built the executable, runtime, and GPU plugin. Generated game code and proprietary assets stay untracked. |
| **2 — Runtime safety and configuration correctness** | Made ModManager reads return synchronized snapshots; made config writes use a temporary file and report failures; preserved runtime `config/mods.toml` during post-build deployment; retained preferences for temporarily undiscovered mods; range-checked TOML priorities and quoted saved mod IDs; aligned manifest metadata defaults with the guide; rejected linked overlay roots, skipped symlinks and Windows reparse points in host indexing, lazy lookup, and overlays, and capped VFS traversal at 128 directory levels; scoped mesh fallback to the current thread/load; serialized shared package/audio guest scratch use and ordered boss-select reset with enqueue; initialized immutable mesh/HUD/player-data strings, fixed voice/SFX/foley package names, and the synthetic-costume record once in `ApplyHooks`; replaced rotating cue-name rings with a deduplicated 16-slot immutable scratch pool; respected explicit false flags; documented restart-required overlay changes; rejected VFS file/directory collisions; fixed auto-continue and exact test-button parsing; moved D3D12 adapter/VSync setup after GPU-plugin loading; preserved higher-priority interval sources; and clamped presentation intervals; initialized D3D12 texture-copy layout values, fixed a dangling XAM launch-path view, fixed signed scissor bounds and `RtlCompareStringN`'s `0xFFFF` length handling; bounded XEX image/header/import/PE ranges; and prevalidated complete LZX delta streams with separate source and destination bounds. | Debug and Release SDK builds/install and CTest pass 1,677 tests each: 1,673 pass and four existing conditional skips (219 unit cases, 1,458 PPC cases). Fresh codegen loads the real `default.xex`, the module is up to date, and all 36 generated hook calls verify. The project Release build passes. Isolated post-plugin launches selected adapter 0 (7,956 MiB VRAM), defaulted interval 1, preserved config/environment interval 2, and preserved explicit CLI interval 0. WPR denied GPU tracing (0xc5585011), so actual frame cadence remains unmeasured; installed and deployed runtime DLL hashes match. Deployment fixtures and both automatic/explicit `tests/test_modding.py` checks pass. Dark Kahn Arcade, Player 2 Practice, and disabled-mod stock combat/pause runs pass. The selection FName rewrite was exercised in an isolated package probe; a fresh 110-second default-speaker loopback run contains active audio during gameplay, but no cue-name hook was logged, so Dark Kahn announcer playback is unverified. Pool exhaustion and package alias gating have helper-level tests; live package-fallback hook execution, FX attachment, general package-object lifetime, live mouse-look behavior, movie identity, and measured frame cadence remain open. |
| **3 — Tooling, regression checks, and documentation** | Made `tests/test_modding.py` use checkout-relative paths, select the newest build logs by default (or honor `MKVDC_BUILD_DIR`), load the effective config/manifests used by that run, and restrict telemetry checks to the latest startup; made vtable harvesting emit a separate candidate file by default, refuse overwriting the canonical config, and added a conflict-checking merge tool; made the Phase M harness select the current audit build by default, accept build overrides, and refuse to overwrite prior evidence; added Windows SDK tests for linked roots, overlay descendants, and lazy base-path lookups, plus overflow boundary tests for XEX byte ranges; fixed the SDK unit-test include path and depfile test's invalid C++ escape; made the SDK patch verifier capture added files through a temporary index; cleaned unsafe pointer-bearing texture resets and SDK initialization-order warnings; made sentinel comparisons explicit; refreshed the SDK patch; ignored Python bytecode caches produced by local test runs; and reconciled current status across the docs and mod/config path precedence in the guide. | Both automatic and explicit-build mod checks pass against the latest interactive startup logs. Synthetic enabled/disabled config fixtures and stale-telemetry checks passed. The vtable scan found 0 unregistered functions; synthetic merge/conflict checks passed without changing the canonical config. Debug and Release full CTest each passed 1,677 tests with four existing conditional skips; a synthetic clean-apply check confirmed the patch verifier detects untracked additions without changing the repository index. Current Dark Kahn and stock runs prove both enabled and disabled flows reach combat; disabled config keeps the Dark Kahn/test overlays unmounted. The selection cue-name FName hook logs a Dark Kahn redirect in an isolated package probe, but direct audio and emitter attachment remain open. |
| **4 — Phase M+ package investigation** | Inspected UI, character, audio, and FX package exports; distinguished the default P2 preview from the selected Dark Kahn screen, versus screen, and combat mesh; removed ineffective body-art and cue rewrite attempts; recorded candidate sockets and effects; replaced reusable cue rings with a deduplicated stable guest-string pool. | The latest capture shows the `DARK KAHN` selection label and matching large art after the boss-slot toggle; the following fight selects Dark Kahn as Player 1. The mesh redirect succeeds. An isolated package probe logs the Shao-to-Dark-Kahn selection cue-name redirect. A later 110-second loopback capture records active audio during gameplay but no announcer hook call; emitter attachment is also unverified. No package repacker or proprietary package is included. |
| **5 — Regression and distribution gate** | Rebuilt after fresh codegen, ran the full SDK Debug/Release CTest suites, rebuilt the project Release target, ran automatic and explicit mod checks, captured Player 1 Arcade and Player 2 Practice combat, exercised clean deployment staging plus a 40-second startup smoke, navigated a staged run into Story Mode, tested keyboard navigation, a configurable LMB-to-A event, save reload, and post-plugin D3D12 CVar setup with default and explicit-override launches, then ran a stock Scorpion-versus-Deathstroke match with gameplay mods disabled. Added a CMake `mkvdc_package` target, package-content tests (including duplicate-ID and linked-input rejection), and a clean staged launch. | Selected-state art, both player slots, Dark Kahn and stock combat input, pause, clean staging, keyboard menu navigation, the configurable mouse-button event, cold-restart save reload, default interval 1 plus preserved config/environment interval 2 and CLI interval 0 are verified. The portable ZIP contains the executable/runtime/GPU plugin, docs, manifests, and config with asset mods disabled; its extracted copy started successfully. Game/mod assets, saves, and logs are excluded. `KOMBAT CPU: HARD` and `ROUNDS TO WIN: 3` survived a clean close and cold restart in staging. The Story Mode run captured the Mortal Kombat campaign's cemetery/portal opening but did not prove the movie asset ID or an exact decoded-frame match. An earlier 10-second loopback capture contained silence; the later 110-second default-speaker capture contains active audio from the combat run, but no cue-name hook was logged. Mouse-look movement and measured frame cadence remain unverified; WPR denied the GPU trace because profiling privileges were unavailable. Project CTest runs pool boundary/concurrency and package-builder tests; the app log/config check (`tests/test_modding.py`) is run directly. Direct announcer audio and FX attachment remain open. |
| **6 — UI input lifecycle** | Found that mouse-look capture was never disabled while ImGui dialogs were open. The SDK suspends capture until the final open dialog closes across all drawers and releases each drawer's ownership during teardown. Review found that an atomic count alone could let concurrent enable/disable callbacks run out of order; a small mutex-backed gate now serializes both count and state transitions. The state-changing release call is kept outside the SDK's debug-only assertion macro so Release builds execute cleanup. Also removed the signed/unsigned mouse-button bounds warnings. | Added two SDK regression cases for nested drawers, underflow rejection, and concurrent lifetimes. Debug and Release each pass all 1,679 CTest entries (1,675 passed, four existing conditional BitStream skips); the two gate cases also passed 100 consecutive Release repeats. Project CTest passes both checks, and the Release app and portable ZIP were rebuilt. The live cursor check could not foreground the game window in this automation session, so actual cursor transition remains unverified. |
| **7 — Announcer scratch-pool coverage** | Extracted the stable guest-string pool used by the Dark Kahn announcer hooks and registered pool/package tests with project CTest. | CTest covers empty/oversize input, 16-slot capacity, duplicate reuse, address overflow, and concurrent distinct and duplicate calls. Both project tests pass, and the concurrency test passed 100 repeated runs. |
| **8 — SDK tool build cleanup** | Made the codegen utility's tool-mode `RuntimeConfig` explicitly initialize its empty graphics, GPU-plugin, audio, and input backends. This preserves the null-backend behavior and removes the missing-field aggregate warning. | SDK Debug and Release tool builds complete without that warning; full CTest passes 1,679 entries in both configurations. The pinned SDK patch was regenerated and the exact-match verifier passes. |
| **9 — Cue-pool exhaustion fallback** | Routed all four announcer name hooks through a shared helper that updates the guest register only after a stable string is allocated. | Added a regression proving successful redirects replace the full 64-bit argument and pool exhaustion preserves it. Release build and project CTest pass (2/2). End-to-end execution of each hook and audible playback remain unverified. |
| **10 — Package fallback gating** | Gated Dark Kahn package aliases on the mod's enabled state, returned safely for a null guest base, and made the guest package-pointer upper bound exclusive. | Added resolver cases for case-insensitive aliases, disabled-mod behavior, unrelated names, and guest pointer bounds. The Release build and project CTest pass (3/3); the live package-miss hook was not reached. The portable ZIP was rebuilt with the latest README. |
| **11 — Current Phase M integration capture** | Ran the current Release build from title screen through Dark Kahn Player 1 combat and pause while capturing default-speaker WASAPI loopback and paired ModManager logs. | The 110-second capture contains active samples from about second 30 onward (peak 0.599, RMS 0.0307); the game log shows Dark Kahn mesh and voice/SFX package redirects and `FX_DarkKahn` linking. No announcer cue-name hook or package-fallback call appears, so the capture verifies active system output during the run, not a specific Dark Kahn cue or FX attachment. Local evidence: `scratch/phase_m/goal_audio_20260926/` and `scratch/phase_m/goal_audio_20260926_loopback.wav` |
| **12 — Bounded package fallback input** | Changed the fallback hook to require a NUL-terminated guest package name within both the remaining guest-memory range and a 128-character limit before alias matching. | Added cases for null, empty, truncated, maximum-length, and unterminated overlong input. The Release app builds and project CTest passes 3/3. Runtime fallback dispatch remains unobserved in the game. |
| **13 — Story Mode movie identity follow-up** | Compared the 10-, 25-, and 45-second current Story Mode captures against every decoded frame of `MK001.wmv` (2,929 frames). | Nearest source times were 13.37, 5.67, and 8.20 seconds, with resized RGB mean absolute errors 19.466, 23.177, and 38.408/255. The source-time order is inconsistent and the errors do not establish a direct match; this current run remains open for movie identification. |
| **14 — Announcer event asset verification** | Listed the retail `snd_vo_anno_shell.xxx` exports and confirmed both Shao Kahn's source name event and Dark Kahn's target name event exist; the stock `SystemArt.xxx` contains the Shao Kahn reference. | UEViewer cannot export these FmodEvent/FmodEventFile classes; the embedded FSB4 is now decoded directly in Stage 19. The current gameplay log has no cue-name callback, so asset presence does not verify playback. |
| **15 — Bounded package diagnostic reads** | Added end-exclusive guest-range checks for package callback structures, safe big-endian field reads, and a 128-character NUL-terminated view for package names. Kept zero filename pointers labeled null/empty. | Added range-boundary tests; Release app builds and CTest passes 3/3. A fresh Phase M run logged 18 package-file results, 108 linker ticks, 68 linker requests, and 578 async ticks; the latest startup block has no invalid/unterminated names, fatal errors, or access violations and reached combat/pause. |
| **16 — Bounded sound-package redirect input** | Validated the guest name pointer and required a NUL terminator within guest memory and the 128-character limit before matching sound-package redirects. Factored exact, case-insensitive redirect decisions into a testable helper; only supported package-load operations and the enabled mod redirect. | Added coverage for all three redirected packages, disabled-mod behavior, unsupported operations, and partial names. Release build and CTest pass 3/3. A fresh Arcade run reached Dark Kahn combat and pause and logged 48 valid sound-package redirects with no invalid or unterminated-name labels, fatal errors, or access violations. |
| **17 — Story Mode cross-asset movie search** | Compared the same three current Story Mode captures against 117 extracted MK/DC gameplay WMVs, sampling each at 1 frame/second and 160×90. | No candidate gives a low-error, timing-consistent match. The lowest aggregate error is `mk025.wmv` at 26.17/255, with best source seconds 38, 44, and 1; the best chronologically ordered candidate is `mk013.wmv` at 26.44/255, with source seconds 0, 1, and 12. This coarse search narrows candidates but does not establish movie identity; keep the current re-entry unresolved. |
| **18 — Range-checked move-table guard** | Routed the guard's guest-structure reads through checked big-endian reads and overflow-safe address calculations. Kept the guest's 32-bit shifted-index encoding; invalid reads skip the guard inspection, while invalid move-descriptor pointers still stop the unsafe dispatch. | Added guest-array address tests for one-based indices, zero, range boundaries, overflow, and the encoded byte-offset path. Release build and CTest pass 3/3. A fresh Dark Kahn Arcade run reached combat after attack inputs and pause with no prevention errors, invalid-name labels, fatal errors, or access violations. |
| **19 — Decode the Dark Kahn announcer sample** | Extracted the exact embedded FSB4 bank from retail `snd_vo_anno_shell.xxx` and decoded it with the official vgmstream CLI. Its 64-stream bank includes `vo_anno_name_dkkn01_frnt.wav` (stream 35) and `vo_anno_name_dkkn01_rear.wav` (stream 29), each 2.917 seconds at 44.1 kHz, Xbox Media Audio 2. | The target sample is now directly decoded, but a fresh match log still has no cue-name hook call. Correlation against the older loopback does not distinguish the target from the Shao Kahn control; the fresh loopback capture was all zero with data-discontinuity warnings. In-game cue playback remains unverified. |

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
check pass. Debug and Release builds/install pass, and full CTest passes 1,679
tests per configuration: 1,675 pass and four existing conditional BitStream
cases skip. Each run includes 221 unit cases (217 pass, four skip) and 1,458 PPC
cases. The project Release target rebuilds, all 36 generated hook sites verify,
and installed/deployed `rexruntime.dll` hashes match.

The fresh capture at
`scratch/phase_m/audit_20260926_sdk_xex_fix/` shows Dark Kahn's selected art and
name after the boss-slot toggle, then Player 1 Dark Kahn in combat and pause.
The second current-build run at
`scratch/practice_runs/audit_20260926_release_p2_smoke2/` shows Player 2 Dark
Kahn in Practice responding to a hit. `tests/test_modding.py` passes with
automatic build selection and an explicit `MKVDC_BUILD_DIR`. Sound-package
redirects ran, but the cue-specific announcer path was not logged; `FX_DarkKahn`
linked, but no emitter was attached. A later current-run loopback capture is
summarized in Stage 11 below.

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
not mounted. A separate clean staging run reached Story Mode's Mortal Kombat
campaign opening and captured the cemetery/portal scenes. The extracted
`MK001.wmv` timeline contains similar footage, but the current run did not prove
the movie asset ID or an exact decoded-frame match, so Story Mode playback stays
open. A corrected keyboard probe navigated menus; changing Gameplay Options to
`KOMBAT CPU: HARD` and `ROUNDS TO WIN: 3` survived a clean close and cold restart
in staging. The cue-name rings have since been replaced by a deduplicated,
immutable 16-slot guest scratch pool, and an isolated package probe confirms the
selection FName redirect. The shared register helper now preserves the stock
argument on pool exhaustion; full guest-hook execution and audible output still
need direct verification. Remaining work is FX binding, general package-object
lifetime checks, mouse-look movement, and measured frame cadence. VSync now
defaults to interval 1 after the GPU plugin loads, and an explicit interval 0
survives startup. WPR's GPU trace request was denied with `0xc5585011`, so this
proves the configured request but not actual frame timing. A configurable
mouse-button event has advanced character select to opponent selection. A
portable ZIP target now writes
`out/build/audit-win-amd64-release/dist/mkvdc-portable-win-amd64.zip`; its
clean extracted launch passed. Build it with
`cmake --build out/build/audit-win-amd64-release --target mkvdc_package`.
The ZIP includes the runtime, docs, mod manifests, and a config with asset mods
disabled. It excludes game/mod asset packages, saves, and logs. This is a
portable ZIP, not a Windows installer.

A fresh stock regression used a separate staging copy with Dark Kahn, Boss
Unlock, and Test Mod disabled. The corrected ordinary-roster run reached
Scorpion-versus-Deathstroke combat, accepted attacks, and opened pause; its
configuration check confirms the Dark Kahn and Test Mod overlays stayed
unmounted. An earlier attempt selected the locked boss slot and was excluded.
The announcer-hook probe changed three equal-length cue strings only in an
isolated `SystemArt.xxx` copy; the runtime log confirms the Shao Kahn selection
cue name was redirected to Dark Kahn. An earlier 10-second WASAPI loopback
capture from the default Headphone endpoint contained only zero samples. The later 110-second default-speaker
capture at `scratch/phase_m/goal_audio_20260926_loopback.wav` has nonzero audio
during the current match run (peak 0.599, RMS 0.0307); its paired log contains
no announcer cue-name hook call. This confirms active system output during that
run, while the specific selection/victory cue remains unverified.

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
four-slot ring. Variable announcer cue names now use a deduplicated immutable
16-slot guest-string pool; runtime tests for capacity and concurrent distinct
names remain open.

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
- **VSync Request**: `d3d12_present_interval` defaults to `1` after the GPU plugin loads and requests sync on each swapchain presentation; config, environment, and CLI overrides are preserved. Current launch checks verify the selected interval, but actual frame cadence was not measured in this audit.
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
ctest --test-dir out/build/win-amd64-release --output-on-failure

# Optional: create a portable ZIP under out/build/win-amd64-release/dist/
cmake --build --preset win-amd64-release --target mkvdc_package

```

The preset uses `../rexglue-sdk` by default. Set `REXSDK_DIR` in the CMake cache to use another checkout. The first configure and codegen pass requires the game paths in `mkvdc_manifest.toml` to point to files on your machine. CMake reapplies the tracked callback patch after codegen; running `rexglue codegen` manually requires `python scripts/apply_generated_hooks.py` afterward.

The output executable `mkvdc.exe` will be located in `out/build/win-amd64-release/`.
The portable archive is written to
`out/build/win-amd64-release/dist/mkvdc-portable-win-amd64.zip`. Asset mods ship
disabled because their game assets are supplied separately; the package omits
game packages, saves, and logs.

---

## Running the Game

Run the executable and provide the path to your extracted Xbox 360 game data:

```powershell
.\out\build\win-amd64-release\mkvdc.exe --game_data_root="D:\Games\MKvDC_Extracted"
```

### Useful Command-Line Flags
- `--d3d12_present_interval=1`: Requests VSync presentation (the app default). Set to `0` for immediate/uncapped presentation.
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
