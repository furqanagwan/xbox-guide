# Xbox Guide

The Xbox 360 Guide UI extracted from [ReXGlue](https://github.com/furqanagwan/rexglue-sdk).
It runs the console's own XUI scenes rather than recreating their appearance.
BSD-3-Clause; no Microsoft scenes, fonts, images, sounds or game files ship here.

The repository owns the Guide, XUI parser and runtime, ImGui renderer, fonts,
keyboard, notifications and Guide pages. Windows is the current host target.

## Use in a ReXGlue recomp

ReXGlue pins this repository as `thirdparty/xbox-guide`. Clone the SDK with
`--recurse-submodules`, or run `git submodule update --init --recursive`.
Build and install the SDK as usual. Existing title builds, Guide controls,
`REXGLUE_SYSTEM_UPDATE`, `REXGLUE_GUIDE_FLASH` and `rexglue guide-bundle` continue
to work. The SDK installs these headers with its runtime.

See the [Guide behavior and asset workflow](docs/xbox-guide.md).

## Use the scene layer in another recomp

Supply `fmt::fmt` and `imgui::imgui` targets, then:

```cmake
set(XBOX_GUIDE_BUILD_CORE ON CACHE BOOL "" FORCE)
set(XBOX_GUIDE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/xbox-guide)
target_link_libraries(my_recomp PRIVATE xbox_guide::core)
```

The scene target has no ReXGlue runtime dependency. It provides XUIZ/XUIS/XUR
parsing, schema, element trees, timelines, layout helpers, ImGui draw-list
rendering and keyboard editing. Headers and namespaces retain `rex::ui` to
preserve compatibility. Parse scenes into `xui::Document`, build an
`xui::Element` tree, supply textures and fonts with `xui::RenderResources`,
then call `xui::Render` on the host's ImGui draw list. Match this repository's
pinned ImGui revision; other revisions are not validated.

The complete Guide currently uses the ReXGlue host adapter. Its
`guide::GuideHost` takes runtime, input, achievement and content services.
Other SDKs must adapt those services; the complete Guide is not yet a
framework-neutral drop-in. STFS/XEX/LZX extraction, XTT decoding, XMA playback,
guest XAM notifications and title-update installs use ReXGlue services and
are not part of `xbox_guide::core`. A host may supply its own extracted scenes
and resource callbacks to the scene layer. No asset extraction is needed for
the synthetic tests.

## Build the standalone scene layer

From a Windows x64 developer shell with Clang, CMake and Ninja:

```powershell
cmake -S . -B out/build -G "Ninja Multi-Config" -DCMAKE_CXX_COMPILER=clang++ -DXBOX_GUIDE_FETCH_DEPENDENCIES=ON
cmake --build out/build --config Debug
ctest --test-dir out/build -C Debug --output-on-failure
cmake --build out/build --config Release
ctest --test-dir out/build -C Release --output-on-failure
```

Fetching is opt-in. Dependency revisions match the extraction's SDK pins.
The standalone tests check linkage without the SDK, packages, strings,
malformed input and the keyboard. The larger existing Guide suite lives in
`tests/unit/ui` and runs through ReXGlue's `unit_tests` target.

## Ownership and evidence

Guide issues move here with GitHub's native issue transfer. Runtime, input,
content and codegen issues remain in ReXGlue. See [migration](docs/extraction.md),
[source provenance](docs/provenance.json) and [original source history](docs/source-history.txt).
Existing console/title observations predate extraction; they do not establish
compatibility with another SDK. The owner pad play session remains pending.
