/**
 * @file        ui/guide/include/rex/ui/guide/device_details.h
 * @brief       What an audio output or display supports, as the guide's pages describe it
 *
 * @copyright   Copyright (c) 2026 Tom Clay
 * @license     BSD 3-Clause License
 */

#pragma once

#include <string>

#include <rex/audio/audio_outputs.h>
#include <rex/ui/display_info.h>

namespace rex::ui::guide {

/// The output's speakers, how the game's 5.1 plays on them, spatial sound and
/// the surround formats it decodes, as lines for the guide's details pane.
std::string DescribeAudioOutput(const audio::AudioOutput& output);

/// The display's current mode, the fastest refresh of its main resolutions,
/// HDR and colour, as lines for the guide's details pane.
std::string DescribeDisplay(const DisplayInfo& display);

}  // namespace rex::ui::guide
