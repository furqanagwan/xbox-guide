/**
 * @file        ui/xui/include/rex/ui/xui/schema.h
 * @brief       XUI class and property order used to decode XUR v8 (RG-GDK-041)
 *
 * @copyright   Copyright (c) 2026 Tom Clay
 * @license     BSD 3-Clause License
 */

#pragma once

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace rex::ui::xui {

enum class PropType : uint8_t {
  kBool,
  kInteger,
  kUnsigned,
  kString,
  kFloat,
  kVector,
  kQuaternion,
  kColor,
  kObject,  // compound: Fill, Gradient, Stroke
  kCustom,  // figure path
};

struct ClassDef;

struct PropDef {
  std::string_view name;
  PropType type;
  bool indexed = false;
};

/// A class's own properties, in the order XUR property masks number them.
struct ClassDef {
  std::string_view name;
  std::string_view base;  // empty for a root class
  std::span<const PropDef> props;
};

/// Classes the console's guide, skin and achievement scenes use.
const ClassDef* FindClass(std::string_view name);

/// Base class first, as XUR property masks are written.
std::vector<const ClassDef*> ClassChain(const ClassDef* derived);

/// The class a compound (kObject) property's value is made of, by the
/// property's name: Fill, Gradient or Stroke.
const ClassDef* CompoundClass(std::string_view property_name);

}  // namespace rex::ui::xui
