# Phase K3 — Dark Kahn Character Investigation & Technical Analysis

## Executive Summary

Phase K3 evaluated whether Dark Kahn (`CHAR_DarkKahn.xxx`) can be cleanly unlocked or substituted into the Mortal Kombat boss roster slot. 

Following manual verification and rigorous reverse engineering across the recompiled C++ codebase, runtime guest memory, and decompressed Unreal Engine 3 package archives, it has been established that:

1. The boss slot toggle (`RB`) natively selects `CHAR_ShaoKahn` (`0x820D17D0`). Our hook at `sub_82634318` (`OnCheckCharacterUnlocked`) successfully unlocks this slot and removes the padlock.
2. The playable fighter rendered on screen is **stock Shao Kahn**, which is a complete, fully playable character created by Midway with animations, move lists, and sound effects.
3. Complete physical replacement of `CHAR_ShaoKahn.xxx` with `CHAR_DarkKahn.xxx` via VFS overlay fails during match loading because Dark Kahn was architected as an NPC boss actor. He lacks standard playable character script tables (`element count = 0`), which results in a NULL pointer dereference in `sub_82681490` at line 11766 of `mkvdc_recomp.104.cpp`.
4. Per project directives, Dark Kahn character injection is formally documented and parked for **Phase M (Character Mod Support)** so that the core generic modding framework (**Phase L**) and subsequent project milestones can proceed without delay.

---

## 1. Complete Pipeline Trace: Character Select to 3D Pawn

### Step 1: Character Select & Roster Selection
- The character selection screen uses hardcoded character descriptor addresses in guest memory:
  - MK Boss Slot: `0x820D17D0` (`"CHAR_ShaoKahn"`)
  - DC Boss Slot: `0x820D17C0` (`"CHAR_Darkseid"`)
  - Standalone Dark Kahn string: `0x820D3218` (`"CHAR_darkkahn"`)
- Pressing `RB` toggles between the standard roster and the boss characters.
- `sub_82634318` checks whether the character name matches entries in `player_locked_list` (`0x820D0BC8`). Returning `r3 = 0` bypasses the lock check and makes the boss selectable.

### Step 2: Player Struct Initialization
- When a match is initiated, the engine calls `sub_8262FCD8(player_index)` to retrieve the player structure.
  - Player 0 struct: `0x83007250` (or dynamic heap instance `0x82FC8810` / `0x82FD62AC`).
  - Character identifier pointer: stored at `player_struct + 24`.
  - Outer package pointer: stored at `player_struct + 50760`.
  - Costume data pointer: stored at `player_struct + 128`.

### Step 3: Script & Move Table Loading
- In `mkvdc_recomp.182.cpp` (`sub_82680EB0`), the engine calls `sub_82761418` to instantiate the character script table.
- For all playable characters (Batman, Catwoman, Scorpion, Shao Kahn), `sub_82761418` returns a script table object containing 70–95 script elements (e.g., `jumping_attack`, `boxing_2player`, `his_info`, `test`).
- The engine then calls `sub_826A46E0(table, "<char_name>_player_data", 1)` to look up the character's player data and costume definitions.
- The returned pointer is stored into `player_struct + 128` (`CostumeData`).

### Step 4: Skeletal Mesh & Material Setup
- In `mkvdc_recomp.104.cpp` (`sub_82681490`), the engine reads:
  ```cpp
  r11 = *(r15 + 128); // CostumeData
  r4  = *(r11 + 0);   // Primary Mesh name FString (e.g. "Meshes.ShaoKahn")
  ```
- It passes `InName = *(r11 + 0)` and `InOuter = *(r15 + 50760)` to `sub_822BC870` (`StaticLoadObject`).
- The resulting `USkeletalMesh*` is attached to `USkeletalMeshComponent` via `sub_82561E38` (`SetSkeletalMesh`).
- Finally, the engine loads accessories (Shao Kahn's skull dome helmet, cape cloth component, and shoulder armor) and attaches them to the pawn.

---

## 2. Root Cause Analysis: Why Dark Kahn Package Overlay Crashed

When `mods/DarkKahn/assets/Asset/CHAR_ShaoKahn.xxx` was overlaid with `CHAR_DarkKahn.xxx`:

1. **VFS Overlay Operational**: The VFS overlay successfully intercepted the request:
   ```
   VFS Overlay: Overriding '\Device\Harddisk0\Partition1\Asset\CHAR_ShaoKahn.xxx' with '...\mods\DarkKahn\assets\Asset\CHAR_ShaoKahn.xxx'
   ```
2. **Package Loaded**: The engine loaded the file data into memory.
3. **Missing Playable Script Table**: Because Dark Kahn was developed strictly as an NPC boss for Story Mode, `CHAR_DarkKahn.xxx` contains no playable character move definitions or player data. 
4. **Lookup Failure**:
   ```
   OnCheckPlayerData: table=0x4496FC20, searching for 'char_shaokahn_player_data'
   OnCheckPlayerData: table has 0 entries, array_ptr=0x00000000
   OnCheckPlayerDataResult: sub_826A46E0 returned 0x00000000
   ```
5. **NULL Dereference**:
   `sub_82681490` attempted to dereference `*(player_struct + 128)`:
   ```cpp
   ctx.r11.u64 = REX_LOAD_U32(ctx.r15.u32 + 128); // r11 = 0
   ctx.r4.u64  = REX_LOAD_U32(ctx.r11.u32 + 0);   // CRASH: read of guest 0x00000000
   ```
   Guest access violation occurred at host PC `0x7ff63d102a65` (RVA `+0x24D2A65`, `mkvdc_recomp.104.cpp:11766`).

---

## 3. Package Structure Comparison

Decompressing both packages with the LZO1X decompressor revealed:

| Property | `CHAR_ShaoKahn.xxx` | `CHAR_DarkKahn.xxx` |
| :--- | :--- | :--- |
| **Uncompressed Size** | 22,569,725 bytes | 25,683,491 bytes |
| **LZO Chunks** | 22 chunks | 25 chunks |
| **Names Count** | 1,420 | 1,447 |
| **Exports Count** | 2,185 | 3,605 |
| **Imports Count** | 156 | 171 |
| **Base Skeleton Rig** | `BaseMale_Medium_Muscles.dkf` | `BaseMale_Medium_Muscles.dkf` |
| **Skeletal Mesh Object** | `Meshes.ShaoKahn` | `Meshes.DarkKahn` |
| **Primary Material** | `MK8_Mat_Character_3MerialKske` | `MK8_Mat_DarkKahn` |
| **Textures** | `ShaoKahn_Diff/Spec/NormHQA/Pmsk` | `DarkKahn_Diff/Spec/NormHQA/Pmsk/Reflection` |
| **Accessories** | `ShaoKahn_Dome_smesh`, Cape cloth | None (integrated single mesh) |
| **Fighting Styles** | `Fstyle_shaokahn`, `Fstyle_ShaokahnBoxing` | `Fstyle_darkkahn`, `Fstyle_SupermanBoxing` |
| **Playable Script Data** | Present (94 elements) | **Absent (0 elements)** |

Both characters share the identical base skeleton (`BaseMale_Medium_Muscles.dkf`), confirming that skeletal animations are interchangeable. However, Dark Kahn cannot function without a full synthetic script table and accessory detachment logic.

---

## 4. Decision & Parking Notice

- **Shao Kahn Unlocked & Functional**: The boss unlock logic in `sub_82634318` remains active and verified. The boss slot can be toggled and played cleanly with stock Shao Kahn assets.
- **Dark Kahn Parked**: Full Dark Kahn mesh and move injection is deferred to **Phase M (Character Mod Support)**.
- **Immediate Next Step**: Proceed to **Phase L (Modding System Completion)** to complete generic mod discovery, priority handling, and multi-mod support.
