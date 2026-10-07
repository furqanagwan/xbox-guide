// Copyright (c) 2026 Xbox Guide contributors. BSD 3-Clause License; see LICENSE.
#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <rex/ui/guide/message_box.h>
#include <rex/ui/guide/xbox_guide.h>
#include <rex/ui/xui/system_update.h>

TEST_CASE("The private backward-compatibility skin hosts source and recovery message boxes",
          "[guide][message_box][local]") {
  const char* path = std::getenv("REXGLUE_GUIDE_FLASH");
  if (!path || !*path)
    SKIP("REXGLUE_GUIDE_FLASH is not set");
  std::string error;
  auto modules = rex::ui::xui::SystemUpdate::ReadModules(std::filesystem::path(path), &error);
  REQUIRE(modules);
  auto assets = rex::ui::guide::GuideAssets::FromUpdate(
      rex::ui::xui::SystemUpdate::FromModules(*modules, &error), &error);
  REQUIRE(assets);
  rex::ui::xui::SceneContext context;
  context.skin = &assets->skin;
  context.package = "huduiskin";
  const std::string choices[] = {"Choose a disc image (ISO)", "Read from disc drive",
                                 "Choose an extracted folder"};
  auto source =
      rex::ui::guide::MessageBoxScene::Create(context, "Game files", "Choose your source", choices);
  REQUIRE(source);
  for (int i = 0; i < 3; ++i) {
    CHECK(source->Activate() == size_t(i));
    source->Move(1);
  }
  const std::string retry[] = {"Retry", "Leave Game"};
  auto recovery = rex::ui::guide::MessageBoxScene::Create(context, "Game source unavailable",
                                                          "Reinsert the disc", retry, 1);
  REQUIRE(recovery);
  CHECK(recovery->Activate() == 1);
  CHECK_FALSE(recovery->root().FindById("Button2")->visible());
}
