/**
 * @file        rex/ui/guide/dlc_catalog.h
 * @brief       A title's downloadable content catalogue, built into it (RG-GDK-050)
 *
 * `rexglue dlc-catalog` fetches the entries a title's config lists from the
 * Xbox 360 marketplace catalogue at build time, in the builder's language,
 * with their banner and tile art. rexglue_configure_target embeds the result,
 * so Games & Apps > Manage Game lists the title's add-ons offline.
 *
 * @copyright   Copyright (c) 2026 Tom Clay
 * @license     BSD 3-Clause License
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace rex::ui::guide {

struct DlcCatalogEntry {
  std::string id;  // marketplace media ID, a GUID ("D4C83E1F-243B-...")
  std::string title;
  std::string publisher;
  std::string developer;
  std::string description;
  std::string release_date;     // yyyy-mm-dd, or empty
  std::vector<uint8_t> banner;  // 420 x 95 PNG, or empty
  std::vector<uint8_t> tile;    // 64 x 64 PNG, or empty
};

/// "RXDLC001", a count, then each entry's strings and images, every field a
/// little-endian u32 length and its bytes.
std::vector<uint8_t> WriteDlcCatalog(const std::vector<DlcCatalogEntry>& entries);
std::optional<std::vector<DlcCatalogEntry>> ReadDlcCatalog(std::span<const uint8_t> data);

/// Called by the catalogue rexglue_configure_target builds into the title.
bool RegisterEmbeddedDlcCatalog(const uint8_t* data, size_t size);
/// The built-in catalogue, empty when the title has none.
std::span<const uint8_t> EmbeddedDlcCatalog();
/// The built-in catalogue read once; empty when there is none or it is bad.
const std::vector<DlcCatalogEntry>& EmbeddedDlcEntries();
/// The built-in entry with this media ID, or null.
const DlcCatalogEntry* FindEmbeddedDlc(std::string_view id);

}  // namespace rex::ui::guide
