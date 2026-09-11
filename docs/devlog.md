# MKvDC Static Recompilation Development Log

## 2026-09-11 — Initialization & Environment Verification
- End-to-end audit of MKvDC_Recomp_Master_Dossier.md completed.
- Verified default.xex SHA-256: 2955F2E2BE61EC1948CD2FD3538AD45BEB04772F5EBD5E0FB1BFDE484748E5A7.
- Verified asset inventory (838 files, 698 .xxx packages, 119 .wmv movies, language packages).
- Cloned official ReXGlue SDK recursively at commit c94f5ebdcb3c9d1a460ca48e04f9758448f8d518.
- Fixed Windows checkout issue in rexglue-sdk/thirdparty/CMakeLists.txt for libmspack where symlink folder was used instead of actual source folder.
- Successfully built and installed ReXGlue SDK (Release) using Clang 22.1.8, Ninja 1.13.2, CMake 4.3.1, and Visual Studio 2026 Developer Environment.
- Executed canonical ReXGlue project initialization:
  rexglue init --project-name MKvDC --xex-path 'D:\mkvsdc\MKvDC_Extracted\Mortal Kombat vs. DC Universe (World) (En,Fr,De,Es,It)\default.xex' --game-root 'D:\mkvsdc\MKvDC_Extracted\Mortal Kombat vs. DC Universe (World) (En,Fr,De,Es,It)' --project-root 'D:\mkvsdc\mkvsdc-recomp'
- Verified generated project structure: mkvdc_manifest.toml, CMakeLists.txt, CMakePresets.json, generated/rexglue.cmake, src/main.cpp, src/mkvdc_app.h.
- Initialized local Git repository with strict .gitignore preventing any copyrighted game files or ephemeral generated C++ from being committed.
