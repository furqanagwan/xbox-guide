/**
 * @file        ui/xui/include/rex/ui/xui/system_update.h
 * @brief       The dashboard XUI packages, read from the user's own system update (RG-GDK-041)
 *
 * @copyright   Copyright (c) 2026 Tom Clay
 * @license     BSD 3-Clause License
 */

#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <rex/ui/xui/package.h>

namespace rex::ui::xui {

struct XexResource {
  std::string name;
  std::vector<uint8_t> bytes;
};

/// The named resources of an unencrypted XEX2 image (uncompressed, basic or
/// LZX-compressed), as the console's system XEXs are.
std::optional<std::vector<XexResource>> ReadXexResources(std::span<const uint8_t> xex,
                                                         std::string* error);

/// XUIZ packages from the console's system XEXs, keyed "module/resource":
/// "hud/hud", "huduiskin/skin", "xam/shrdres", "gamerprofile/gp"...
class SystemUpdate {
 public:
  /// The system XEXs the guide reads, as they are named in the update package;
  /// vk is the on-screen keyboard (RG-GDK-059).
  static constexpr std::string_view kModules[] = {"hud", "huduiskin", "xam", "gamerprofile", "vk"};
  /// The console's system fonts (.xtt) the guide's text uses, when present:
  /// the Latin and Japanese/Korean font of the PC backward-compatibility
  /// files, the update's light Segoe and the Chinese ones.
  static constexpr std::string_view kFonts[] = {"xenonjklatin", "xenonclatin", "xenonsclatin",
                                                "SegoeXbox-Light"};

  /// The system XEXs by module name ("hud" -> `$flash_hud.xex` bytes), and
  /// the fonts as "font/<name>" -> .xtt bytes.
  using Modules = std::map<std::string, std::vector<uint8_t>>;

  /// `path` is a `$SystemUpdate` folder, the `su20076000_00000000` package in
  /// it, or a folder holding the `$flash_<module>.xex` (or `<module>.xex`)
  /// files, such as the Flash folder of an Xbox PC backward-compatibility game.
  static std::unique_ptr<SystemUpdate> Load(const std::filesystem::path& path, std::string* error);

  /// Reads the system XEXs the guide needs from `path` (as for Load).
  static std::optional<Modules> ReadModules(const std::filesystem::path& path, std::string* error);
  /// Reads several sources; a module or font comes from the first that has it.
  static std::optional<Modules> ReadModules(std::span<const std::filesystem::path> paths,
                                            std::string* error);
  /// Builds the update from system XEXs; fails when a package the guide needs
  /// is missing.
  static std::unique_ptr<SystemUpdate> FromModules(const Modules& modules, std::string* error);

  /// The guide bundle: the system XEXs in one blob, which the title build
  /// embeds into the executable (rexglue guide-bundle, rexglue_configure_target)
  /// so the guide needs nothing at run time.
  static std::vector<uint8_t> WriteBundle(const Modules& modules);
  static std::optional<Modules> ReadBundle(std::span<const uint8_t> bundle, std::string* error);

  /// Adds the XUIZ resources of one system XEX. Used by Load and by tests.
  bool AddModule(std::string_view module, std::span<const uint8_t> xex, std::string* error);

  /// Converts a "font/<name>" module to TrueType. Used by FromModules and tests.
  bool AddFont(std::string_view name, std::span<const uint8_t> xtt, std::string* error);

  const Package* Find(std::string_view module_resource) const;
  const std::map<std::string, Package>& packages() const { return packages_; }
  /// A system font as TrueType ("xenonjklatin"), or empty.
  std::span<const uint8_t> Font(std::string_view name) const;

 private:
  std::map<std::string, Package> packages_;
  std::map<std::string, std::vector<uint8_t>> fonts_;
};

}  // namespace rex::ui::xui
