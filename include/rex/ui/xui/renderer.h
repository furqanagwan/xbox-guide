/**
 * @file        rex/ui/xui/renderer.h
 * @brief       Draws live XUI elements onto an ImGui draw list (RG-GDK-041)
 *
 * @copyright   Copyright (c) 2026 Tom Clay
 * @license     BSD 3-Clause License
 */

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <string_view>

#include <imgui.h>

#include <rex/ui/xui/text_scroll.h>

namespace rex::ui::xui {

class Element;

// Text style bits, read from the skin's named label visuals (docs/xbox-guide.md).
enum TextStyleFlags : uint32_t {
  kTextBold = 0x0001,
  kTextNoWrap = 0x0010,
  kTextRight = 0x0200,
  kTextCenter = 0x0400,
  kTextVerticalCenter = 0x1000,
  kTextEllipsis = 0x4000,
};

/// XUI point sizes to scene units. Measured against a 1080p capture of the
/// dashboard 2.0.17559 guide, where "Xbox Home" stands 24 of its row's 61
/// pixels (RG-GDK-043); the earlier 1.2, from the scenes' proportions alone,
/// drew text a quarter too small.
constexpr float kPointToSceneUnits = 1.6f;

/// The Segoe Xbox gamerscore glyph, private use U+E00A, in UTF-8 (the skin's
/// btn_Count_achiev glyph_presenter). The console's XUI fonts are encrypted,
/// so text drawing puts the image `kGamerscoreImage` (a white disc with a G
/// cut out, from sharedres GScore_white.png) in its place.
inline constexpr std::string_view kGamerscoreGlyph = "\xEE\x80\x8A";
inline constexpr std::string_view kGamerscoreImage = "rex://gamerscore";

struct RenderResources {
  /// Texture for a scene image path, resolved against `package`. Returns an
  /// empty ImTextureRef when missing and sets the image's pixel size.
  std::function<ImTextureID(std::string_view path, std::string_view package, int* width,
                            int* height)>
      texture;
  ImFont* regular_font = nullptr;
  ImFont* bold_font = nullptr;
  /// Draws an image path as vectors instead of its texture, so it stays sharp
  /// at any resolution. `to_screen` maps the image element's own units.
  /// Returns false to draw the path as usual.
  std::function<bool(ImDrawList& list, std::string_view path,
                     const std::function<ImVec2(ImVec2)>& to_screen, float opacity)>
      vector_image;
  /// When each overflowing text began scrolling (TextScrollOffset), kept
  /// across frames; null turns the scrolling off.
  std::shared_ptr<std::unordered_map<const Element*, TextScroll>> text_scroll =
      std::make_shared<std::unordered_map<const Element*, TextScroll>>();
  /// Physical pixels per draw list unit: the window's DPI scale when the host
  /// lays ImGui out in logical units (3 for 4K at 300% scaling). Gradients
  /// are subdivided and soft edges sized for the physical pixels.
  float pixels_per_point = 1.0f;
};

/// The span of a gradient's last stops, as positions along the gradient.
struct GradientSpan {
  float start = 0;
  float end = 0;
};
/// A radial gradient that ends by fading its colour out is the skin's soft
/// edge for a disc (the radio buttons' discs and dot): about 1.5 pixels at the
/// console's 720p. Drawn `pixels_per_unit` larger, the fade keeps that width
/// on screen, around the same middle, instead of growing into a blur.
GradientSpan EdgeFadeOnScreen(GradientSpan fade, float pixels_per_unit);

/// Draws `root` with scene unit (x, y) at screen (origin + scale * (x, y)).
void Render(ImDrawList* list, const Element& root, ImVec2 origin, float scale, float opacity,
            const RenderResources& resources);

}  // namespace rex::ui::xui
