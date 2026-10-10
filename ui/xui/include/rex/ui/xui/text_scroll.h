/**
 * @file        ui/xui/include/rex/ui/xui/text_scroll.h
 * @brief       Scrolling for text taller than its box, as the console's long descriptions
 *
 * @copyright   Copyright (c) 2026 Tom Clay
 * @license     BSD 3-Clause License
 */

#pragma once

#include <string>

namespace rex::ui::xui {

/// A wrapped text taller than its box scrolls as the console's long
/// descriptions did: it holds at the top, scrolls slowly to the end, holds,
/// fades out, and fades back in at the top.
struct TextScroll {
  std::string text;  ///< what it showed when `start` was taken
  double start = 0;  ///< seconds (ImGui::GetTime)
};
struct TextScrollFrame {
  float offset = 0;  ///< how far the text has moved up, in its own units
  float alpha = 1;   ///< its opacity in the fade
};
/// Where a text `overflow` taller than its box is `elapsed` seconds into the
/// cycle, scrolling `speed` units a second.
TextScrollFrame TextScrollAt(double elapsed, float overflow, float speed);

}  // namespace rex::ui::xui
