#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <rex/ppc/context.h>

namespace rex::filesystem {
class VirtualFileSystem;
}

namespace rex::runtime {
class FunctionDispatcher;
}

extern "C" {
void OnGetPlayerStruct(PPCContext& ctx, uint8_t* base);
bool OnCheckCharacterUnlocked(PPCContext& ctx, uint8_t* base);
bool OnCheckCharacterLocked(PPCContext& ctx, uint8_t* base);
uint32_t OnGetBossCharacterName(uint32_t stock_addr);
void OnResolveCharacterMode(PPCContext& ctx, uint8_t* base);
void OnMatchPackageLoad(PPCContext& ctx, uint8_t* base);
void OnSpawnPawn(PPCContext& ctx, uint8_t* base);
void OnLoadPackage(PPCContext& ctx, uint8_t* base);
void OnFindPackageFileResult(PPCContext& ctx, uint8_t* base);
void OnTickLinker(PPCContext& ctx, uint8_t* base);
void OnGetPackageLinker(PPCContext& ctx, uint8_t* base);
void OnAsyncPackageTick(PPCContext& ctx, uint8_t* base);
bool OnFindPackageFallback(PPCContext& ctx, uint8_t* base);
void OnCheckPlayerData(PPCContext& ctx, uint8_t* base);
void OnCheckPlayerDataResult(PPCContext& ctx, uint8_t* base);
void OnPreLoadCharacterMesh(PPCContext& ctx, uint8_t* base);
void OnStringAssign(PPCContext& ctx, uint8_t* base);
void OnCheckStricmp(PPCContext& ctx, uint8_t* base);
void OnCheckPawn(PPCContext& ctx, uint8_t* base);
void OnTraceMeshStep(PPCContext& ctx, uint8_t* base, int step);
void OnPreInitMatchPlayer(PPCContext& ctx, uint8_t* base);
void OnPostInitMatchPlayer(PPCContext& ctx, uint8_t* base);
}

namespace mkvsdc {

struct ModManifest {
  std::string id;
  std::string name;
  std::string version;
  std::string author;
  std::string description;
  bool enabled = true;
  int priority = 0;
  std::filesystem::path root_dir;
  std::filesystem::path assets_dir;
  bool has_assets = false;
};

class ModManager {
 public:
  static ModManager& Instance();

  // Search paths management
  void AddSearchPath(const std::filesystem::path& path);
  void ClearSearchPaths();
  const std::vector<std::filesystem::path>& GetSearchPaths() const { return search_paths_; }

  // Scans configured mod search paths and applies config/mods.toml overrides
  bool DiscoverMods(const std::filesystem::path& central_config_path = "");
  bool DiscoverMods(const std::filesystem::path& mods_dir,
                    const std::filesystem::path& central_config_path);
  bool DiscoverMods(const std::vector<std::filesystem::path>& search_paths,
                    const std::filesystem::path& central_config_path);

  // Reloads mods and config
  bool Reload();

  // Mounts enabled mod assets into the VFS partition
  void ApplyVfsOverlays(rex::filesystem::VirtualFileSystem* vfs);

  // Registers runtime guest hooks for active mods
  void ApplyHooks(rex::runtime::FunctionDispatcher* dispatcher, uint8_t* base = nullptr);

  // Status and querying (for launcher / UI integration)
  bool IsMasterEnabled() const { return master_enabled_; }
  bool IsModEnabled(const std::string& mod_id) const;
  size_t GetModCount() const { return mods_.size(); }
  const ModManifest* GetMod(size_t index) const;
  const ModManifest* FindMod(const std::string& mod_id) const;
  const std::vector<ModManifest>& GetDiscoveredMods() const { return mods_; }

  // Modification and persistence
  bool SetModEnabled(const std::string& mod_id, bool enabled);
  bool SetModPriority(const std::string& mod_id, int priority);
  bool SetMasterEnabled(bool enabled);
  bool SaveCentralConfig();

 private:
  ModManager() = default;

  void SortModsByPriority();

  bool master_enabled_ = true;
  std::vector<std::filesystem::path> search_paths_;
  std::filesystem::path central_config_path_;
  std::vector<ModManifest> mods_;
};

}  // namespace mkvsdc
