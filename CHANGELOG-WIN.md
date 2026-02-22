# CHANGELOG-WIN.md — Windows-Specific Development History

> This file tracks changes made exclusively on the `windows-dev` branch that relate to
> the native Windows GUI (`mcaster1win.exe`), Visual Studio build infrastructure, and
> Windows-specific runtime fixes.  Cross-platform server changes are in [CHANGELOG.md](CHANGELOG.md).

---

## [2.5.1-beta.1-win] — 2026-02-18/19 — Windows-Dev Branch

### Visual Studio 2022 (v17) Build Infrastructure

#### Project & Solution Modernisation
- Migrated solution and project files to **Visual Studio 2022 / MSBuild v17** toolset (`v143`)
- Added `Mcaster1DNAS.sln` as the primary solution file replacing the legacy VC6-era `.dsw`
- All four sub-projects compile cleanly under x64 Debug and Release configurations:
  - `mcaster1win.exe` — Windows GUI tray application
  - `mcaster1.exe` — Headless console server
  - `mcaster1Service.exe` — Windows Service wrapper
  - `mcaster1Console.exe` — Lightweight console runner
- Removed stale VC6 `.dsp/.dsw` build files from active build paths (retained in `Archive/` for historical reference)

#### vcpkg Dependency Integration
- All third-party libraries now sourced through **vcpkg** (`x64-windows` triplet):
  - `libxml2`, `libxslt`, `libcurl`, `openssl`, `libyaml`, `ogg`, `vorbis`, `theora`, `speex`, `pthreads`
- Include paths, library paths, and DLL copy steps wired into `Mcaster1Win.vcxproj` `<AdditionalIncludeDirectories>` and post-build events

#### Automatic Git Version Stamping (PreBuildEvent)
- Added `windows/git-version.ps1` — PowerShell script invoked as a **PreBuildEvent**
- Generates `src/git_hash.h` at build time with `GIT_VERSION`, `GIT_HASH`, `GIT_BRANCH`, `GIT_DIRTY`
- Window title bar now shows `Mcaster1DNAS vX.Y.Z-dev.HASH` (or `-modified` suffix when working tree is dirty)
- Confirmed output: `GIT_VERSION = 2.5.1-dev.c2bad3c-modified`

---

### Windows GUI (`mcaster1win.exe`) Enhancements

#### Command-Line Flags
Added full CLI argument parsing to `Mcaster1WinDlg.cpp`:

| Flag | Behaviour |
|------|-----------|
| `-c <file>` | Specify config file path (overrides default `.\mcaster1.yaml` search) |
| `-s` | Auto-start the server on launch (no button click required) |
| `-m` | Start minimised to system tray |
| `-v` | Print version string and exit |
| `-h` | Print help and exit |

These allow the GUI to be scripted, launched from shortcuts with specific configs, or started automatically as a login item.

#### ResizableLib Integration (Resizable Dialogs)
Integrated **ResizableLib by Paolo Messina** (MIT/Artistic-2.0) into the project:

- `windows/ResizableLib/` — full library source (8 compilation units) added to the repository
- All four MFC dialog classes migrated from `CDialog` → `CResizableDialog`:
  - `CMcaster1WinDlg` (main window)
  - `CTabPageSSL` (shared tab base class → transitively: `CStatus`, `CStatsTab`, `CConfigTab`)
- `AddAnchor()` calls activated for all resizable controls:
  - Main window: `IDC_MAINTAB` anchored `TOP_LEFT → BOTTOM_RIGHT`
  - Status tab: `IDC_GLOBALSTAT_LIST` fills tab; `IDC_STATIC_RUN`, `IDC_RUNNINGFOR` pinned to bottom
  - Stats tab: `IDC_SOURCELIST` fixed-width left panel; `IDC_STATSLIST` fills right side
  - Config tab: `IDC_CONFIG` editor fills entire tab
- `EnableSaveRestore("mcaster1win")` — window position and size now persist between sessions (registry key `HKCU\Software\mcaster1win\positions`)
- 40-line hand-rolled `OnSize()` pixel math replaced with 8-line ResizableLib cascade:
  ```cpp
  CResizableDialog::OnSize(nType, cx, cy);   // moves IDC_MAINTAB via anchor
  m_MainTab.ResizeDialog(active, ...);        // propagates to active tab page
  ```
- **Bugs fixed during integration:**
  - `IDC_STATICBLACK` referenced in old `AddAnchor` calls but not present in dialog resource → removed (banner uses `IDC_STATIC`)
  - `IDC_FILLER1` similarly absent from `IDD_STATSDIALOG` → removed
  - `EnableSaveRestore("mcaster1win", "positions")` passed a string as second arg; actual API signature is `EnableSaveRestore(LPCTSTR pszSection, BOOL bRectOnly = FALSE)` → fixed to `EnableSaveRestore("mcaster1win")`

**Build configuration for ResizableLib in vcxproj:**
- Each ResizableLib `.cpp` entry carries per-file `<PrecompiledHeader>NotUsing</PrecompiledHeader>` and `<AdditionalIncludeDirectories>$(ProjectDir)ResizableLib;...</AdditionalIncludeDirectories>` so the library finds its own `StdAfx.h` (which defines `WINVER 0x0501`, `uxtheme.h`, `shlwapi.h`) without conflicting with the main project's precompiled header
- Global include path has `$(ProjectDir)ResizableLib` placed *after* `$(ProjectDir)` to prevent include order conflicts

---

### YAML Configuration — Windows Runtime Fix

#### Root Cause
`yaml_parser_load()` crashed on Windows when called with a `FILE*` opened by the main exe.
`yaml.dll` (from vcpkg) was compiled against a different C runtime (UCRT) than the MSVC-compiled
exe, making the `FILE` struct layout binary-incompatible. Any read attempt inside libyaml
dereferenced invalid struct offsets → access violation.

The last successful trace line before the crash was:
```
[yaml] yaml_parser_load          ← trace written BEFORE the call; nothing after = crash inside
```

XML config was unaffected because `libxml2` opens the file internally using its own CRT handles.

#### Fix Applied (`src/cfgfile_yaml.c` — `config_parse_yaml_file()`)
- Read the entire config file into a heap buffer using the exe's own CRT `fread()` / `ftell()`
- Close the `FILE*` before calling any libyaml function
- Call `yaml_parser_set_input_string(&parser, buf, len)` instead of `yaml_parser_set_input_file(&parser, fp)`
- No `FILE*` ever crosses the DLL boundary

**Verified by full trace log after fix:**
```
[yaml] file read into memory
[yaml] yaml_parser_initialize ok
[yaml] yaml_parser_load
[yaml] yaml_parser_load ok          ← was the crash point; now passes cleanly
[yaml] root node ok
[yaml] root is mapping
[yaml] config_init_configuration ok
[yaml] yaml_parse_root
[yaml] yaml_parse_root done
[si]  connection_setup_sockets ok
[si]  server_init done -> returning 0
[3]   server_process starting
```

#### Diagnostic Infrastructure (retained for future use)
- `CFG_TRACE(msg)` macro in `src/cfgfile.c` — appends to `mcaster1win_start.log` in the exe's working directory (WIN32 only, no-op on Linux/macOS)
- `YAML_TRACE(msg)` macro in `src/cfgfile_yaml.c` — same log file, `[yaml]` prefix
- 23 trace checkpoints from server init through `server_process starting`

---

### Graphics & Branding

#### Full Visual Rebrand: Icecast2 → Mcaster1DNAS
- Replaced all Icecast2 bitmap assets in `windows/` with Mcaster1DNAS branded equivalents:
  - `mcaster1dnaslogo2.bmp` — main application logo
  - `mcaster1dnastitle.bmp` — title bar / about screen banner
  - `running.bmp` / `stopped.bmp` — server status indicator icons
  - `credits.bmp` — credits screen image
- Dialog resource (`Mcaster1Win.rc`) updated to reference new bitmaps
- Application icon (`mcaster1.ico`) updated to Mcaster1DNAS icon
- Installer scripts (`mcaster1dnas.iss`, `mcaster164.nsis`) updated with new branding, paths, and version strings
- **Original developer credits preserved in full** — the Credits dialog/screen continues to display acknowledgment of Xiph.Org Foundation (Icecast2), Karl Heyes (Icecast-KH), Jack Moffitt, Michael Smith, oddsock, and all upstream contributors in accordance with GNU GPL v2 attribution requirements

---

### Configuration Files (Windows Dev)

#### XML Config (`windows/x64/Debug/mcaster1.xml`)
- Created Windows-specific XML development config targeting `127.0.0.1:9300` (HTTP) and `127.0.0.1:9443` (HTTPS)
- Absolute Windows paths for basedir, logdir, webroot, adminroot, ssl-certificate, pidfile
- Used to verify XML code path and isolate YAML crash during diagnosis

#### YAML Config (`windows/x64/Debug/mcaster1.yaml`)
- Created equivalent YAML development config with identical settings to `mcaster1.xml`
- Both configs now start the server cleanly and produce identical runtime behaviour
- Config auto-detection (`detect_config_format()`) reads the first non-whitespace byte: `<` → XML, `#`/`-`/word → YAML

---

### Build Verification

| Test | Result |
|------|--------|
| MSBuild x64 Debug — 0 errors | PASS |
| Window launch + 15s resize stress test | PASS |
| Resize to 900×700 | PASS |
| Maximize → 1550×830 | PASS |
| Restore → 900×700 | PASS |
| Process survival (no crash) | PASS |
| XML config server start (`-c mcaster1.xml -s`) | PASS — 15s healthy |
| YAML config server start (`-c mcaster1.yaml -s`) | PASS — 15s healthy |
| Full YAML trace (23/23 checkpoints) | PASS |

---

## Known Issues / In Progress

- **Config dialog editor** — planned visual editor for `mcaster1.yaml`/`mcaster1.xml` directly inside the GUI (no hand-editing required)
- **Stats tab auto-refresh** — GUI polls admin HTTP API; requires server to be listening on the configured port for tabs to populate
- **Windows Service installer** — `mcaster1Service.vcxproj` compiles; NSIS/Inno Setup installer packaging not yet automated in CI

---

## Upcoming Windows-Specific Roadmap

See also the main [README.md](README.md) roadmap section.

- [ ] **Config Dialog Editor** — GUI-based config editor with validation, replacing raw file editing
- [ ] **Enhanced Documentation** — Windows-specific installation guide, dependency setup wizard
- [ ] **Podcast & On-Demand File Manager** — GUI panel for managing podcast episode files and on-demand audio assets
- [ ] **RSS Podcast Feed Generator** — Built-in feed builder that publishes an RSS 2.0 / Apple Podcasts compatible feed from the on-demand library
- [ ] **Podcast Core Server Features** — Episode scheduling, chapter markers, podcast-specific ICY metadata fields, listener analytics per episode
- [ ] **Windows Installer (MSI/Inno)** — Full automated installer with dependency bundling (vcredist, vcpkg DLLs, config wizard)
- [ ] **Auto-Update Mechanism** — In-app update check against GitHub releases
- [ ] **Dark Mode** — Windows 10/11 dark mode support via `DwmSetWindowAttribute`

---

*Branch: `windows-dev` | Maintainer: David St John <davestj@gmail.com> | MediaCast1*
