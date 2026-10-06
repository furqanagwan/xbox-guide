/**
 * @file        rex/ui/guide/code_patch_states.h
 * @brief       Saving the player's switchable code patch choices (RG-GDK-041)
 *
 * @copyright   Copyright (c) 2026 Tom Clay
 * @license     BSD 3-Clause License
 */

#pragma once

#include <string>

#include <rex/image_info.h>

namespace rex::ui::guide {

/// Sets each switchable patch's flag from code_patch_states ("Name=1;...");
/// patches it does not name keep their compiled-in default.
void ApplySavedCodePatches(const PPCSwitchablePatch* patches);
/// code_patch_states for the patches' current flags.
std::string SaveCodePatchStates(const PPCSwitchablePatch* patches);

}  // namespace rex::ui::guide
