// Copyright (c) 2026 Furqan Agwan
// SPDX-License-Identifier: BSD-3-Clause
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <rex/ui/guide/virtual_keyboard.h>
#include <rex/ui/xui/document.h>
#include <rex/ui/xui/package.h>
#include <rex/ui/xui/renderer.h>
#include <rex/ui/xui/runtime.h>

#include "xui_test_data.h"

using namespace rex::ui::xui;

TEST_CASE("Standalone packages and strings preserve console path semantics") {
  std::string error;
  auto package = Package::Parse(xui_test::Xuiz({{"Folder/File.xus", "hello"}}), &error);
  REQUIRE(package);
  auto bytes = package->Find("folder\\FILE.xus");
  CHECK(std::string(bytes.begin(), bytes.end()) == "hello");
  CHECK_FALSE(Package::Parse({0, 1, 2}, &error));
  auto strings = ParseStringTable(xui_test::Xuis({"Xbox Guide", "Settings"}), &error);
  REQUIRE(strings);
  CHECK(strings->at(1) == "Settings");
  CHECK_FALSE(ParseXur(std::vector<uint8_t>{0, 1, 2}, &error));
}

TEST_CASE("Standalone runtime and renderer link without ReXGlue") {
  Node node;
  node.class_name = "XuiCanvas";
  node.cls = FindClass(node.class_name);
  REQUIRE(node.cls);
  SceneContext context;
  auto element = Element::Create(node, context);
  REQUIRE(element);
  CHECK(EaseProgress(0.5f, 0, 0) == Catch::Approx(0.5f));
  ImGui::CreateContext();
  {
    ImDrawList list(ImGui::GetDrawListSharedData());
    list._ResetForNewFrame();
    Render(&list, *element, ImVec2(0, 0), 1.0f, 1.0f, RenderResources{});
  }
  ImGui::DestroyContext();
}

TEST_CASE("Standalone keyboard edits within its buffer limit") {
  rex::ui::guide::VirtualKeyboard keyboard(u"ab", 3);
  CHECK(keyboard.KeyAt(1, 0) == u'q');
  CHECK(keyboard.KeyAt(5, 0) == 0);
  keyboard.MoveCursor(-1);
  REQUIRE(keyboard.Insert(u'c'));
  CHECK(keyboard.text() == u"acb");
  CHECK_FALSE(keyboard.Insert(u'd'));
  REQUIRE(keyboard.Backspace());
  CHECK(keyboard.text() == u"ab");
}
