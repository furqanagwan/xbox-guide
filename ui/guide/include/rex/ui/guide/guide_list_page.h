// Copyright (c) 2026 Xbox Guide contributors. BSD 3-Clause License; see LICENSE.
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <rex/ui/xui/runtime.h>

namespace rex::ui::guide {
struct GuideListRow {
  std::string text;
  std::string secondary;  // right-aligned, as Manage Storage shows a save's size
  std::string details;    // the right pane while this row has focus
};

// A Guide page outside the running guide: the HUD frame at full height
// hosting the Options scene as a list, as Manage Storage and Active Downloads
// look in the in-game Guide. No runtime or guest dependencies; the caller
// supplies input, rendering resources and actions. The assets and the
// contexts' skin must outlive this object.
class GuideListPage {
 public:
  static constexpr float kSceneWidth = 852.0f;
  static constexpr float kSceneHeight = 480.0f;
  static constexpr size_t kVisibleRows = 11;  // the list area of the Options scene

  GuideListPage(const GuideListPage&) = delete;
  GuideListPage& operator=(const GuideListPage&) = delete;

  // `frame` is GuideAssets::backdrop (context package xam/xam) and `options`
  // GuideAssets::options_notifications (hud/hud). Null when either lacks the
  // controls this uses. The caller hides the legend glyphs' letters
  // (HideGuideButtonLetters) when it draws them itself.
  static std::unique_ptr<GuideListPage> Create(const xui::Document& frame,
                                               const xui::SceneContext& frame_context,
                                               const xui::Document& options,
                                               const xui::SceneContext& options_context);

  // Replaces the page's contents; `empty_details` fills the right pane when
  // there are no rows. The frame's open animation is not replayed.
  void Show(std::string heading, std::vector<GuideListRow> rows, size_t focus = 0,
            std::string legend_a = "Select", std::string legend_b = "Back",
            std::string empty_details = {});
  // Up (-1) or down (+1), scrolling the visible window; stops at the ends.
  void Move(int direction);
  void Focus(size_t index);
  // The focused row, pressed, unless there are none.
  std::optional<size_t> Activate();
  size_t focused() const { return focused_; }
  size_t size() const { return rows_.size(); }
  size_t first_visible() const { return first_; }
  // The visible row control at `slot` (0 to kVisibleRows - 1), or null.
  xui::Element* slot(size_t index) const {
    return index < slots_.size() && slots_[index]->visible() ? slots_[index] : nullptr;
  }
  xui::Element& root() { return *backdrop_; }
  xui::Element& scene() { return *scene_; }

  // Plays the frame's open animation (on creation) or close animation.
  void PlayClose();
  bool animating() const { return hud_root_->playing(); }

 private:
  GuideListPage() = default;
  void Refresh();
  void SetLegend(const char* button, const char* label, const std::string& text);

  xui::SceneContext backdrop_context_, hud_context_;
  std::unique_ptr<xui::Element> backdrop_;
  xui::Element* hud_root_ = nullptr;
  xui::Element* scene_ = nullptr;
  xui::Element* details_ = nullptr;
  std::vector<xui::Element*> slots_;
  std::vector<GuideListRow> rows_;
  std::string empty_details_;
  size_t first_ = 0;
  size_t focused_ = 0;
  xui::Element* focus_ = nullptr;
};
}  // namespace rex::ui::guide
