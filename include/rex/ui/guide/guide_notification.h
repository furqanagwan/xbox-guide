/**
 * @file        rex/ui/guide/guide_notification.h
 * @brief       Achievement unlocks as the console shows them: XAM's notify scene (RG-GDK-041)
 *
 * @copyright   Copyright (c) 2026 Tom Clay
 * @license     BSD 3-Clause License
 */

#pragma once

#include <deque>
#include <functional>
#include <memory>
#include <mutex>

#include <rex/ui/guide/xbox_guide.h>
#include <rex/ui/overlay/achievement_notification.h>

namespace rex::ui::guide {

/// Plays xam's notify.xur (the scr_Notification visual: the Xbox logo burst,
/// the bar sliding out, NotifyPopup.xma) with XAM's "Achievement unlocked"
/// text, one unlock at a time at the bottom of the screen. Without a system
/// update it hands unlocks to `fallback`.
class GuideNotificationDialog final : public AchievementNotificationDialog {
 public:
  struct Media {
    std::shared_ptr<const GuideAssets> assets;
    GuideMedia* media = nullptr;
    bool unavailable = false;  // no system update: use the fallback
  };
  /// Called on the UI thread each frame.
  using MediaSource = std::function<Media()>;

  GuideNotificationDialog(ImGuiDrawer* drawer, MediaSource source, GuideFonts fonts,
                          std::unique_ptr<AchievementNotificationDialog> fallback);
  ~GuideNotificationDialog() override;

  void Push(const system::AchievementEvent& event) override;

 protected:
  void OnDraw(ImGuiIO& io) override;

 private:
  void Start(const system::AchievementEvent& event, const Media& media);

  MediaSource source_;
  GuideFonts fonts_;
  std::unique_ptr<AchievementNotificationDialog> fallback_;
  std::mutex mutex_;
  std::deque<system::AchievementEvent> queue_;  // guarded by mutex_

  std::shared_ptr<const GuideAssets> assets_;
  GuideMedia* media_ = nullptr;
  xui::SceneContext context_;
  xui::RenderResources render_;
  std::unique_ptr<xui::Element> scene_;
  xui::Element* popup_ = nullptr;
  std::chrono::steady_clock::time_point last_tick_;
  std::chrono::steady_clock::time_point started_;
};

}  // namespace rex::ui::guide
