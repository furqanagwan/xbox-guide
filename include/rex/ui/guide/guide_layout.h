/**
 * @file        rex/ui/guide/guide_layout.h
 * @brief       Removing and adding menu entries in the console's scenes (RG-GDK-041)
 *
 * @copyright   Copyright (c) 2026 Tom Clay
 * @license     BSD 3-Clause License
 */

#pragma once

#include <cmath>
#include <string>
#include <string_view>

#include <rex/ui/xui/runtime.h>

namespace rex::ui::guide {

/// The first child of a scene file's canvas: the scene itself.
inline const xui::Node& SceneNode(const xui::Document& document) {
  return document.root.children.empty() ? document.root : document.root.children.front();
}

/// Rewrites GuideMain (its Tabscene's timelines) for three tabs, Games & Apps
/// (1), Home (2) and Settings (4), without Media (3). Every switch moves each
/// blade one slot, so a side with one tab fewer is the same motion with its
/// outermost blade hidden and each label on its inner neighbour's track:
/// - tabs 1 and 2 (1To2, 2To1, 1Open to 2Close): Blade5, the third blade on
///   the right, is hidden and txt_Settings follows txt_Media;
/// - tab 4: Home to Settings and back play 3To4 and 4To3, renamed 2To4 and
///   4To2. Blade6, the third on the left, is hidden; txt_Games follows
///   txt_home and txt_home txt_Media; Home's content (Tab2) and selected label
///   (txt_homeSel) fade as Media's did. 4Open and 4Close keep Blade6 hidden
///   and the labels moved.
/// 2To3 and 3To2 are dropped. Returns false (scene untouched) when GuideMain
/// is not the 17559 layout this expects.
bool UseThreeTabs(xui::Node& guide_main);

/// Hides `id` in `scene` for good and closes the gap: entries below it move
/// up by its height, and the neighbours' NavUp/NavDown skip it.
inline void RemoveEntry(xui::Element* scene, std::string_view id) {
  xui::Element* entry = scene->FindById(id);
  if (!entry || entry->suppressed()) {
    return;
  }
  const std::string up(entry->GetString("NavUp"));
  const std::string down(entry->GetString("NavDown"));
  const float y = entry->GetVector("Position").y;
  const float height = entry->height();
  entry->Suppress();
  xui::Element* parent = entry->parent();
  for (const auto& sibling : parent->children()) {
    xui::Element* s = sibling.get();
    if (s == entry) {
      continue;
    }
    if (s->GetString("NavDown") == id) {
      s->Set("NavDown", xui::Value{down});
    }
    if (s->GetString("NavUp") == id) {
      s->Set("NavUp", xui::Value{up});
    }
    xui::Vec3 p = s->GetVector("Position");
    if (s->IsA("XuiControl") && !s->suppressed() && p.y > y + 0.5f) {
      p.y -= height;
      s->Set("Position", xui::Value{p});
    }
  }
}

/// Adds an entry below `after` that is a copy of `model` (same class and
/// visual, so it looks like its neighbours, or `visual` when given), with
/// `text`. Entries below `after` move down to make room.
inline xui::Element* AddEntry(xui::Element* scene, std::string_view model, std::string_view after,
                              std::string id, std::string text, std::string_view visual = {}) {
  xui::Element* model_entry = scene->FindById(model);
  xui::Element* after_entry = scene->FindById(after);
  if (!model_entry || !after_entry || model_entry->parent() != after_entry->parent()) {
    return nullptr;
  }
  xui::Element* parent = after_entry->parent();
  const float y = after_entry->GetVector("Position").y;
  const float height = after_entry->height();
  for (const auto& sibling : parent->children()) {
    xui::Vec3 p = sibling->GetVector("Position");
    if (sibling->IsA("XuiControl") && !sibling->suppressed() && p.y > y + 0.5f) {
      p.y += height;
      sibling->Set("Position", xui::Value{p});
    }
  }
  xui::Element* entry = parent->CloneChild(*model_entry, id, visual);
  xui::Vec3 p = model_entry->GetVector("Position");
  p.y = y + height;
  entry->Set("Position", xui::Value{p});
  entry->SetText(std::move(text));
  entry->SetVisible(true);
  const std::string below(after_entry->GetString("NavDown"));
  entry->Set("NavUp", xui::Value{std::string(after)});
  entry->Set("NavDown", xui::Value{below});
  after_entry->Set("NavDown", xui::Value{id});
  if (xui::Element* next = below.empty() ? nullptr : scene->FindById(below)) {
    next->Set("NavUp", xui::Value{id});
  }
  return entry;
}

/// Scrolls the menu holding `entry` by whole entries until `entry` lies inside
/// the menu scene's height. A menu with entries past that height is clipped to
/// it, so the entries scrolled out of view are not drawn over the blade.
inline void ScrollMenuTo(xui::Element* entry) {
  xui::Element* menu = entry ? entry->parent() : nullptr;
  if (!menu || !menu->IsA("XuiScene")) {
    return;
  }
  const float height = menu->height();
  bool overflows = false;
  for (const auto& child : menu->children()) {
    if (child->IsA("XuiControl") && child->visible() &&
        child->GetVector("Position").y + child->height() > height + 0.5f) {
      overflows = true;
    }
  }
  if (overflows) {
    menu->Set("ClipChildren", xui::Value{true});
  }
  const float row = entry->height();
  const float top = entry->GetVector("Position").y;
  float shift = 0.0f;
  if (top < -0.5f) {
    shift = -top;
  } else if (row > 0.0f && top + row > height + 0.5f) {
    shift = -std::ceil((top + row - height) / row) * row;
  }
  if (shift == 0.0f) {
    return;
  }
  for (const auto& child : menu->children()) {
    if (child->IsA("XuiControl") && !child->suppressed()) {
      xui::Vec3 p = child->GetVector("Position");
      p.y += shift;
      child->Set("Position", xui::Value{p});
    }
  }
}

}  // namespace rex::ui::guide
