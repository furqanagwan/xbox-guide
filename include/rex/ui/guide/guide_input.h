/**
 * @file        rex/ui/guide/guide_input.h
 * @brief       Opening the Xbox guide and driving it from a pad (RG-GDK-041)
 *
 * @copyright   Copyright (c) 2026 Tom Clay
 * @license     BSD 3-Clause License
 */

#pragma once

#include <cstdint>
#include <vector>

namespace rex::ui::guide {

/// The guide chord: View (Back) and Menu (Start) held together, as Xbox
/// backward compatibility opens the Xbox 360 guide. The Xbox button is left
/// to Windows, which opens Game Bar with it. Fires once per press; both must
/// be let go before it fires again.
class GuideChord {
 public:
  bool Update(uint16_t buttons);

 private:
  bool latched_ = true;  // a chord held when polling starts does not fire
};

enum class GuideAction : uint8_t {
  kUp,
  kDown,
  kLeft,
  kRight,
  kA,
  kB,
  kX,
  kY,
  kPreviousTab,  // left bumper
  kNextTab,      // right bumper
  kStart,
  kLeftThumb,    // the left stick pressed in
  kLeftTrigger,  // pulled past half way
  kRightTrigger,
};

/// Pad buttons and the left stick as guide actions. Directions repeat while
/// held (after kRepeatDelayMs, every kRepeatIntervalMs); buttons fire once
/// per press. Buttons already held when the guide opens are ignored until
/// released.
class GuidePad {
 public:
  static constexpr uint64_t kRepeatDelayMs = 400;
  static constexpr uint64_t kRepeatIntervalMs = 110;
  static constexpr int16_t kStickThreshold = 16384;
  static constexpr uint8_t kTriggerThreshold = 128;

  /// `buttons` held at open; they will not produce actions.
  explicit GuidePad(uint16_t held_buttons = 0xFFFF);

  std::vector<GuideAction> Update(uint16_t buttons, int16_t thumb_lx, int16_t thumb_ly,
                                  uint64_t now_ms, uint8_t left_trigger = 0,
                                  uint8_t right_trigger = 0);

 private:
  uint16_t previous_ = 0;
  uint16_t ignored_ = 0;
  int direction_ = -1;  // GuideAction for the held direction, -1 when none
  uint64_t next_repeat_ms_ = 0;
  // The triggers as two more buttons: pulled past kTriggerThreshold.
  bool left_trigger_ = true, right_trigger_ = true;
};

}  // namespace rex::ui::guide
