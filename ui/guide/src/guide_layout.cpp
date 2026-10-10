/**
 * @file        ui/guide/src/guide_layout.cpp
 * @brief       GuideMain's tab blades for three tabs (Games & Apps, Home, Settings)
 *
 * @copyright   Copyright (c) 2026 Tom Clay
 * @license     BSD 3-Clause License
 */

#include <rex/ui/guide/guide_layout.h>

#include <algorithm>
#include <initializer_list>
#include <map>
#include <string>

namespace rex::ui::guide {
namespace {

struct Range {
  int32_t first, last;
};

// The Tabscene's frames for each layout (17559 GuideMain named frames).
constexpr Range kRightRanges[] = {{1, 24}, {73, 180}};   // 1To2, 2To1; 1Open to 2CloseEnd
constexpr Range kLeftRanges[] = {{49, 72}, {232, 283}};  // 3To4, 4To3; 4Open to 4CloseEnd
constexpr Range kLeftSwitches[] = {{49, 72}};

bool InRanges(std::initializer_list<Range> ranges, int32_t frame) {
  return std::any_of(ranges.begin(), ranges.end(),
                     [&](const Range& r) { return frame >= r.first && frame <= r.last; });
}

xui::Node* FindNode(xui::Node& node, std::string_view id) {
  for (xui::Node& child : node.children) {
    if (child.id() == id) {
      return &child;
    }
    if (xui::Node* found = FindNode(child, id)) {
      return found;
    }
  }
  return nullptr;
}

bool SameProps(const xui::Timeline& a, const xui::Timeline& b) {
  return std::equal(a.props.begin(), a.props.end(), b.props.begin(), b.props.end(),
                    [](const xui::AnimatedProperty& x, const xui::AnimatedProperty& y) {
                      return x.path == y.path && x.index == y.index;
                    });
}

// `target`'s keyframes in `ranges` replaced by `source`'s there.
void Follow(xui::Timeline& target, const xui::Timeline& source,
            std::initializer_list<Range> ranges) {
  std::erase_if(target.keyframes,
                [&](const xui::Keyframe& k) { return InRanges(ranges, k.frame); });
  for (const xui::Keyframe& k : source.keyframes) {
    if (InRanges(ranges, k.frame)) {
      target.keyframes.push_back(k);
    }
  }
  std::sort(target.keyframes.begin(), target.keyframes.end(),
            [](const xui::Keyframe& a, const xui::Keyframe& b) { return a.frame < b.frame; });
}

// `track`'s Show false at every keyframe in `ranges`.
void Hide(xui::Timeline& track, std::initializer_list<Range> ranges) {
  for (size_t i = 0; i < track.props.size(); ++i) {
    const auto& path = track.props[i].path;
    if (path.empty() || path.back()->name != "Show") {
      continue;
    }
    for (xui::Keyframe& k : track.keyframes) {
      if (InRanges(ranges, k.frame) && i < k.values.size()) {
        k.values[i] = xui::Value{false};
      }
    }
  }
}

}  // namespace

bool UseThreeTabs(xui::Node& guide_main) {
  xui::Node* tabs = FindNode(guide_main, "Tabscene");
  if (!tabs) {
    return false;
  }
  // Copies of the scene's own tracks, so a label that moves to its
  // neighbour's track is read before that neighbour is changed.
  std::map<std::string, xui::Timeline, std::less<>> original;
  for (const xui::Timeline& t : tabs->timelines) {
    original.emplace(t.target_id, t);
  }
  auto track = [&](std::string_view id) -> xui::Timeline* {
    for (xui::Timeline& t : tabs->timelines) {
      if (t.target_id == id) {
        return &t;
      }
    }
    return nullptr;
  };
  struct Move {
    std::string_view target, source;
    std::initializer_list<Range> ranges;
  };
  const Move moves[] = {
      {"txt_Settings", "txt_Media", {kRightRanges[0], kRightRanges[1]}},
      {"txt_Games", "txt_home", {kLeftRanges[0], kLeftRanges[1]}},
      {"txt_home", "txt_Media", {kLeftRanges[0], kLeftRanges[1]}},
      {"Tab2", "Tab3", {kLeftSwitches[0]}},
      {"txt_homeSel", "txt_MediaSel", {kLeftSwitches[0]}},
  };
  for (const Move& m : moves) {
    const auto source = original.find(m.source);
    if (!track(m.target) || source == original.end() ||
        !SameProps(*track(m.target), source->second)) {
      return false;
    }
  }
  xui::Timeline* right_blade = track("Blade5");
  xui::Timeline* left_blade = track("Blade6");
  if (!right_blade || !left_blade) {
    return false;
  }

  for (const Move& m : moves) {
    Follow(*track(m.target), original.find(m.source)->second, m.ranges);
  }
  Hide(*right_blade, {kRightRanges[0], kRightRanges[1]});
  Hide(*left_blade, {kLeftRanges[0], kLeftRanges[1]});

  // Home to Settings is 3To4's frames; 2To3 and 3To2 have no use.
  std::erase_if(tabs->named_frames, [](const xui::NamedFrame& f) {
    return f.name.starts_with("2To3") || f.name.starts_with("3To2");
  });
  for (xui::NamedFrame& f : tabs->named_frames) {
    if (f.name.starts_with("3To4")) {
      f.name.replace(0, 4, "2To4");
    } else if (f.name.starts_with("4To3")) {
      f.name.replace(0, 4, "4To2");
    }
  }
  return true;
}

}  // namespace rex::ui::guide
