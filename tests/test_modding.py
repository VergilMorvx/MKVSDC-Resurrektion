import os
import sys
import subprocess
import time

try:
    import tomllib  # Python 3.11+
except ImportError:
    try:
        import tomli as tomllib
    except ImportError:
        tomllib = None

def test_toml_configs():
    print("[TEST] 1. Validating TOML files syntax...")
    toml_files = [
        "config/mods.toml",
        "mods/BossUnlock/mod.toml",
        "mods/DarkKahn/mod.toml",
        "mods/TestMod/mod.toml",
    ]
    for tf in toml_files:
        path = os.path.join(r"d:\mkvsdc\mkvsdc-recomp", tf)
        assert os.path.exists(path), f"File not found: {path}"
        with open(path, "rb") as f:
            content = f.read()
            if tomllib:
                parsed = tomllib.loads(content.decode("utf-8"))
                print(f"  OK: {tf} -> {list(parsed.keys())}")
            else:
                print(f"  OK: {tf} (exists, size={len(content)})")

def test_game_mod_manager_logs():
    print("[TEST] 2. Inspecting ModManager discovery and mounting in engine logs...")
    log_dir = r"d:\mkvsdc\mkvsdc-recomp\out\build\win-amd64-release\logs"
    assert os.path.exists(log_dir), f"Logs directory not found: {log_dir}"
    
    # Find latest log file
    log_files = [os.path.join(log_dir, f) for f in os.listdir(log_dir) if f.startswith("mkvdc_") and f.endswith(".log")]
    log_files.sort(key=lambda p: os.path.getmtime(p), reverse=True)
    assert log_files, "No mkvdc log files found in build directory!"
    latest_log = log_files[0]
    print(f"  Reading latest log: {os.path.basename(latest_log)}")
    
    with open(latest_log, "r", encoding="utf-8", errors="ignore") as f:
        lines = f.readlines()
    
    mod_lines = [l.strip() for l in lines if "ModManager" in l or "VFS Overlay" in l or "HostPathDevice" in l]
    print(f"  Found {len(mod_lines)} ModManager / VFS log entries:")
    for l in mod_lines:
        print(f"    {l}")
    
    # Verify expected events
    found_central = any("Loaded central configuration" in l for l in mod_lines)
    found_boss_unlock = any("boss_unlock" in l for l in mod_lines)
    found_test_mod = any("test_mod" in l for l in mod_lines)
    found_darkkahn = any("darkkahn" in l for l in mod_lines)
    found_overlay_mount = any("Mounting VFS overlay for mod 'test_mod'" in l for l in mod_lines)
    found_marker_file = any("test_mod_marker.txt" in l for l in mod_lines)
    found_guard = any("sub_826AC700" in l for l in mod_lines)
    
    # Verify that darkkahn was NOT mounted because it is disabled
    darkkahn_mounted = any("Mounting VFS overlay for mod 'darkkahn'" in l for l in mod_lines)
    
    print(f"  [CHECK] Central config loaded:        {found_central}")
    print(f"  [CHECK] boss_unlock discovered:       {found_boss_unlock}")
    print(f"  [CHECK] test_mod discovered:          {found_test_mod}")
    print(f"  [CHECK] darkkahn discovered (parked): {found_darkkahn}")
    print(f"  [CHECK] test_mod overlay mounted:     {found_overlay_mount}")
    print(f"  [CHECK] marker file registered:       {found_marker_file}")
    print(f"  [CHECK] safety guard registered:      {found_guard}")
    print(f"  [CHECK] darkkahn overlay skipped:     {not darkkahn_mounted}")
    
    assert found_central, "Central config was not loaded!"
    assert found_boss_unlock, "boss_unlock was not discovered!"
    assert found_test_mod, "test_mod was not discovered!"
    assert found_darkkahn, "darkkahn was not discovered!"
    assert found_overlay_mount, "test_mod overlay was not mounted!"
    assert found_marker_file, "test_mod_marker.txt was not added to VFS!"
    assert found_guard, "Safety guard was not registered!"
    assert not darkkahn_mounted, "darkkahn should not be mounted when disabled!"
    print("[PASS] Engine log verification completed successfully!")

if __name__ == "__main__":
    test_toml_configs()
    test_game_mod_manager_logs()
    print("\nALL PHASE L MODDING TESTS PASSED!")
