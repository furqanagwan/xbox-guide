# Extraction from ReXGlue

Date: 2026-10-06. Source commit: `d1a87b4ef0a09c7a7813ab2a2b27976de01de203`.

Guide and XUI sources, public headers and their existing unit tests are now
owned by this repository. Original paths and notices are retained. Git history
is available at the immutable source commit; docs/source-history.txt records
the affected commits. No upstream subsystem was imported.

ReXGlue consumes a pinned submodule and attaches the sources to its existing
rexui object library. That preserves the single runtime copy of cvar and
logging state, DLL exports, installed headers and guest service integration.
The SDK keeps its CLI registration and title embedding helper as adapters.
ResolveFile was split from runtime.cpp into resolve_file.cpp so the reusable
scene runtime can link without the SDK's system-update loader. Its behavior
is unchanged. Standalone and SDK consumers are separate build paths; a title
using the SDK must not also link a second copy of xbox_guide::core.

The complete Guide still depends on ReXGlue input, achievements, content,
settings, UI dialogs and audio. The standalone scene layer is reusable now;
porting all Guide pages to another framework needs service adapters and new
validation. This extraction does not claim that work is complete.

## Issue ownership

Transfer source issues 127 (Guide epic), 132 (look/mods/cheats), 143 (Home and
blades), 145 (Manage Game), 153 (Guide title updates), 161 (keyboard), and
166 (PC backward-compatibility Guide) using GitHub transfer. Existing state,
comments and redirects are retained. The epic remains open.

Keep 137 (render resolution), 139 (input backends and battery), 141 (guest
achievement enumerators), 154 (disc/ISO loading) and runtime/codegen issues
in ReXGlue. Their Guide connections do not move ownership of the underlying
SDK service. Issue 153 also includes SDK codegen: future changes to generating
the update executables belong in ReXGlue, with a link to its transferred record.

The completed transfer map and validation results will be recorded below.
