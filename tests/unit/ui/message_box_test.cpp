// Copyright (c) 2026 Xbox Guide contributors. BSD 3-Clause License; see LICENSE.
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <cstdlib>
#include <rex/ui/guide/message_box.h>
#include <rex/ui/guide/xbox_guide.h>
#include <rex/ui/xui/system_update.h>

TEST_CASE("Both private Guide skins host source and recovery message boxes",
          "[guide][message_box][local]") {
  const bool original_xbox = GENERATE(false, true);
  const char* asset_variable = original_xbox ? "REXGLUE_GUIDE_FLASH" : "REXGLUE_SYSTEM_UPDATE";
  const char* path = std::getenv(asset_variable);
  INFO(asset_variable);
  if (!path || !*path)
    SKIP("Selected private Guide assets are not set");
  std::string error;
  auto modules = rex::ui::xui::SystemUpdate::ReadModules(std::filesystem::path(path), &error);
  REQUIRE(modules);
  auto assets = rex::ui::guide::GuideAssets::FromUpdate(
      rex::ui::xui::SystemUpdate::FromModules(*modules, &error), &error,
      original_xbox ? rex::ui::guide::GuidePresentation::OriginalXbox
                    : rex::ui::guide::GuidePresentation::Xbox360);
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
  auto downloads =
      rex::ui::guide::ActiveDownloadsScene::Create(assets->options_notifications, context);
  REQUIRE(downloads);
  downloads->Update({});
  CHECK(downloads->size() == 0);
  // Initial zero rows must still remove the notification page's controls.
  for (const char* id :
       {"chkShow", "chkSound", "chkShowMovies", "chkShowIPTV", "labelSoundDisabled", "XuiLabel2"}) {
    if (auto* control = downloads->root().FindById(id))
      CHECK_FALSE(control->visible());
  }
  CHECK(downloads->root().FindById("XuiLabel1")->text() == "Nothing is downloading.");
  int cancelled = 0;
  downloads->Update({{"Game source extraction", "50%", "Copying", [&] { ++cancelled; }}});
  REQUIRE(downloads->size() == 1);
  CHECK(downloads->row(0)->secondary_text() == "50%");
  downloads->Activate();
  CHECK(cancelled == 1);
  downloads->Update({{"Game source extraction", "Completed", "Copied to PC", {}}});
  downloads->Activate();
  CHECK(cancelled == 1);
}
