/**
 * @file        ui/xui/include/rex/ui/xui/xtt_font.h
 * @brief       The console's XTT system fonts as standard TrueType (RG-GDK-061)
 *
 * @copyright   Copyright (c) 2026 Tom Clay
 * @license     BSD 3-Clause License
 */

#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace rex::ui::xui {

/// Converts one of the console's .xtt fonts (xenonjklatin.xtt,
/// SegoeXbox-Light.xtt...) to a TrueType font that ImGui and FreeType load.
///
/// An XTT file is a signed header and a zlib-compressed sfnt directory whose
/// outlines live in Xbox tables: `xglf` holds the glyphs in separately
/// compressed 4 KiB blocks, `xloc` locates each glyph as (block << 16) |
/// offset, and `xchk` holds a SHA-1 per block. The result keeps the font's
/// own standard tables (cmap, head, hhea, hmtx, name...), rebuilds glyf and
/// loca from the blocks, and derives maxp, OS/2 and post when the font has
/// none.
std::optional<std::vector<uint8_t>> XttToTrueType(std::span<const uint8_t> xtt, std::string* error);

}  // namespace rex::ui::xui
