/**
 * @file        ui/xui/include/rex/ui/xui/runtime.h
 * @brief       Live XUI elements: skin visuals, anchoring, named-frame timelines, focus
 * (RG-GDK-041)
 *
 * @copyright   Copyright (c) 2026 Tom Clay
 * @license     BSD 3-Clause License
 */

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <rex/ui/xui/document.h>

namespace rex::ui::xui {

class SystemUpdate;

/// XUI timelines count frames at this rate (inferred; see docs/xbox-guide.md).
constexpr double kFramesPerSecond = 60.0;

enum AnchorFlags : uint32_t {
  kAnchorLeft = 0x01,
  kAnchorTop = 0x02,
  kAnchorRight = 0x04,
  kAnchorBottom = 0x08,
  kAnchorXCenter = 0x10,
  kAnchorYCenter = 0x20,
  kAnchorXScale = 0x40,
  kAnchorYScale = 0x80,
};

enum class NavDirection { kUp, kDown, kLeft, kRight };

/// Resolves a scene path ("sharedres://A-Button.png", "xam://x.png" or a
/// name relative to `base_package`) to a file in the system update. Relative
/// names not in the base package are looked up in the skin and shared
/// packages, where skin visuals keep their images. Empty when missing.
std::span<const uint8_t> ResolveFile(const SystemUpdate& update, std::string_view path,
                                     std::string_view base_package);

/// The cubic ease XUI keyframes use, for signed -100..100 in/out values.
float EaseProgress(float t, int8_t ease_in, int8_t ease_out);

/// Where a scene's relative paths resolve and the skin it takes visuals from.
struct SceneContext {
  const Document* skin = nullptr;  // skin.xur; visuals are its root's XuiVisual children
  std::string package;             // "hud/hud": base for relative paths
  /// Called for each sound cue a playing timeline crosses.
  std::function<void(std::string_view file, std::string_view package)> play_sound;
};

class Element {
 public:
  /// Builds `node` and its subtree, applying skin visuals to controls, and
  /// shows every timeline's frame 0.
  static std::unique_ptr<Element> Create(const Node& node, const SceneContext& context);

  ~Element();
  Element(const Element&) = delete;
  Element& operator=(const Element&) = delete;

  std::string_view id() const;
  const std::string& class_name() const { return class_name_; }
  /// True when the element's class is `name` or derives from it.
  bool IsA(std::string_view name) const;
  Element* parent() const { return parent_; }
  const std::vector<std::unique_ptr<Element>>& children() const { return children_; }
  /// Depth-first search of the descendants by Id.
  Element* FindById(std::string_view id);
  const std::string& package() const { return context_->package; }
  const SceneContext& context() const { return *context_; }

  // Properties: the scene's values, then whatever timelines and code set.
  const Value* Get(std::string_view name) const;
  float GetFloat(std::string_view name, float fallback = 0.0f) const;
  bool GetBool(std::string_view name, bool fallback = false) const;
  uint32_t GetUnsigned(std::string_view name, uint32_t fallback = 0) const;
  int32_t GetInteger(std::string_view name, int32_t fallback = 0) const;
  uint32_t GetColor(std::string_view name, uint32_t fallback = 0) const;
  std::string_view GetString(std::string_view name) const;
  Vec3 GetVector(std::string_view name, Vec3 fallback = {}) const;
  Quat GetQuaternion(std::string_view name) const;
  /// Compound property (Fill, Stroke), with its own members.
  const PropertyBag* GetCompound(std::string_view name) const;
  void Set(std::string_view name, Value value);

  void SetText(std::string text) { Set("Text", Value{std::move(text)}); }
  std::string_view text() const { return GetString("Text"); }
  /// A second label some visuals show (text_Label2: counts, values).
  void SetSecondaryText(std::string text) { secondary_text_ = std::move(text); }
  const std::string& secondary_text() const { return secondary_text_; }
  /// Shown, and not suppressed.
  bool visible() const { return !suppressed_ && GetBool("Show", true); }
  void SetVisible(bool show) { Set("Show", Value{show}); }
  /// Hidden whatever its timelines say: for entries a host leaves out.
  void Suppress() { suppressed_ = true; }
  bool suppressed() const { return suppressed_; }

  /// Size after anchoring against the parent's current size.
  float width() const;
  float height() const;
  /// Position after anchoring (the element's top-left in its parent).
  Vec3 position() const;

  // Timelines: an element's timelines animate its descendants.
  bool HasNamedFrame(std::string_view name) const;
  /// Jumps to the named frame and plays until a stop command. False when the
  /// element has no such frame.
  bool Play(std::string_view name, bool sounds = true);
  bool playing() const { return playing_; }
  double frame() const { return frame_; }
  /// Shows frame `frame` of this element's timelines and stops there (a
  /// slider's body is posed by its value this way).
  void Seek(double frame);
  /// Advances this element's and all descendants' timelines.
  void Advance(double frames);

  /// Adds a scene built from `node` (a tab's content file) as a child.
  Element* AttachScene(const Node& node, const SceneContext& context);
  /// A copy of `source` (a child of this element) built from the same scene
  /// data, placed after it: new menu entries look exactly like the others.
  /// `visual` names a different skin visual for the copy.
  Element* CloneChild(const Element& source, std::string id, std::string_view visual = {});
  void RemoveChild(Element* child);

  /// Fills a list with `count` copies of its visual's item template
  /// (control_ListItem), `columns` per row. Replaces earlier items.
  std::vector<Element*> PopulateList(size_t count, int columns = 1);
  const std::vector<Element*>& list_items() const { return list_items_; }

  // Focus: shown controls (not scenes). Disabled ones take focus too, and
  // play their visual's Disable frames, as on the console.
  bool focusable() const;
  bool enabled() const { return GetBool("Enabled", true); }
  /// Plays Press (PressCheck when checked), or PressDisable when disabled.
  void Press();
  /// Checkboxes and radio buttons: plays the visual's Check (or plain)
  /// state for the control's focus.
  void SetChecked(bool checked);
  bool checked() const { return checked_; }
  bool focused() const { return focused_; }
  /// The control Nav<direction> names, searched in the enclosing scene.
  Element* Navigate(NavDirection direction);
  /// Plays the visual's KillFocus on `from` and Focus (or InitFocus when
  /// `initial`) on `to`, with the Check and Disable variants that apply.
  static void MoveFocus(Element* from, Element* to, bool initial = false);

 private:
  struct TimelineSet {
    const std::vector<Timeline>* timelines = nullptr;
    const std::vector<NamedFrame>* frames = nullptr;
  };

  Element(const Node& node, Element* parent, const SceneContext* context);
  void Build(const Node& node);
  void ApplyVisual(const Node& visual);
  void AddTimelines(const Node& owner);
  void ApplyFrame(double frame);
  void FireSounds(double from, double to, bool inclusive);
  const PropDef* FindDef(std::string_view name) const;
  void SetPath(std::span<const PropDef* const> path, int32_t index, const Value& value);
  /// Plays `base` + "Check" when checked + "Disable" when disabled, falling
  /// back to fewer suffixes when the visual lacks that frame.
  bool PlayState(std::string_view base, bool sounds);

  Element* parent_ = nullptr;
  const Node* node_ = nullptr;
  const SceneContext* context_ = nullptr;
  std::unique_ptr<SceneContext> owned_context_;  // for AttachScene'd content
  const ClassDef* cls_ = nullptr;
  std::string class_name_;
  PropertyBag props_;
  std::vector<std::unique_ptr<Element>> children_;
  std::string secondary_text_;
  std::vector<Element*> list_items_;
  // Size the parent had when this element was laid out, for anchoring.
  float design_parent_width_ = 0.0f;
  float design_parent_height_ = 0.0f;

  std::vector<TimelineSet> timeline_sets_;
  double frame_ = 0.0;
  bool playing_ = false;
  bool suppressed_ = false;
  bool checked_ = false;
  bool focused_ = false;
};

}  // namespace rex::ui::xui
