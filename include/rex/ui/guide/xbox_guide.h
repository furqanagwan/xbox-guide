/**
 * @file        rex/ui/guide/xbox_guide.h
 * @brief       The Xbox 360 guide, run from the console's own scenes (RG-GDK-041)
 *
 * @copyright   Copyright (c) 2026 Tom Clay
 * @license     BSD 3-Clause License
 */

#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <imgui.h>

#include <rex/cvar.h>
#include <rex/image_info.h>
#include <rex/ui/guide/code_patch_states.h>
#include <rex/ui/guide/active_downloads.h>
#include <rex/system/achievement_store.h>
#include <rex/ui/guide/guide_input.h>
#include <rex/ui/imgui_dialog.h>
#include <rex/ui/overlay/achievement_icon_cache.h>
#include <rex/ui/xui/document.h>
#include <rex/ui/xui/renderer.h>
#include <rex/ui/xui/runtime.h>
#include <rex/ui/xui/system_update.h>

namespace rex {
class Runtime;
namespace audio {
struct PcmSound;
class UiSoundPlayer;
}  // namespace audio
namespace input {
class InputSystem;
}  // namespace input
namespace system {
class AchievementManager;
class KernelState;
}  // namespace system
}  // namespace rex

namespace rex::ui {
class ImmediateDrawer;
class ImmediateTexture;
}  // namespace rex::ui

REXCVAR_DECLARE(bool, xbox_guide);
REXCVAR_DECLARE(std::string, xbox_guide_system_update);
REXCVAR_DECLARE(bool, notifications_show);
REXCVAR_DECLARE(bool, notifications_sound);
REXCVAR_DECLARE(std::string, code_patch_states);
REXCVAR_DECLARE(bool, resolution_match_display);

namespace rex::ui::guide {

/// Where the guide looks for the console's system update: the
/// xbox_guide_system_update cvar, then `$SystemUpdate` beside the executable,
/// then %LOCALAPPDATA%\ReXGlue\$SystemUpdate.
std::vector<std::filesystem::path> SystemUpdateLocations();

/// The guide bundle the title build embedded (rexglue_configure_target), so
/// players need nothing for the guide. Called by the generated registration
/// at static initialisation; empty when the title was built without one.
bool RegisterEmbeddedGuide(const uint8_t* data, size_t size);
std::span<const uint8_t> EmbeddedGuide();

/// The scenes and strings the guide uses, parsed once from the owner's system
/// update.
/// The title host chooses presentation; asset availability never chooses it.
/// OriginalXbox selects BC scenes, not an original Xbox execution backend.
enum class GuidePresentation { Xbox360, OriginalXbox };

struct GuideAssets {
  std::unique_ptr<xui::SystemUpdate> update;
  xui::Document skin;      // huduiskin: control visuals, message boxes
  xui::Document backdrop;  // xam hudbkgnd: the HUD frame the guide opens in
  xui::Document main;      // hud GuideMain: tabs and blades
  xui::Document home_tab, games_tab, settings_tab;
  // The guide Microsoft's backward compatibility shows (GuideMainEmulator
  // and the *TabEmulator* scenes, in the PC backward-compatibility HUD): three
  // tabs, Games (1), Home (2) and Settings (3). Otherwise the 2.0.17559 guide
  // without Media: tabs 1, 2 and 4 (UseThreeTabs).
  bool emulator_layout = false;
  xui::Document xbox_settings;  // hud XboxOneXSettings (emulator layout only)
  bool has_xbox_settings = false;
  xui::Document achievements, achievement_details;  // gamerprofile
  bool has_achievement_scenes = false;
  xui::Document notify;  // xam: the notification popup
  bool has_notify = false;
  // Preferences and the pages built on its scenes (hud).
  xui::Document options, options_vibration, options_notifications, options_voice;
  bool has_options = false;
  // The on-screen keyboard (vk: KeyboardMain hosts KeyboardBase; RG-GDK-059).
  xui::Document keyboard_main, keyboard_base;
  bool has_keyboard = false;
  std::vector<std::string> hud_strings, xam_strings, profile_strings;

  static std::unique_ptr<GuideAssets> Load(
      const std::filesystem::path& path, std::string* error,
      GuidePresentation presentation = GuidePresentation::Xbox360);
  /// From a guide bundle (see EmbeddedGuide).
  static std::unique_ptr<GuideAssets> LoadBundle(
      std::span<const uint8_t> bundle, std::string* error,
      GuidePresentation presentation = GuidePresentation::Xbox360);
  static std::unique_ptr<GuideAssets> FromUpdate(
      std::unique_ptr<xui::SystemUpdate> update, std::string* error,
      GuidePresentation presentation = GuidePresentation::Xbox360);
};

/// The first string in `strings` starting with `prefix` (tables are per
/// language; matching on English text keeps this independent of order).
std::string FindString(const std::vector<std::string>& strings, std::string_view prefix,
                       std::string_view fallback);

struct GuideFonts {
  ImFont* regular = nullptr;
  ImFont* bold = nullptr;
};

/// The guide's own vector drawings of the console's small images, for
/// xui::RenderResources::vector_image: the legend button glyphs and the
/// controller battery, sharp at any size.
bool DrawGuideVectorImage(ImDrawList& list, std::string_view path,
                          const std::function<ImVec2(ImVec2)>& to_screen, float opacity,
                          ImFont* bold);
/// Hides the letters scenes lay over the button glyphs, which
/// DrawGuideVectorImage draws itself.
void HideGuideButtonLetters(xui::Element* root);

/// Adds the console's system font from the guide built into the title
/// (RG-GDK-061), or Segoe UI, the host stand-in, without one. Call from the
/// ImGui drawer's font setup, before the atlas is built. The glyphs are baked
/// for `display_height` (the guide's text is sharp at 4K too).
GuideFonts AddGuideFonts(ImFontAtlas* atlas, int display_height = 0);

/// Textures and sounds from the system update, kept across openings.
class GuideMedia {
 public:
  GuideMedia(ImmediateDrawer* immediate_drawer, std::shared_ptr<const GuideAssets> assets);
  ~GuideMedia();

  ImTextureID Texture(std::string_view path, std::string_view package, int* width, int* height);
  void PlaySound(std::string_view file, std::string_view package);

 private:
  struct Image {
    std::unique_ptr<ImmediateTexture> texture;
    int width = 0, height = 0;
  };

  ImmediateDrawer* immediate_drawer_;
  std::shared_ptr<const GuideAssets> assets_;
  std::map<std::string, Image, std::less<>> images_;
  std::map<std::string, std::shared_ptr<const audio::PcmSound>, std::less<>> sounds_;
  std::unique_ptr<audio::UiSoundPlayer> player_;
};

struct GuideHost {
  input::InputSystem* input = nullptr;
  system::KernelState* kernel_state = nullptr;
  system::AchievementManager* achievements = nullptr;
  Runtime* runtime = nullptr;  // the title's XDBF achievement icons
  ImmediateDrawer* immediate_drawer = nullptr;
  std::string title_name;
  /// The title's switchable code patches (null name ends the list).
  const PPCSwitchablePatch* patches = nullptr;
  /// The title's own cheat codes (null name ends the list).
  const PPCTitleCheat* cheats = nullptr;
  /// The title's add-ons (null id ends the list), described by the built-in
  /// catalogue (EmbeddedDlcCatalog).
  const PPCTitleDlc* dlc = nullptr;
  /// The title update version the title was built with; 0 for none.
  uint32_t title_update = 0;
  /// The title's updates (zero version ends the list), for the Title Updates page, where
  /// the player can download one and turn it on: they are optional.
  const PPCTitleUpdate* title_updates = nullptr;
  /// The title's local data folder (%LOCALAPPDATA%\<name>): updates install
  /// under title_updates there.
  std::filesystem::path local_dir;
  /// Restarts the title once it has closed, so a title update choice takes
  /// effect; the guide then ends the title as Leave Game does.
  std::function<void()> restart_title;
  /// The draw resolution scale that matches the display (3 for 4K).
  int display_scale = 1;
  /// Writes changed settings to the title's config file.
  std::function<void()> save_settings;
  /// Host copy/install jobs, newest first; read only on the UI thread.
  std::function<std::vector<GuideActivity>()> activities;
  /// After the guide has closed; `exit_title` when the owner confirmed Xbox
  /// Home.
  std::function<void(bool exit_title)> on_closed;
};

/// The guide over a running title. It owns itself, as XAM dialogs do: it
/// deletes itself after its close animation and then calls on_closed.
class XboxGuide final : public ImGuiDialog {
 public:
  XboxGuide(ImGuiDrawer* drawer, std::shared_ptr<const GuideAssets> assets, GuideMedia* media,
            GuideFonts fonts, GuideHost host, uint16_t held_buttons);
  ~XboxGuide() override;

  /// Closes with the console's close animation (the chord pressed again).
  void Dismiss();

 protected:
  void OnDraw(ImGuiIO& io) override;
  void OnClose() override;

 private:
  enum class Screen { kMain, kAchievements, kAchievementDetail, kConfirm, kSettings };
  enum class Confirm { kXboxHome, kTurnOff, kTitleUpdate, kDeleteSave };

  void Handle(GuideAction action);
  void HandleMain(GuideAction action);
  void HandleAchievements(GuideAction action);
  void HandleConfirm(GuideAction action);
  void Activate(xui::Element* control);

  /// The tabs left to right: Games, Home and Settings (GuideAssets::emulator_layout).
  std::span<const int> Tabs() const;
  int SettingsTab() const { return Tabs().back(); }
  /// The next tab left (-1) or right (+1) of the current one.
  void SwitchTab(int direction);
  xui::Element* FirstFocusable(xui::Element* root);
  void SetFocus(xui::Element* control, bool initial = false);
  void SetLegends(std::string_view a, std::string_view b, std::string_view y,
                  std::string_view x = {});
  void ConfigureMain();

  void OpenAchievements();
  void ShowAchievement(size_t index);
  void ScrollAchievements();
  void OpenAchievementDetail();
  void CloseAchievements();

  void OpenConfirm(Confirm confirm);
  void CloseConfirm();

  // Settings pages (guide_settings.cpp): Preferences and what it opens,
  // Patches and Cheats, each one of the console's own Options scenes.
  struct SettingsPage {
    xui::Element* scene = nullptr;
    xui::Element* return_focus = nullptr;  // focus on the page below
    std::function<void(xui::Element*)> on_select;
    std::function<void()> on_focus;  // after focus moves on the page
    std::function<void(xui::Element*, int)> on_adjust;
    std::function<void(xui::Element*)> on_x;  // X on the focused control
  };
  SettingsPage& PushPage(const xui::Document& scene, std::string heading);
  void PopPage();
  void HandleSettings(GuideAction action);
  void OpenPreferences();
  void OpenVibration();
  void OpenVolume();
  void OpenNotifications();
  void OpenResolution();
  /// Settings > Xbox Settings, emulator layout: the render resolution on the
  /// XboxOneXSettings scene's Graphics and Performance choice.
  void OpenXboxSettings();
  void OpenPatches(std::string_view category);
  void OpenCheats();
  void SetSlider(xui::Element* slider, int value);

  // Games & Apps > Manage Game (guide_dlc.cpp): the title's add-ons from its
  // catalogue, installed from packages on this PC.
  struct DlcEntry {
    std::string id;                 // catalogue media ID; empty for content it lacks
    std::filesystem::path package;  // a package on this PC for it, if found
    std::string file_name;          // installed content's file name
    std::string name;
    std::string package_name;  // the display name its package carries
    std::string publisher;
    std::string description;
    uint32_t requires_title_update = 0;
    bool installed = false;
  };
  struct DlcJob;  // an install or file pick running off the UI thread
  void OpenManageGame();
  void FillManageGame();
  void StartDlcInstall(const DlcEntry& entry);
  void ShowDlc(xui::Element* row);
  void PollManageGame();
  std::vector<DlcEntry> FindDlc() const;
  // Home > Manage Storage (guide_storage.cpp): the title's saved games on
  // this PC, which the player can delete.
  struct SaveEntry {
    std::string name;       // display name
    std::string file_name;  // content file name
    uint64_t xuid = 0;      // 0 for saves common to every profile
    uint64_t size = 0;      // bytes
    std::string modified;   // last write, local time
  };
  void OpenManageStorage();
  void FillManageStorage();
  void ShowSave(xui::Element* row);
  void DeleteChosenSave();
  std::vector<SaveEntry> FindSaves() const;
  // Games & Apps > Title Updates (guide_title_update.cpp): the title's
  // updates, optional, downloaded and turned on or off. Not add-ons, so not
  // in Manage Game.
  void OpenTitleUpdates();
  void FillTitleUpdates();
  void PollTitleUpdates();
  /// The title update `row` is on the Title Updates page, or null.
  const PPCTitleUpdate* TitleUpdateAt(xui::Element* row) const;
  std::string TitleUpdateAction(const PPCTitleUpdate& update) const;
  void ShowTitleUpdate(const PPCTitleUpdate& update);
  void SelectTitleUpdate(const PPCTitleUpdate& update);
  void ChooseTitleUpdateFile(const PPCTitleUpdate& update);
  void ApplyTitleUpdateChoice();
  // Games & Apps > Active Downloads: title update downloads and installs.
  void OpenActiveDownloads();
  void FillActiveDownloads();
  void PollActiveDownloads();

  void BeginClose(bool exit_title);
  void UpdateClock();
  /// The Achievements row's gamerscore glyph in its label's colour: the skin
  /// draws it near white, unseen on an unfocused row.
  void ColourGamerscoreGlyph();
  /// Shows player 1's battery, at most once a second.
  void UpdateControllerBattery();
  ImTextureID Texture(std::string_view path, std::string_view package, int* width, int* height);

  std::shared_ptr<const GuideAssets> assets_;
  GuideMedia* media_;
  GuideFonts fonts_;
  GuideHost host_;
  GuidePad pad_;
  AchievementIconCache icons_;
  xui::RenderResources render_;

  xui::SceneContext backdrop_context_, hud_context_, skin_context_, profile_context_;
  std::unique_ptr<xui::Element> backdrop_;
  xui::Element* hud_root_ = nullptr;  // HUDRootScene
  xui::Element* app_host_ = nullptr;
  xui::Element* error_host_ = nullptr;
  xui::Element* main_ = nullptr;
  xui::Element* tabs_ = nullptr;
  xui::Element* tab_scenes_[5] = {};
  xui::Element* tab_focus_[5] = {};
  int tab_ = 2;  // Home
  xui::Element* focus_ = nullptr;

  Screen screen_ = Screen::kMain;
  std::vector<system::AchievementInfo> achievement_list_;
  xui::Element* achievements_ = nullptr;
  xui::Element* details_ = nullptr;
  size_t selected_ = 0;
  size_t first_row_ = 0;
  int columns_ = 1;
  int visible_rows_ = 1;

  Confirm confirm_ = Confirm::kXboxHome;
  uint32_t confirm_title_update_ = 0;  // kTitleUpdate: the version to run (0: the original)
  xui::Element* message_ = nullptr;
  xui::Element* return_focus_ = nullptr;
  std::vector<xui::Element*> pending_removal_;  // detached once the backdrop stops
  std::vector<SettingsPage> pages_;
  xui::Element* manage_scene_ = nullptr;
  std::vector<xui::Element*> manage_rows_;
  std::vector<DlcEntry> dlc_;
  std::vector<std::filesystem::path> picked_packages_;
  std::shared_ptr<DlcJob> dlc_job_;
  std::string dlc_status_;
  xui::Element* storage_scene_ = nullptr;
  std::vector<xui::Element*> storage_rows_;
  std::vector<SaveEntry> saves_;
  size_t confirm_save_ = 0;  // kDeleteSave: the index in saves_
  std::string storage_status_;
  std::shared_ptr<std::filesystem::path> picked_title_update_;  // a file pick's result
  std::shared_ptr<std::atomic<bool>> title_update_pick_;        // set when the pick is done
  uint32_t title_update_pick_version_ = 0;
  xui::Element* updates_scene_ = nullptr;
  std::vector<xui::Element*> update_rows_;
  xui::Element* downloads_scene_ = nullptr;
  std::vector<xui::Element*> download_rows_;
  std::vector<GuideActivity> download_items_;
  // The right pane's banner: an XuiImage the Options scene does not have.
  std::unique_ptr<xui::Node> dlc_banner_node_;
  xui::Element* dlc_banner_ = nullptr;

  bool closing_ = false;
  bool exit_title_ = false;
  float dim_ = 0.0f;  // how far the title behind the guide is dimmed, 0 to 1
  std::chrono::steady_clock::time_point last_tick_;
  std::chrono::steady_clock::time_point opened_;
  int64_t clock_minute_ = -1;
  int64_t battery_second_ = -1;
};

}  // namespace rex::ui::guide
