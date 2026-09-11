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

## Current Phase Goals: Phase J (Cinematic Sequences & Cutscene Playback)
1. Verify WMV video decoding pipeline via guest Xenon DXVA shaders.
2. Audit playback of intro movies, story mode transitions, and attract mode cinematics against the 119-movie ledger (`docs/wmv_status.md`).
3. Validate audio synchronization across multi-channel video playback and transitions into interactive matches.
