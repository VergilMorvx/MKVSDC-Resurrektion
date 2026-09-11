#!/usr/bin/env python3
"""
harvest_vtables.py - Systemic RTTI-agnostic VTable Harvester for MKvDC

Scans .text for class constructors materializing .rdata vtable pointers,
discovers all virtual methods across all classes, validates boundaries
via Capstone PPC disassembly, and emits config/vtables.toml.
"""

import os
import sys
import bisect
import json
import capstone

BASE_ADDR = 0x82000000
BIN_PATH = r"D:\mkvsdc\default_mapped.bin"
PARTITION_PATH = r"D:\mkvsdc\mkvsdc-recomp\generated\default\codegen.partition.json"
OUT_TOML_PATH = r"D:\mkvsdc\mkvsdc-recomp\config\vtables.toml"

def main():
    print("[*] Loading mapped binary...")
    with open(BIN_PATH, "rb") as f:
        bin_data = f.read()

    print("[*] Loading known recompiler functions...")
    with open(PARTITION_PATH, "r") as f:
        part_data = json.load(f)
    known = set(int(k, 16) for k in part_data["assignments"].keys())
    sorted_known = sorted(list(known))

    md = capstone.Cs(capstone.CS_ARCH_PPC, capstone.CS_MODE_BIG_ENDIAN)

    print("[*] Scanning .text for constructor vtable assignments...")
    vtables = set()
    for off in range(0x250000, len(bin_data) - 8, 4):
        w1 = int.from_bytes(bin_data[off : off + 4], "big")
        w2 = int.from_bytes(bin_data[off + 4 : off + 8], "big")
        # lis rD, HIGH (opcode 15, rA == 0)
        if (w1 >> 26) == 15 and ((w1 >> 16) & 0x1F) == 0:
            rD1 = (w1 >> 21) & 0x1F
            imm1 = w1 & 0xFFFF
            if 0x8200 <= imm1 <= 0x8225:
                # addi rD, rA, LOW (opcode 14)
                if (w2 >> 26) == 14:
                    rD2 = (w2 >> 21) & 0x1F
                    rA2 = (w2 >> 16) & 0x1F
                    imm2 = w2 & 0xFFFF
                    if imm2 >= 0x8000:
                        imm2 -= 0x10000
                    if rD1 == rA2:
                        vtable_addr = (imm1 << 16) + imm2
                        if 0x82000600 <= vtable_addr < 0x82250000:
                            voff = vtable_addr - BASE_ADDR
                            first_slot = int.from_bytes(bin_data[voff : voff + 4], "big")
                            if 0x82250000 <= first_slot < 0x82F3A000:
                                vtables.add(vtable_addr)

    print(f"[*] Discovered {len(vtables)} candidate class vtables.")

    all_vtable_funcs = set()
    for vt in sorted(vtables):
        voff = vt - BASE_ADDR
        while voff + 4 <= len(bin_data):
            slot = int.from_bytes(bin_data[voff : voff + 4], "big")
            if 0x82250000 <= slot < 0x82F3A000 and (slot & 3) == 0:
                all_vtable_funcs.add(slot)
                voff += 4
            else:
                break

    print(f"[*] Total vtable slots: {len(all_vtable_funcs)}")
    missing = sorted(list(all_vtable_funcs - known))
    print(f"[*] Unregistered vtable functions: {len(missing)}")

    toml_lines = [
        "# MKvDC ReXGlue VTable Configuration",
        "# Harvested systematically from 763 class constructors in .text",
        "# Resolves missing virtual leaf accessors, thunks, and interface dispatchers",
        ""
    ]

    for i, f in enumerate(missing):
        idx = bisect.bisect_right(sorted_known, f) - 1
        prev_func = sorted_known[idx]
        next_func = sorted_known[idx + 1] if idx + 1 < len(sorted_known) else 0x82F39000

        next_missing = missing[i + 1] if i + 1 < len(missing) else None
        effective_next = min(next_func, next_missing) if next_missing else next_func

        off = f - BASE_ADDR
        pc = f
        size = 0
        terminal = "unknown"

        for step in range(128):
            c_off = off + step * 4
            if c_off + 4 > len(bin_data):
                break
            raw = bin_data[c_off : c_off + 4]
            dis = list(md.disasm(raw, pc))
            if not dis:
                break
            insn = dis[0]
            pc += 4
            if insn.mnemonic in ("blr", "bctr") or insn.mnemonic == "b":
                terminal = insn.mnemonic
                size = (step + 1) * 4
                break
            if pc >= effective_next:
                terminal = f"reach bound 0x{effective_next:08X}"
                size = pc - f
                break

        if size == 0 or f + size > effective_next:
            print(f"[!] Error: invalid bounds for 0x{f:08X}: size 0x{size:X}, next 0x{effective_next:08X}")
            sys.exit(1)

        toml_lines.append(f"# Discovered via vtable; terminates with {terminal}")
        toml_lines.append(f"[functions.0x{f:08X}]")
        toml_lines.append(f"size = 0x{size:02X}")
        toml_lines.append(f'name = "sub_{f:08X}"')
        toml_lines.append("")

    with open(OUT_TOML_PATH, "w", encoding="utf-8") as out:
        out.write("\n".join(toml_lines))

    print(f"[+] Successfully wrote {len(missing)} functions to {OUT_TOML_PATH}")

if __name__ == "__main__":
    main()
