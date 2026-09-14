#include "mod_manager.h"

#include <rex/logging.h>
#include <rex/filesystem/vfs.h>
#include <rex/ppc/context.h>
#include <rex/ppc/func.h>
#include <rex/system/function_dispatcher.h>
#include <toml++/toml.h>

#include <algorithm>
#include <bit>
#include <fstream>

extern "C" {
void sub_82670208(PPCContext& ctx, uint8_t* base);
void sub_82683BE8(PPCContext& ctx, uint8_t* base);
}

namespace mkvsdc {

static void ModLog(const std::string& msg) {
  static std::ofstream s_mod_log("logs/mod_manager.log", std::ios::out | std::ios::app);
  if (s_mod_log.is_open()) {
    s_mod_log << msg << "\n";
    s_mod_log.flush();
  }
}

static PPCFunc* s_orig_sub_82638E38 = nullptr;
static PPCFunc* s_orig_sub_826AF0E0 = nullptr;
static PPCFunc* s_orig_sub_82634318 = nullptr;
static PPCFunc* s_orig_sub_826E1F90 = nullptr;
static PPCFunc* s_orig_sub_82680EB0 = nullptr;
static PPCFunc* s_orig_sub_8262FCD8 = nullptr;
static PPCFunc* s_orig_sub_826A46E0 = nullptr;
static PPCFunc* s_orig_sub_82681490 = nullptr;
static PPCFunc* s_orig_sub_82683BE8 = nullptr;
static PPCFunc* s_orig_sub_8269D0C8 = nullptr;
static PPCFunc* s_orig_sub_826AC700 = nullptr;

static void Hook_sub_826AC700(PPCContext& ctx, uint8_t* base) {
  uint32_t r3 = ctx.r3.u32;
  uint32_t r4 = ctx.r4.u32;
  uint32_t r5 = ctx.r5.u32;

  // Validate object and array bounds to prevent crashes on missing move descriptors
  uint32_t obj = r4;
  uint32_t idx = r5;

  if (idx & 0x80000000) {
    if (r3 >= 0x10000 && r3 <= 0x8FFFFFFF) {
      uint32_t r11_val = std::byteswap(*reinterpret_cast<const uint32_t*>(base + r3 + 112));
      if (r11_val >= 0x10000 && r11_val <= 0x8FFFFFFF) {
        uint32_t r10_val = (idx << 2);
        uint32_t r30_val = std::byteswap(*reinterpret_cast<const uint32_t*>(base + r11_val + r10_val - 4));
        if (r30_val >= 0x10000 && r30_val <= 0x8FFFFFFF) {
          idx = std::byteswap(*reinterpret_cast<const uint32_t*>(base + r30_val + 20));
        }
      }
    }
  }

  if (obj >= 0x10000 && obj <= 0x8FFFFFFF) {
    uint32_t count = std::byteswap(*reinterpret_cast<const uint32_t*>(base + obj + 32));
    if (idx <= count && idx > 0) {
      uint32_t arr = std::byteswap(*reinterpret_cast<const uint32_t*>(base + obj + 104));
      if (arr >= 0x10000 && arr <= 0x8FFFFFFF) {
        uint32_t elem = std::byteswap(*reinterpret_cast<const uint32_t*>(base + arr + (idx << 2) - 4));
        if (elem < 0x10000 || elem > 0x8FFFFFFF) {
          char buf[256];
          snprintf(buf, sizeof(buf), "Hook_sub_826AC700: PREVENTED CRASH! obj=0x%08X, idx=%u, arr=0x%08X, elem=0x%08X (invalid pointer!)",
                   obj, idx, arr, elem);
          ModLog(buf);
          return;
        }
        uint32_t elem_16 = std::byteswap(*reinterpret_cast<const uint32_t*>(base + elem + 16));
        if (elem_16 < 0x10000 || elem_16 > 0x8FFFFFFF) {
          char buf[256];
          snprintf(buf, sizeof(buf), "Hook_sub_826AC700: PREVENTED CRASH on elem+16! obj=0x%08X, elem=0x%08X, elem_16=0x%08X",
                   obj, elem, elem_16);
          ModLog(buf);
          return;
        }
      }
    }
  }

  if (s_orig_sub_826AC700) {
    s_orig_sub_826AC700(ctx, base);
  }
}

static void Hook_sub_82638E38(PPCContext& ctx, uint8_t* base) {
  if (s_orig_sub_82638E38) {
    s_orig_sub_82638E38(ctx, base);
  }
}

static void Hook_sub_826AF0E0(PPCContext& ctx, uint8_t* base) {
  if (s_orig_sub_826AF0E0) {
    s_orig_sub_826AF0E0(ctx, base);
  }
}

static void Hook_sub_82634318(PPCContext& ctx, uint8_t* base) {
  if (s_orig_sub_82634318) {
    s_orig_sub_82634318(ctx, base);
  }
}

static void Hook_sub_826E1F90(PPCContext& ctx, uint8_t* base) {
  if (s_orig_sub_826E1F90) {
    s_orig_sub_826E1F90(ctx, base);
  }
}

static void Hook_sub_82680EB0(PPCContext& ctx, uint8_t* base) {
  if (s_orig_sub_82680EB0) {
    s_orig_sub_82680EB0(ctx, base);
  }
}

static void Hook_sub_826A46E0(PPCContext& ctx, uint8_t* base) {
  if (s_orig_sub_826A46E0) {
    s_orig_sub_826A46E0(ctx, base);
  }
}

static void Hook_sub_82681490(PPCContext& ctx, uint8_t* base) {
  if (s_orig_sub_82681490) {
    s_orig_sub_82681490(ctx, base);
  }
}

static void Hook_sub_8262FCD8(PPCContext& ctx, uint8_t* base) {
  if (s_orig_sub_8262FCD8) {
    s_orig_sub_8262FCD8(ctx, base);
  }
}

static void Hook_sub_8269D0C8(PPCContext& ctx, uint8_t* base) {
  if (s_orig_sub_8269D0C8) {
    s_orig_sub_8269D0C8(ctx, base);
  }
}

static void Hook_sub_82683BE8(PPCContext& ctx, uint8_t* base) {
  if (s_orig_sub_82683BE8) {
    s_orig_sub_82683BE8(ctx, base);
  }
}

}  // namespace mkvsdc

extern "C" {

void OnPreInitMatchPlayer(PPCContext& ctx, uint8_t* base) {}

void OnPostInitMatchPlayer(PPCContext& ctx, uint8_t* base) {}

void OnGetPlayerStruct(PPCContext& ctx, uint8_t* base) {}

bool OnCheckCharacterUnlocked(PPCContext& ctx, uint8_t* base) {
  uint32_t fstr = ctx.r3.u32;
  uint32_t data_ptr = 0;
  const char* name = "<null>";
  if (fstr >= 0x10000 && fstr <= 0x8FFFFFFF) {
    data_ptr = std::byteswap(*reinterpret_cast<const uint32_t*>(base + fstr));
    if (data_ptr >= 0x10000 && data_ptr <= 0x8FFFFFFF) {
      name = reinterpret_cast<const char*>(base + data_ptr);
    }
  }
  bool boss_enabled = mkvsdc::ModManager::Instance().IsModEnabled("darkkahn") ||
                      mkvsdc::ModManager::Instance().IsModEnabled("boss_unlock");
  char dbg[256];
  snprintf(dbg, sizeof(dbg), "OnCheckCharacterUnlocked: fstr=0x%08X, data_ptr=0x%08X, name='%s', boss_enabled=%d",
           fstr, data_ptr, name, boss_enabled ? 1 : 0);
  mkvsdc::ModLog(dbg);

  if (boss_enabled) {
    if (_stricmp(name, "CHAR_ShaoKahn") == 0 || _stricmp(name, "CHAR_Darkseid") == 0 || _stricmp(name, "CHAR_darkkahn") == 0) {
      char buf[128];
      snprintf(buf, sizeof(buf), "OnCheckCharacterUnlocked: UNLOCKED boss character: %s", name);
      mkvsdc::ModLog(buf);
      return true;
    }
  }
  return false;
}

bool OnCheckCharacterLocked(PPCContext& ctx, uint8_t* base) {
  uint32_t fstr = ctx.r3.u32;
  uint32_t data_ptr = 0;
  const char* name = "<null>";
  if (fstr >= 0x10000 && fstr <= 0x8FFFFFFF) {
    data_ptr = std::byteswap(*reinterpret_cast<const uint32_t*>(base + fstr));
    if (data_ptr >= 0x10000 && data_ptr <= 0x8FFFFFFF) {
      name = reinterpret_cast<const char*>(base + data_ptr);
    }
  }
  bool boss_enabled = mkvsdc::ModManager::Instance().IsModEnabled("darkkahn") ||
                      mkvsdc::ModManager::Instance().IsModEnabled("boss_unlock");
  if (boss_enabled) {
    if (_stricmp(name, "CHAR_ShaoKahn") == 0 || _stricmp(name, "CHAR_Darkseid") == 0 || _stricmp(name, "CHAR_darkkahn") == 0) {
      char buf[128];
      snprintf(buf, sizeof(buf), "OnCheckCharacterLocked: BYPASSED LOCK for boss character: %s", name);
      mkvsdc::ModLog(buf);
      return true; // True tells caller that this boss is NOT locked (caller sets r3=0)
    }
  }
  return false;
}

uint32_t OnGetBossCharacterName(uint32_t stock_addr) {
  if (mkvsdc::ModManager::Instance().IsModEnabled("darkkahn")) {
    char buf[128];
    snprintf(buf, sizeof(buf), "OnGetBossCharacterName: Redirecting boss from 0x%08X to CHAR_darkkahn (0x820D3218)", stock_addr);
    mkvsdc::ModLog(buf);
    return 0x820D3218; // CHAR_darkkahn
  }
  return stock_addr;
}

void OnResolveCharacterMode(PPCContext& ctx, uint8_t* base) {}

void OnMatchPackageLoad(PPCContext& ctx, uint8_t* base) {}

void OnSpawnPawn(PPCContext& ctx, uint8_t* base) {}

void OnFatalError(PPCContext& ctx, uint8_t* base) {
  auto dump_str = [base](const char* name, uint64_t addr, int len = 128) {
    if (addr >= 0x00010000 && addr + len <= 0x90000000) {
      const uint8_t* p = base + (addr & 0xFFFFFFFF);
      char asc[130];
      for (int i = 0; i < len; ++i) {
        asc[i] = (p[i] >= 32 && p[i] < 127) ? (char)p[i] : (p[i] == 0 ? '|' : '.');
      }
      asc[len] = 0;
      char buf[256];
      snprintf(buf, sizeof(buf), "%s (0x%llX): '%s'", name, (unsigned long long)addr, asc);
      mkvsdc::ModLog(buf);
    }
  };

  mkvsdc::ModLog("=== FATAL ERROR INSPECTION ===");
  dump_str("r3", ctx.r3.u64, 120);
  dump_str("r4", ctx.r4.u64, 64);
  if (ctx.r4.u64 >= 0x00010000 && ctx.r4.u64 <= 0x90000000) {
    uint32_t data_ptr = *reinterpret_cast<uint32_t*>(base + (ctx.r4.u64 & 0xFFFFFFFF));
    data_ptr = std::byteswap(data_ptr);
    dump_str("r4->Data", data_ptr, 120);
  }
}

void OnLoadPackage(PPCContext& ctx, uint8_t* base) {
  const char* fn = "<null>";
  if (ctx.r4.u32 >= 0x00010000 && ctx.r4.u32 <= 0x90000000) {
    fn = reinterpret_cast<const char*>(base + ctx.r4.u32);
  }
  char buf[256];
  snprintf(buf, sizeof(buf), "OnLoadPackage: InOuter=0x%08X, InFilename=0x%08X ('%s'), flags=0x%llX, LR=0x%llX",
           ctx.r3.u32, ctx.r4.u32, fn, (unsigned long long)ctx.r5.u64, (unsigned long long)ctx.lr);
  mkvsdc::ModLog(buf);
}

void OnFindPackageFileResult(PPCContext& ctx, uint8_t* base) {
  uint32_t pkg_ptr = std::byteswap(*reinterpret_cast<uint32_t*>(base + ctx.r31.u32 + 80));
  const char* pkg_name = "<null>";
  if (pkg_ptr >= 0x00010000 && pkg_ptr <= 0x90000000) {
    pkg_name = reinterpret_cast<const char*>(base + pkg_ptr);
  }
  uint32_t out_num = std::byteswap(*reinterpret_cast<uint32_t*>(base + ctx.r31.u32 + 100));
  uint32_t out_ptr = std::byteswap(*reinterpret_cast<uint32_t*>(base + ctx.r31.u32 + 96));
  const char* out_fn = "<empty>";
  if (out_ptr >= 0x00010000 && out_ptr <= 0x90000000) {
    out_fn = reinterpret_cast<const char*>(base + out_ptr);
  }
  char buf[256];
  snprintf(buf, sizeof(buf), "OnFindPackageFileResult: pkg='%s' (ptr=0x%08X), ret=%d, out_fn='%s' (num=%u, ptr=0x%08X)",
           pkg_name, pkg_ptr, ctx.r3.s32, out_fn, out_num, out_ptr);
  mkvsdc::ModLog(buf);
}

void OnTickLinker(PPCContext& ctx, uint8_t* base) {
  uint32_t linker = ctx.r3.u32;
  uint32_t fname_ptr = 0;
  uint32_t fname_num = 0;
  const char* fn = "<null>";
  if (linker >= 0x00010000 && linker <= 0x90000000) {
    fname_ptr = std::byteswap(*reinterpret_cast<uint32_t*>(base + linker + 308));
    fname_num = std::byteswap(*reinterpret_cast<uint32_t*>(base + linker + 312));
    if (fname_ptr >= 0x00010000 && fname_ptr <= 0x90000000) {
      fn = reinterpret_cast<const char*>(base + fname_ptr);
    }
  }
  char buf[256];
  snprintf(buf, sizeof(buf), "OnTickLinker: linker=0x%08X, fn='%s' (num=%u, ptr=0x%08X), LR=0x%llX",
           linker, fn, fname_num, fname_ptr, (unsigned long long)ctx.lr);
  mkvsdc::ModLog(buf);
}

void OnGetPackageLinker(PPCContext& ctx, uint8_t* base) {
  uint32_t in_outer = ctx.r3.u32;
  uint32_t in_fn_ptr = ctx.r4.u32;
  const char* in_fn = "<null>";
  if (in_fn_ptr >= 0x00010000 && in_fn_ptr <= 0x90000000) {
    in_fn = reinterpret_cast<const char*>(base + in_fn_ptr);
  }
  uint32_t name_idx = 0;
  if (in_outer >= 0x00010000 && in_outer <= 0x90000000) {
    name_idx = std::byteswap(*reinterpret_cast<uint32_t*>(base + in_outer + 0x20));
  }
  char buf[256];
  snprintf(buf, sizeof(buf), "OnGetPackageLinker: InOuter=0x%08X (nameIdx=%u), InFilename=0x%08X ('%s'), flags=0x%llX, LR=0x%llX",
           in_outer, name_idx, in_fn_ptr, in_fn, (unsigned long long)ctx.r5.u64, (unsigned long long)ctx.lr);
  mkvsdc::ModLog(buf);
}

void OnAsyncPackageTick(PPCContext& ctx, uint8_t* base) {
  uint32_t async_pkg = ctx.r3.u32;
  uint32_t pkg_ptr = 0;
  uint32_t pkg_len = 0;
  const char* pkg_str = "<null>";
  if (async_pkg >= 0x00010000 && async_pkg <= 0x90000000) {
    pkg_ptr = std::byteswap(*reinterpret_cast<uint32_t*>(base + async_pkg + 4));
    pkg_len = std::byteswap(*reinterpret_cast<uint32_t*>(base + async_pkg + 8));
    if (pkg_ptr >= 0x00010000 && pkg_ptr <= 0x90000000) {
      pkg_str = reinterpret_cast<const char*>(base + pkg_ptr);
    }
  }
  char buf[256];
  snprintf(buf, sizeof(buf), "OnAsyncPackageTick: asyncPkg=0x%08X, name='%s' (len=%u, ptr=0x%08X), LR=0x%llX",
           async_pkg, pkg_str, pkg_len, pkg_ptr, (unsigned long long)ctx.lr);
  mkvsdc::ModLog(buf);
}

extern void sub_82254E50(PPCContext& ctx, uint8_t* base);

bool OnFindPackageFallback(PPCContext& ctx, uint8_t* base) {
  uint32_t pkg_ptr = ctx.r29.u32;
  const char* pkg_name = "<null>";
  if (pkg_ptr >= 0x00010000 && pkg_ptr <= 0x90000000) {
    pkg_name = reinterpret_cast<const char*>(base + pkg_ptr);
  }

  std::string target_path = "";
  if (_stricmp(pkg_name, "darkkahn") == 0 || _stricmp(pkg_name, "char_darkkahn") == 0 ||
      _stricmp(pkg_name, "CHAR_DarkKahn") == 0) {
    target_path = "..\\Asset\\CHAR_DarkKahn.xxx";
  } else if (_stricmp(pkg_name, "fx_darkkahn") == 0 || _stricmp(pkg_name, "FX_DarkKahn") == 0) {
    target_path = "..\\Asset\\FX_DarkKahn.xxx";
  } else if (_stricmp(pkg_name, "snd_vo_dkkn") == 0 || _stricmp(pkg_name, "snd_vo_darkkahn") == 0) {
    target_path = "..\\Asset\\snd_vo_dkkn.xxx";
  } else if (_stricmp(pkg_name, "snd_sfx_dkkn") == 0 || _stricmp(pkg_name, "snd_sfx_darkkahn") == 0) {
    target_path = "..\\Asset\\snd_sfx_dkkn.xxx";
  } else if (_stricmp(pkg_name, "snd_sfx_dark") == 0) {
    target_path = "..\\Asset\\snd_sfx_dark.xxx";
  } else if (_stricmp(pkg_name, "snd_foley_dkkn") == 0 || _stricmp(pkg_name, "snd_foley_darkkahn") == 0) {
    target_path = "..\\Asset\\snd_foley_dkkn.xxx";
  }

  if (!target_path.empty()) {
    char buf[256];
    snprintf(buf, sizeof(buf), "OnFindPackageFallback: RESOLVED '%s' -> '%s' (outFStr=0x%08X)",
             pkg_name, target_path.c_str(), ctx.r22.u32);
    mkvsdc::ModLog(buf);

    uint32_t scratch_addr = 0x83008500;
    char* guest_scratch = reinterpret_cast<char*>(base + scratch_addr);
    memcpy(guest_scratch, target_path.c_str(), target_path.size() + 1);

    PPCContext call_ctx = ctx;
    call_ctx.r3.u64 = ctx.r22.u64;
    call_ctx.r4.u64 = scratch_addr;
    sub_82254E50(call_ctx, base);

    ctx.r3.s64 = 1;
    return true;
  }

  return false;
}

void OnCheckPlayerData(PPCContext& ctx, uint8_t* base) {
  uint32_t table_ptr = ctx.r3.u32;
  uint32_t search_str_ptr = ctx.r4.u32;
  const char* search_str = "<null>";
  if (search_str_ptr >= 0x10000 && search_str_ptr <= 0x8FFFFFFF) {
    search_str = reinterpret_cast<const char*>(base + search_str_ptr);
  }

  char buf[512];
  snprintf(buf, sizeof(buf), "OnCheckPlayerData: table=0x%08X, searching for '%s' (ptr=0x%08X)",
           table_ptr, search_str, search_str_ptr);
  mkvsdc::ModLog(buf);

  // Dump table entries if valid
  if (table_ptr >= 0x10000 && table_ptr <= 0x8FFFFFFF) {
    uint32_t count = std::byteswap(*reinterpret_cast<const uint32_t*>(base + table_ptr + 36));
    uint32_t array_ptr = std::byteswap(*reinterpret_cast<const uint32_t*>(base + table_ptr + 108));
    snprintf(buf, sizeof(buf), "OnCheckPlayerData: table has %u entries, array_ptr=0x%08X", count, array_ptr);
    mkvsdc::ModLog(buf);

    if (array_ptr >= 0x10000 && array_ptr <= 0x8FFFFFFF && count < 1000) {
      for (uint32_t i = 0; i < count; ++i) {
        uint32_t entry_ptr = std::byteswap(*reinterpret_cast<const uint32_t*>(base + array_ptr + i * 4));
        if (entry_ptr >= 0x10000 && entry_ptr <= 0x8FFFFFFF) {
          uint32_t name_ptr = std::byteswap(*reinterpret_cast<const uint32_t*>(base + entry_ptr + 0));
          uint32_t val_ptr = std::byteswap(*reinterpret_cast<const uint32_t*>(base + entry_ptr + 12));
          const char* entry_name = "<null>";
          if (name_ptr >= 0x10000 && name_ptr <= 0x8FFFFFFF) {
            entry_name = reinterpret_cast<const char*>(base + name_ptr);
          }
          snprintf(buf, sizeof(buf), "  entry[%u]: ptr=0x%08X, name='%s' (0x%08X), data=0x%08X",
                   i, entry_ptr, entry_name, name_ptr, val_ptr);
          mkvsdc::ModLog(buf);
        }
      }
    }
  }
}

void OnCheckPlayerDataResult(PPCContext& ctx, uint8_t* base) {
  char buf[256];
  snprintf(buf, sizeof(buf), "OnCheckPlayerDataResult: sub_826A46E0 returned 0x%08X", ctx.r3.u32);
  mkvsdc::ModLog(buf);
}

void OnPreLoadCharacterMesh(PPCContext& ctx, uint8_t* base) {
  uint32_t pstruct = ctx.r15.u32;
  if (pstruct >= 0x10000 && pstruct <= 0x8FFFFFFF) {
    uint32_t cdata = std::byteswap(*reinterpret_cast<const uint32_t*>(base + pstruct + 128));
    if (cdata >= 0x10000 && cdata <= 0x8FFFFFFF) {
      uint32_t mesh0_ptr = std::byteswap(*reinterpret_cast<const uint32_t*>(base + cdata + 0));
      uint32_t mesh1_ptr = std::byteswap(*reinterpret_cast<const uint32_t*>(base + cdata + 4));
      const char* m0 = (mesh0_ptr >= 0x10000 && mesh0_ptr <= 0x8FFFFFFF) ? reinterpret_cast<const char*>(base + mesh0_ptr) : "<null>";
      const char* m1 = (mesh1_ptr >= 0x10000 && mesh1_ptr <= 0x8FFFFFFF) ? reinterpret_cast<const char*>(base + mesh1_ptr) : "<null>";
      char buf[256];
      snprintf(buf, sizeof(buf), "STOCK COSTUME DATA: cdata=0x%08X, mesh0='%s' (0x%08X), mesh1='%s' (0x%08X)",
               cdata, m0, mesh0_ptr, m1, mesh1_ptr);
      mkvsdc::ModLog(buf);
    }
  }
}

void OnPreStaticLoadMesh(PPCContext& ctx, uint8_t* base) {
  char buf[256];
  snprintf(buf, sizeof(buf), "OnPreStaticLoadMesh: r3=0x%08X, r4=0x%08X, r5=0x%08X", ctx.r3.u32, ctx.r4.u32, ctx.r5.u32);
  mkvsdc::ModLog(buf);
}
void OnPostStaticLoadMesh(PPCContext& ctx, uint8_t* base) {
  char buf[256];
  snprintf(buf, sizeof(buf), "OnPostStaticLoadMesh: returned r3=0x%08X", ctx.r3.u32);
  mkvsdc::ModLog(buf);
}

void OnStringAssign(PPCContext& ctx, uint8_t* base) {
  uint32_t src_ptr = ctx.r4.u32;
  if (src_ptr < 0x00010000 || src_ptr > 0x8FFFFFFF || src_ptr == 0x5F446172) {
    char buf[256];
    snprintf(buf, sizeof(buf), "sub_82254E50 BAD: src=0x%08X, this=0x%08X, LR=0x%08X",
             src_ptr, ctx.r3.u32, static_cast<uint32_t>(ctx.lr));
    mkvsdc::ModLog(buf);
    base[0x830085F0] = 0;
    ctx.r4.u64 = 0x830085F0;
  }
}

void OnCheckStricmp(PPCContext& ctx, uint8_t* base) {
  uint32_t r3 = ctx.r3.u32;
  uint32_t r4 = ctx.r4.u32;
  bool bad_r3 = (r3 < 0x00010000 || r3 > 0x8FFFFFFF || r3 == 0x5F446172);
  bool bad_r4 = (r4 < 0x00010000 || r4 > 0x8FFFFFFF || r4 == 0x5F446172);
  if (bad_r3 || bad_r4) {
    const char* s3 = (!bad_r3) ? reinterpret_cast<const char*>(base + r3) : "<invalid>";
    const char* s4 = (!bad_r4) ? reinterpret_cast<const char*>(base + r4) : "<invalid>";
    char buf[256];
    snprintf(buf, sizeof(buf), "OnCheckStricmp: BAD POINTER! r3=0x%08X ('%s'), r4=0x%08X ('%s'), LR=0x%llX",
             r3, s3, r4, s4, (unsigned long long)ctx.lr);
    mkvsdc::ModLog(buf);
    base[0x830085F0] = 0;
    if (bad_r3) ctx.r3.u64 = 0x830085F0;
    if (bad_r4) ctx.r4.u64 = 0x830085F0;
  }
}

void OnCheckPawn(PPCContext& ctx, uint8_t* base) {}

void OnTraceMeshStep(PPCContext& ctx, uint8_t* base, int step) {}

}  // extern "C"



namespace mkvsdc {



ModManager& ModManager::Instance() {
  static ModManager s_instance;
  return s_instance;
}

void ModManager::AddSearchPath(const std::filesystem::path& path) {
  if (std::find(search_paths_.begin(), search_paths_.end(), path) == search_paths_.end()) {
    search_paths_.push_back(path);
  }
}

void ModManager::ClearSearchPaths() {
  search_paths_.clear();
}

void ModManager::SortModsByPriority() {
  // Stable sort by priority ascending so lowest priority mounts first and highest mounts last (winning file conflicts)
  std::stable_sort(mods_.begin(), mods_.end(), [](const ModManifest& a, const ModManifest& b) {
    return a.priority < b.priority;
  });
}

bool ModManager::DiscoverMods(const std::filesystem::path& central_config_path) {
  return DiscoverMods(search_paths_, central_config_path);
}

bool ModManager::DiscoverMods(const std::filesystem::path& mods_dir,
                              const std::filesystem::path& central_config_path) {
  std::vector<std::filesystem::path> paths = {mods_dir};
  return DiscoverMods(paths, central_config_path);
}

bool ModManager::DiscoverMods(const std::vector<std::filesystem::path>& search_paths,
                              const std::filesystem::path& central_config_path) {
  search_paths_ = search_paths;
  central_config_path_ = central_config_path;
  mods_.clear();
  master_enabled_ = true;

  std::error_code ec;

  // 1. Read central config if present
  std::unordered_map<std::string, bool> config_mod_enabled;
  std::unordered_map<std::string, int> config_mod_priority;

  if (!central_config_path_.empty() && std::filesystem::exists(central_config_path_, ec)) {
    try {
      auto tbl = toml::parse_file(central_config_path_.string());
      if (auto mods_tbl = tbl.get_as<toml::table>("mods")) {
        if (auto master_val = mods_tbl->get_as<bool>("enabled")) {
          master_enabled_ = master_val->get();
        }
        for (const auto& [key, node] : *mods_tbl) {
          if (node.is_table()) {
            const auto& mod_tbl = *node.as_table();
            std::string mod_key(key.str());
            if (auto enabled_node = mod_tbl.get_as<bool>("enabled")) {
              config_mod_enabled[mod_key] = enabled_node->get();
            }
            if (auto priority_node = mod_tbl.get_as<int64_t>("priority")) {
              config_mod_priority[mod_key] = static_cast<int>(priority_node->get());
            }
          }
        }
      }
      REXSYS_INFO("ModManager: Loaded central configuration from {}", central_config_path_.string());
    } catch (const std::exception& e) {
      REXSYS_WARN("ModManager: Failed to parse central config {}: {}", central_config_path_.string(),
                  e.what());
    }
  }

  // 2. Discover mod directories across all search paths
  for (const auto& search_path : search_paths_) {
    if (!std::filesystem::exists(search_path, ec)) {
      std::filesystem::create_directories(search_path, ec);
    }
    if (!std::filesystem::is_directory(search_path, ec)) {
      continue;
    }

    for (const auto& dir_entry : std::filesystem::directory_iterator(search_path, ec)) {
      if (!dir_entry.is_directory()) {
        continue;
      }

      auto mod_root = dir_entry.path();
      auto manifest_path = mod_root / "mod.toml";
      if (!std::filesystem::exists(manifest_path, ec)) {
        continue;
      }

      ModManifest manifest;
      manifest.root_dir = mod_root;
      manifest.id = mod_root.filename().string();
      manifest.name = manifest.id;

      try {
        auto manifest_tbl = toml::parse_file(manifest_path.string());
        if (auto id_node = manifest_tbl.get_as<std::string>("id")) {
          manifest.id = id_node->get();
        }
        if (auto name_node = manifest_tbl.get_as<std::string>("name")) {
          manifest.name = name_node->get();
        }
        if (auto ver_node = manifest_tbl.get_as<std::string>("version")) {
          manifest.version = ver_node->get();
        }
        if (auto auth_node = manifest_tbl.get_as<std::string>("author")) {
          manifest.author = auth_node->get();
        }
        if (auto desc_node = manifest_tbl.get_as<std::string>("description")) {
          manifest.description = desc_node->get();
        }
        if (auto enabled_node = manifest_tbl.get_as<bool>("enabled")) {
          manifest.enabled = enabled_node->get();
        }
        if (auto prio_node = manifest_tbl.get_as<int64_t>("priority")) {
          manifest.priority = static_cast<int>(prio_node->get());
        }
      } catch (const std::exception& e) {
        REXSYS_WARN("ModManager: Error parsing mod manifest {}: {}", manifest_path.string(), e.what());
        continue;
      }

      // Check for assets directory
      auto assets_dir = mod_root / "assets";
      if (std::filesystem::is_directory(assets_dir, ec)) {
        manifest.assets_dir = assets_dir;
        manifest.has_assets = true;
      } else {
        manifest.has_assets = false;
      }

      // Central config overrides manifest defaults
      if (auto it = config_mod_enabled.find(manifest.id); it != config_mod_enabled.end()) {
        manifest.enabled = it->second;
      }
      if (auto it = config_mod_priority.find(manifest.id); it != config_mod_priority.end()) {
        manifest.priority = it->second;
      }

      // Deduplicate if mod ID already encountered (replace if higher priority or from newer search path)
      auto existing = std::find_if(mods_.begin(), mods_.end(),
                                   [&manifest](const ModManifest& m) { return m.id == manifest.id; });
      if (existing != mods_.end()) {
        REXSYS_INFO("ModManager: Replacing earlier discovery of mod '{}' with version at {}",
                    manifest.id, manifest.root_dir.string());
        *existing = manifest;
      } else {
        REXSYS_INFO("ModManager: Discovered mod '{}' ({}) - v{}, enabled: {}, priority: {}, has_assets: {}",
                    manifest.name, manifest.id, manifest.version, manifest.enabled ? "yes" : "no",
                    manifest.priority, manifest.has_assets ? "yes" : "no");
        mods_.push_back(manifest);
      }
    }
  }

  SortModsByPriority();
  return true;
}

bool ModManager::Reload() {
  return DiscoverMods(search_paths_, central_config_path_);
}

void ModManager::ApplyVfsOverlays(rex::filesystem::VirtualFileSystem* vfs) {
  if (!vfs) {
    REXSYS_ERROR("ModManager: VFS pointer is null");
    return;
  }

  if (!master_enabled_) {
    REXSYS_INFO("ModManager: Master mod switch is disabled. Skipping all mod VFS overlays.");
    return;
  }

  for (const auto& mod : mods_) {
    if (!mod.enabled) {
      REXSYS_DEBUG("ModManager: Skipping disabled mod '{}'", mod.id);
      continue;
    }

    if (!mod.has_assets || mod.assets_dir.empty()) {
      REXSYS_DEBUG("ModManager: Mod '{}' has no assets directory, skipping VFS overlay", mod.id);
      continue;
    }

    REXSYS_INFO("ModManager: Mounting VFS overlay for mod '{}' (priority {}) from {}", mod.id,
                mod.priority, mod.assets_dir.string());
    if (!vfs->AddOverlay("\\Device\\Harddisk0\\Partition1", mod.assets_dir)) {
      REXSYS_WARN("ModManager: Failed to mount VFS overlay for mod '{}'", mod.id);
    }
  }
}

void ModManager::ApplyHooks(rex::runtime::FunctionDispatcher* dispatcher, uint8_t* base) {
  if (!dispatcher) {
    REXSYS_ERROR("ModManager: dispatcher is null");
    return;
  }

  // Hook safety checks
  s_orig_sub_826AC700 = dispatcher->GetFunction(0x826AC700);
  if (s_orig_sub_826AC700) {
    dispatcher->SetFunction(0x826AC700, Hook_sub_826AC700);
    REXSYS_INFO("ModManager: Hooked sub_826AC700 (move table safety guard)");
  }
}

bool ModManager::IsModEnabled(const std::string& mod_id) const {
  if (!master_enabled_) {
    return false;
  }
  for (const auto& mod : mods_) {
    if (mod.id == mod_id) {
      return mod.enabled;
    }
  }
  return false;
}

const ModManifest* ModManager::GetMod(size_t index) const {
  if (index < mods_.size()) {
    return &mods_[index];
  }
  return nullptr;
}

const ModManifest* ModManager::FindMod(const std::string& mod_id) const {
  for (const auto& mod : mods_) {
    if (mod.id == mod_id) {
      return &mod;
    }
  }
  return nullptr;
}

bool ModManager::SetModEnabled(const std::string& mod_id, bool enabled) {
  bool found = false;
  for (auto& mod : mods_) {
    if (mod.id == mod_id) {
      mod.enabled = enabled;
      found = true;
      break;
    }
  }
  if (found) {
    SaveCentralConfig();
  }
  return found;
}

bool ModManager::SetModPriority(const std::string& mod_id, int priority) {
  bool found = false;
  for (auto& mod : mods_) {
    if (mod.id == mod_id) {
      mod.priority = priority;
      found = true;
      break;
    }
  }
  if (found) {
    SortModsByPriority();
    SaveCentralConfig();
  }
  return found;
}

bool ModManager::SetMasterEnabled(bool enabled) {
  master_enabled_ = enabled;
  SaveCentralConfig();
  return true;
}

bool ModManager::SaveCentralConfig() {
  if (central_config_path_.empty()) {
    return false;
  }

  std::error_code ec;
  auto config_parent = central_config_path_.parent_path();
  if (!config_parent.empty() && !std::filesystem::exists(config_parent, ec)) {
    std::filesystem::create_directories(config_parent, ec);
  }

  std::ofstream out(central_config_path_, std::ios::out | std::ios::trunc);
  if (!out.is_open()) {
    REXSYS_ERROR("ModManager: Failed to write central config to {}", central_config_path_.string());
    return false;
  }

  out << "# MKVSDC: Resurrektion - Mod Manager Configuration\n";
  out << "# This file can be edited manually or manipulated by the MKVSDC Mod Launcher.\n\n";
  out << "[mods]\n";
  out << "enabled = " << (master_enabled_ ? "true" : "false") << "\n\n";

  for (const auto& mod : mods_) {
    out << "[mods." << mod.id << "]\n";
    out << "enabled = " << (mod.enabled ? "true" : "false") << "\n";
    out << "priority = " << mod.priority << "\n\n";
  }

  out.flush();
  REXSYS_INFO("ModManager: Saved central config to {}", central_config_path_.string());
  return true;
}

}  // namespace mkvsdc
