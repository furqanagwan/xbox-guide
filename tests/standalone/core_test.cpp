// Copyright (c) 2026 Furqan Agwan
// SPDX-License-Identifier: BSD-3-Clause
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <rex/ui/guide/virtual_keyboard.h>
#include <rex/ui/guide/message_box.h>
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

TEST_CASE("Standalone console message boxes retain choices, safe defaults and disabled actions") {
  auto node = [](const char* cls, std::string id) {
    Node n;
    n.class_name = cls;
    n.cls = FindClass(cls);
    auto props = std::make_shared<PropertyBag>();
    const PropDef* id_def = nullptr;
    for (const auto* cls_def : ClassChain(n.cls))
      for (const auto& def : cls_def->props)
        if (def.name == "Id")
          id_def = &def;
    REQUIRE(id_def);
    props->entries.push_back({id_def, Value{std::move(id)}});
    n.props = props;
    return n;
  };
  Document skin;
  auto visual = node("XuiVisual", "XuiMessageBox3");
  auto scene = node("XuiScene", "message");
  scene.children.push_back(node("XuiText", "MessageText"));
  for (int i = 0; i < 3; ++i)
    scene.children.push_back(node("XuiButton", "Button" + std::to_string(i)));
  visual.children.push_back(std::move(scene));
  skin.root.children.push_back(std::move(visual));
  SceneContext context;
  context.skin = &skin;
  const std::string choices[] = {"ISO", "Disc", "Folder"};
  auto box =
      rex::ui::guide::MessageBoxScene::Create(context, "Game files", "Choose source", choices, 2);
  REQUIRE(box);
  context.package = "changed";
  CHECK(box->root().package().empty());  // owns its context
  CHECK(box->focused_choice() == 2);
  CHECK(box->Activate() == 2);
  box->Move(1);
  CHECK(box->Activate() == 0);
  box->SetEnabled(0, false);
  CHECK_FALSE(box->Activate());
  box->Move(-1);
  CHECK(box->Activate() == 2);
  box->SetBody("Reinsert the disc");
  CHECK(std::string(box->root().FindById("MessageText")->text()) == "Reinsert the disc");
  CHECK_FALSE(rex::ui::guide::MessageBoxScene::Create(context, "", "", choices, 3));
  CHECK_FALSE(rex::ui::guide::MessageBoxScene::Create(context, "", "", choices, 0, "missing"));
  const std::string retry[] = {"Retry", "Leave Game"};
  auto recovery = rex::ui::guide::MessageBoxScene::Create(context, "Media", "Reconnect", retry, 1);
  REQUIRE(recovery);
  CHECK_FALSE(recovery->root().FindById("Button2")->visible());
  CHECK(recovery->Activate() == 1);
  box.reset();
  recovery.reset();
  skin.root.children[0].children[0].children.erase(
      skin.root.children[0].children[0].children.begin());
  CHECK_FALSE(rex::ui::guide::MessageBoxScene::Create(context, "", "", choices));
}
