/**
 * @file        rex/ui/xui/document.h
 * @brief       XUR v8 scene documents: element tree, timelines, named frames (RG-GDK-041)
 *
 * @copyright   Copyright (c) 2026 Tom Clay
 * @license     BSD 3-Clause License
 */

#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <rex/ui/xui/schema.h>

namespace rex::ui::xui {

struct Vec3 {
  float x = 0.0f, y = 0.0f, z = 0.0f;
  bool operator==(const Vec3&) const = default;
};

struct Quat {
  float x = 0.0f, y = 0.0f, z = 0.0f, w = 1.0f;
  bool operator==(const Quat&) const = default;
};

/// 0xAARRGGBB.
struct Color {
  uint32_t argb = 0;
  bool operator==(const Color&) const = default;
};

/// A figure outline: cubic Bezier segments in the figure's own box.
struct Path {
  struct Point {
    float x, y;      // anchor
    float c1x, c1y;  // control point before the anchor
    float c2x, c2y;  // control point after the anchor
  };
  float width = 0.0f, height = 0.0f;
  std::vector<Point> points;
};

struct PropertyBag;
struct Value;

using ValueData =
    std::variant<std::monostate, bool, int32_t, uint32_t, float, std::string, Vec3, Quat, Color,
                 std::shared_ptr<const PropertyBag>, std::shared_ptr<const Path>,
                 std::shared_ptr<const std::vector<Value>>>;

struct Value {
  ValueData data;

  template <typename T>
  const T* get() const {
    return std::get_if<T>(&data);
  }
};

/// An element's (or a compound property's) set properties.
struct PropertyBag {
  struct Entry {
    const PropDef* def = nullptr;  // nullptr: a property the schema does not know
    Value value;
  };
  std::vector<Entry> entries;

  const Value* Find(std::string_view name) const;
};

enum class Interpolation : uint8_t { kLinear, kNone, kEase };

struct Keyframe {
  int32_t frame = 0;
  Interpolation interpolation = Interpolation::kLinear;
  int8_t ease_in = 0;  // -100..100
  int8_t ease_out = 0;
  uint8_t ease_scale = 0;
  std::vector<Value> values;  // one per Timeline::props entry
};

/// One animated property: a path through compound properties (Fill, then
/// FillColor) ending at the animated one, and the array element for
/// indexed properties.
struct AnimatedProperty {
  std::vector<const PropDef*> path;  // empty: not in the schema, ignored
  int32_t index = -1;
};

struct Timeline {
  std::string target_id;
  std::vector<AnimatedProperty> props;
  std::vector<Keyframe> keyframes;
};

enum class FrameCommand : uint8_t { kPlay, kStop, kGoTo, kGoToAndPlay, kGoToAndStop };

struct NamedFrame {
  std::string name;
  int32_t frame = 0;
  FrameCommand command = FrameCommand::kPlay;
  std::string target;  // the named frame the GoTo commands jump to
};

struct Node {
  std::string class_name;
  const ClassDef* cls = nullptr;
  std::shared_ptr<const PropertyBag> props;
  std::vector<Node> children;
  std::vector<Timeline> timelines;  // animate this node's descendants
  std::vector<NamedFrame> named_frames;

  const Value* Find(std::string_view name) const { return props ? props->Find(name) : nullptr; }
  std::string_view id() const;
  /// Depth-first search of the descendants (not this node) by Id.
  const Node* FindById(std::string_view id) const;
};

struct Document {
  Node root;  // an XuiCanvas
};

std::optional<Document> ParseXur(std::span<const uint8_t> bytes, std::string* error);

}  // namespace rex::ui::xui
