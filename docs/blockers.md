# Project Blockers & Open Investigations

## Active Blockers
- **NONE**. All baseline, vtable, VM bytecode dispatcher, COM interface stub, CRT comparator, D3D12 presentation pacing, and PSO caching barriers have been resolved with strict zero-force codegen.

## Achieved Milestones
- **RESOLVED & VERIFIED (2026-09-11)**: **Phase G (First Visual Frame)**:
  - Definitive visual verification: Host window `mkvdc [rexglue-v0.10.0.2-dev.gc94f5eb-Release]` rendering 1280x720 3D Unreal Engine 3 graphics with 99.93% non-black pixels.
- **RESOLVED & VERIFIED (2026-09-11)**: **Phase H (In-Game 3D Gameplay & Attract Mode)**:
  - Real-time in-engine combat sequence active: The Flash vs. Kano in the Fortress of Solitude ice arena.
  - Dynamic character skeletal animation, particle physics (blood splatters), dynamic shadow casting, and 5.1 surround audio streaming running stably at 60 FPS.
- **RESOLVED & VERIFIED (2026-09-11)**: **Phase I (Interactive Gameplay & Performance Optimization)**:
  - Interactive combat confirmed stable across multiple character matchups (Liu Kang, Catwoman, Wonder Woman, Sub-Zero, Raiden).
  - First-encounter combat stutter isolated via A/B/C testing and resolved: proved to be host D3D12 PSO compilation. Pre-warmed persistent shader cache (`4D5707E9.rtv.d3d12.xpso`, 597 pipelines) deserializes in 131 ms and eliminates hitching completely.
  - Presentation pacing locked at 60 FPS (`d3d12_present_interval = 1`) via custom ReXGlue SDK CVar.
  - Automatic discrete GPU detection implemented in `src/mkvdc_app.h`, correctly routing rendering to NVIDIA GeForce RTX 4060 Laptop GPU.
- **RESOLVED & VERIFIED (2026-09-11)**: **Phase J1 (Studio Logo Cutscenes & In-Engine WMV Playback)**:
  - Video decoding subsystem empirically confirmed active in host execution.
  - Bit-for-bit frame match verified against source assets for `midway_logo.wmv` (3.32 MB), `WB_Logo.wmv` (5.97 MB), and `DC_Logo.wmv` (4.43 MB).
  - Clean video-to-viewport state restoration and transition into live 3D title screen verified.
  - Keyboard/mouse controller emulation (`mnk_mode = true`) integrated by default.
- **RESOLVED & VERIFIED (2026-09-12)**: **Phase J2 (Story Mode In-Engine Cutscenes)**:
  - In-engine high-bandwidth cinematic playback empirically verified for `MK001.wmv` (Mortal Kombat intro, 81.17 MB) and `dc001.wmv` (DC Universe intro, 104.40 MB).
  - Sampled frame sequences matched source FFmpeg reference frames bit-for-bit with full 60 FPS DXVA shader decode, synchronized audio, and smooth transition back to Unreal Engine 3 gameplay.
- **RESOLVED & VERIFIED (2026-09-13)**: **Phase K (In-Game Save Persistence & Container Lifecycle)**:
  - Local save redirection unconditionally mapped to `./savedata/B13EBABEBABEBABE/4D5707E9/00000001/`.
  - Automated headless profile sign-in and device selection configured.
  - In-game container creation (`xeXamContentCreate`, `CREATE_ALWAYS`) verified: setting adjustments in `GAMEPLAY OPTIONS` (`KOMBAT CPU: HARD`, `ROUNDS TO WIN: 3`) write binary payloads and `.header` metadata to disk.
  - Cold process reboot verification: after complete process exit, restarting from scratch successfully discovers existing containers, mounts `save:`, and restores modified gameplay parameters bit-for-bit into active engine state.

## Current Phase Goals: Phase L (Extended Progression, Cheats & Character Unlock Integrity)
1. Verify story chapter completion progression writing to `MK vs DCU` save container.
2. Validate arcade ladder save state and unlocked characters (Dark Kahn, Shao Kahn, Darkseid).
3. Continue monitoring stability and regression checks across long gameplay sessions.
