# Xbox Guide development

Read README.md, docs/extraction.md, docs/xbox-guide.md and the assigned issue.
This repository owns the Guide and XUI sources; ReXGlue is a consumer and
provides the current complete host adapter. Keep the standalone scene target
free of ReXGlue runtime dependencies. Preserve existing public include paths
and guest behavior during extraction. Introduce further host interfaces in
small, tested changes rather than removing features to compile.

Use Windows x64, Clang 18 or newer, CMake 3.25 or newer and Ninja. Run standalone
Debug/Release builds and CTest, and the affected ReXGlue unit suite for adapter
changes. Zero tests is not a pass. Real scene/title tests require private
owner-supplied assets; report absent assets and hardware as blocked.

Do not commit console updates, extracted assets, game data or restricted SDK
material. Preserve copyright notices and licenses. Do not introduce CPU JIT
or title-ID hacks. Source provenance is pinned in docs/provenance.json.

Follow .clang-format and .editorconfig. Work on focused branches, preserve
user edits and never force-push. Record validation and limitations. Only close
issues when their tests and acceptance criteria are met.
