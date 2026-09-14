# MKVSDC: Resurrektion — Modding Guide & Architecture Specification

## 1. Overview

**MKVSDC: Resurrektion** features a modern, non-destructive modding subsystem integrated directly into the host runtime and Virtual File System (VFS). Mod authors can replace game assets, alter configurations, and inject custom content without modifying the original game disc images or base installation packages.

### Key Capabilities
- **Non-Destructive VFS Layering**: Modded files dynamically shadow stock assets at runtime.
- **Deterministic Priority & Conflict Resolution**: Explicit priority indices determine which file wins when multiple mods provide the same asset.
- **Unified Configuration**: Centralized enable/disable and priority overrides via `config/mods.toml`.
- **Multi-Directory Discovery**: Automatic scanning of primary game directory (`mods/`) and user profile (`%APPDATA%/MKVSDC/mods/`).
- **Safe Disable & Isolation**: Disabling a mod instantly restores stock behavior without restarting or reinstalling files.

---

## 2. Directory Structure

A mod resides in its own folder inside any active mod search path.

```
mkvsdc/
├── config/
│   └── mods.toml                 # Central mod toggle and priority overrides
├── mods/
│   ├── BossUnlock/
│   │   └── mod.toml              # Mod manifest
│   ├── HighResUI/
│   │   ├── mod.toml              # Mod manifest
│   │   └── assets/               # Mirrored game VFS hierarchy
│   │       └── Asset/
│   │           └── UI_HUD.xxx
│   └── AudioOverhaul/
│       ├── mod.toml
│       └── assets/
│           └── Asset/
│               └── snd_sfx_dkkn.xxx
```

### Search Paths
The runtime automatically queries the following paths in order:
1. `<InstallDir>/mods/` — Primary local mod repository.
2. `%APPDATA%/MKVSDC/mods/` — User-specific mod repository (Windows).

---

## 3. Mod Manifest (`mod.toml`)

Every mod folder **must** contain a `mod.toml` file at its root.

```toml
# Example mod.toml
id = "high_res_ui"
name = "High-Resolution UI & HUD"
version = "1.2.0"
author = "MK Community"
description = "Upscaled HUD and menu textures for 4K displays."
enabled = true
priority = 100
```

### Manifest Fields

| Field | Type | Required | Default | Description |
|---|---|---|---|---|
| `id` | String | Yes | Folder name | Unique identifier used for configuration and conflicts. |
| `name` | String | No | `id` | Human-readable display name for launchers. |
| `version` | String | No | `"0.0.0"` | SemVer version string. |
| `author` | String | No | `"Unknown"` | Creator name or organization. |
| `description` | String | No | `""` | Description of the mod's contents and features. |
| `enabled` | Boolean | No | `true` | Default activation state. |
| `priority` | Integer | No | `0` | Default load priority (higher = higher precedence). |

---

## 4. Central Configuration (`config/mods.toml`)

The central configuration file allows players or launchers to override mod states without editing the individual mod manifests.

```toml
# MKVSDC: Resurrektion - Mod Manager Configuration
[mods]
enabled = true

[mods.boss_unlock]
enabled = true
priority = 50

[mods.high_res_ui]
enabled = true
priority = 100

[mods.darkkahn]
enabled = false
priority = 10
```

### Precedence Rules:
1. **Master Switch (`[mods].enabled`)**: If `false`, all mod VFS overlays are bypassed.
2. **Individual Overrides (`[mods.<id>]`)**: Central values override the manifest's default `enabled` and `priority`.

---

## 5. VFS Overlay & Priority Conflict Resolution

When multiple mods supply the same relative file path (e.g. `Asset/CHAR_SubZero.xxx`), conflict resolution is strictly deterministic:

- **Sorting**: Mods are sorted by `priority` in **ascending order** (`priority 10 < priority 50 < priority 100`).
- **Mount Sequence**: Overlays are mounted sequentially. Lower-priority mods mount first; higher-priority mods mount subsequently.
- **Winning Path**: In ReXGlue's `HostPathDevice`, the latest mounted overlay updates the VFS entry pointer (`host_path_`). Therefore, the mod with the **highest priority value wins**.

---

## 6. ModManager C++ API (Launcher & Tooling Integration)

The runtime exposes a singleton interface for inspecting and modifying mods:

```cpp
#include "mod_manager.h"

auto& mm = mkvsdc::ModManager::Instance();

// Querying
size_t count = mm.GetModCount();
const mkvsdc::ModManifest* mod = mm.FindMod("boss_unlock");
if (mod && mod->enabled) {
    // Mod is active
}

// Modifying and saving
mm.SetModEnabled("high_res_ui", false);
mm.SetModPriority("high_res_ui", 200);
mm.SaveCentralConfig();

// Reloading after disk changes
mm.Reload();
```

---

## 7. Best Practices for Mod Creators

1. **Keep Asset Paths Relative to Game Root**:
   Files placed inside `<ModDir>/assets/` should mirror the stock installation paths (e.g., `assets/Asset/` for packages, `assets/Config/` for configs).
2. **Do Not Overwrite Stock Packages Directly**:
   Always place files in your mod folder; never modify the files under the base installation.
3. **Use Descriptive IDs**:
   Use lowercase alphanumeric characters and underscores (e.g., `cyber_subzero_skin`).
4. **Clean Distribution**:
   Distribute mod packages as a single zip archive whose root matches `<ModFolder>/mod.toml` + `<ModFolder>/assets/`.
