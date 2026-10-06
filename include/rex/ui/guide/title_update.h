/**
 * @file        rex/ui/guide/title_update.h
 * @brief       Optional title updates: packages, sources, install and launch (RG-GDK-057)
 *
 * @copyright   Copyright (c) 2026 Tom Clay
 * @license     BSD 3-Clause License
 */

#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <rex/cvar.h>
#include <rex/image_info.h>

/// The title update the player turned on in the guide; 0 runs the original.
REXCVAR_DECLARE(int32_t, title_update);
/// Set on the executable a title update choice started, so it never hands back.
REXCVAR_DECLARE(bool, title_update_handoff);

namespace rex::ui::guide {

/// STFS content type of a title update package.
constexpr uint32_t kTitleUpdateContentType = 0x000B0000;

/// What a LIVE/CON/PIRS title update package says about itself.
struct TitleUpdatePackage {
  uint32_t content_type = 0;
  uint32_t title_id = 0;
  uint32_t media_id = 0;
  uint32_t base_version = 0;  ///< the executable version it updates (XEX version word)
  /// The content ID stored in the header (0x32C), which Xbox Unity lists as
  /// the update's hash: SHA-1 of 0x344 up to the header size rounded to 4 KB.
  /// That range holds the top hash table's hash, so it covers the whole file.
  std::array<uint8_t, 20> content_id{};
  bool content_id_valid = false;  ///< the header region hashes to content_id
  std::string display_name;
};

/// Reads a package from its first bytes (at least the header, 0xB000 suffices
/// for every title update seen). Nullopt, with `error`, if it isn't one.
std::optional<TitleUpdatePackage> ReadTitleUpdatePackage(std::span<const uint8_t> head,
                                                         std::string* error);
std::optional<TitleUpdatePackage> ReadTitleUpdatePackage(const std::filesystem::path& path,
                                                         std::string* error);

std::string HexString(std::span<const uint8_t> bytes);

/// Whether `package` is the update `expected` describes, for title `title_id`.
/// Empty when it is; otherwise what's wrong, for the player.
std::string CheckTitleUpdatePackage(const TitleUpdatePackage& package,
                                    const PPCTitleUpdate& expected, uint32_t title_id);

/// Where an installed update lives: <local>\title_updates\<version>, holding
/// the package as downloaded (mounted at update: without extracting).
std::filesystem::path TitleUpdateFolder(const std::filesystem::path& local_dir, uint32_t version);
/// The installed package for `version`, or empty.
std::filesystem::path FindInstalledTitleUpdate(const std::filesystem::path& local_dir,
                                               uint32_t version);

/// Checks `package_file` and copies it into the update's folder, replacing
/// any earlier copy. Empty on success, else the reason.
std::string InstallTitleUpdate(const std::filesystem::path& package_file,
                               const PPCTitleUpdate& expected, uint32_t title_id,
                               const std::filesystem::path& local_dir);

/// Progress of a download: bytes so far and the total (0 when unknown).
using DownloadProgress = std::function<void(uint64_t done, uint64_t total)>;

/// Downloads `url` to `destination`. Empty on success, else the reason.
std::string DownloadFile(const std::string& url, const std::filesystem::path& destination,
                         const DownloadProgress& progress, const std::atomic<bool>& cancel);

/// Fetches `url` into memory (small responses). Nullopt with `error` on failure.
std::optional<std::string> FetchText(const std::string& url, std::string* error);

/// A place a title update can be downloaded from. `find_url` returns the
/// package URL for `update` of `title_id`, or nullopt with the reason.
struct TitleUpdateSource {
  std::string name;
  std::function<std::optional<std::string>(const PPCTitleUpdate& update, uint32_t title_id,
                                           std::string* error)>
      find_url;
};

/// Xbox Unity: TitleUpdateInfo.php lists a title's updates per media ID with
/// their content IDs ("hash"); the one matching is TitleUpdate.php?tuid=.
TitleUpdateSource XboxUnitySource();

/// The TitleUpdateID Xbox Unity gives the update whose hash is `content_id`,
/// from a TitleUpdateInfo.php response. Nullopt when it isn't listed.
std::optional<std::string> FindXboxUnityUpdateId(std::string_view info_json,
                                                 std::string_view content_id);

/// The sources tried in order: Xbox Unity, then any the title_update_sources
/// setting lists (URL templates with {title_id}, {media_id}, {version},
/// {content_id}).
std::vector<TitleUpdateSource> TitleUpdateSources();

/// Downloads `update` from the first source that has it and installs it.
/// Each failed source's reason is appended to `errors`. True when installed.
bool DownloadTitleUpdate(const PPCTitleUpdate& update, uint32_t title_id,
                         const std::filesystem::path& local_dir,
                         const std::vector<TitleUpdateSource>& sources,
                         const DownloadProgress& progress, const std::atomic<bool>& cancel,
                         std::vector<std::string>* errors);

/// A title update download or install from a file, running in the background
/// so it outlives the guide. The guide's Title Updates and Active Downloads read
/// it; the fields are written by its thread.
struct TitleUpdateJob {
  enum class State { kRunning, kInstalled, kFailed, kCancelled };
  PPCTitleUpdate update{};
  bool from_file = false;  ///< installing a package the player chose
  std::atomic<uint64_t> done{0};
  std::atomic<uint64_t> total{0};
  std::atomic<State> state{State::kRunning};
  std::atomic<bool> cancel{false};
  /// Why it failed, one line per source tried; complete once state leaves kRunning.
  std::vector<std::string> errors;
};

/// Starts downloading `update` (TitleUpdateSources, in order) unless a job for
/// that version is already running, which is returned instead.
std::shared_ptr<TitleUpdateJob> StartTitleUpdateDownload(const PPCTitleUpdate& update,
                                                         uint32_t title_id,
                                                         const std::filesystem::path& local_dir);
/// Starts installing `package_file`, a package the player chose, as `update`.
std::shared_ptr<TitleUpdateJob> StartTitleUpdateInstall(const PPCTitleUpdate& update,
                                                        uint32_t title_id,
                                                        const std::filesystem::path& local_dir,
                                                        const std::filesystem::path& package_file);
/// The latest job for `version`, or null.
std::shared_ptr<TitleUpdateJob> FindTitleUpdateJob(uint32_t version);
/// Every job this run, oldest first.
std::vector<std::shared_ptr<TitleUpdateJob>> TitleUpdateJobs();

/// Which executable runs. A title update build is <original>_tu<version>.exe
/// beside the original; the player's choice (`wanted`, 0 for the original)
/// only takes effect when that update is installed and its executable exists,
/// and the original is always the fallback: an update is never required.
struct LaunchChoice {
  bool hand_off = false;  ///< start `executable` and exit
  std::filesystem::path executable;
  std::filesystem::path update;  ///< the package this one mounts at update:
  std::string note;              ///< why, for the log
  bool cannot_run = false;       ///< this update build has no update and no original
};
LaunchChoice ChooseLaunch(uint32_t built, uint32_t wanted, const std::filesystem::path& exe_path,
                          const std::filesystem::path& local_dir, bool handed_off);

/// The executable for `version` beside `exe_path` (built as `built`).
std::filesystem::path TitleUpdateExecutable(const std::filesystem::path& exe_path, uint32_t built,
                                            uint32_t version);

}  // namespace rex::ui::guide
