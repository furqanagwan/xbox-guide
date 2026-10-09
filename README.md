# Xbox Guide

The Xbox 360 Guide for recompiled games: the menu that opens over a game,
with achievements, settings, notifications, the keyboard and Leave Game. It
plays the console's own Guide scenes instead of redrawing them, following how
Microsoft's Xbox backward compatibility shows the Guide on Xbox on PC and the
next-generation Xbox (Project Helix) app.

No Microsoft scenes, fonts, images or sounds are included. Each player's build
reads them from their own console's system update.

> Status: alpha. Used by [ReXGlue](https://github.com/furqanagwan/rexglue-sdk).

## Two parts

| Part | What it is | Depends on |
| --- | --- | --- |
| `xbox_guide::core` | Reads and draws Xbox 360 XUI scenes with ImGui | `fmt`, `imgui` |
| The full Guide | Pages, achievements, notifications, settings | ReXGlue's runtime |

Any recompilation project can use `core`. The full Guide currently needs
ReXGlue's services.

## Use it in ReXGlue

ReXGlue includes this repository as the `thirdparty/xbox-guide` submodule.
Clone ReXGlue with `--recurse-submodules` and build it as usual. See
[Guide behavior and assets](docs/xbox-guide.md).

## Use the scene layer in another project

```cmake
set(XBOX_GUIDE_BUILD_CORE ON CACHE BOOL "" FORCE)
add_subdirectory(external/xbox-guide)
target_link_libraries(my_game PRIVATE xbox_guide::core)
```

Parse a scene into `xui::Document`, build an `xui::Element` tree, then call
`xui::Render` on an ImGui draw list. Host-provided pages (message boxes, lists,
file pickers, downloads) are described in [host UI](docs/host-ui.md).

## Build and test on its own

From a Visual Studio x64 developer prompt:

```powershell
cmake -S . -B out/build -G "Ninja Multi-Config" -DCMAKE_CXX_COMPILER=clang++ -DXBOX_GUIDE_FETCH_DEPENDENCIES=ON
cmake --build out/build --config Release
ctest --test-dir out/build -C Release --output-on-failure
```

## Contributing

Work on a branch and open a pull request; `main` only changes through reviewed
PRs with a passing build. Guide issues live here; runtime and input issues
live in ReXGlue. Where the code came from is in
[provenance](docs/provenance.json).

## License

BSD-3-Clause. Not affiliated with or endorsed by Microsoft or Xbox.
