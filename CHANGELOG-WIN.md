# CHANGELOG-WIN.md — Windows-Specific Development History

> This file tracks changes made exclusively on the `windows-dev` branch that relate to
> the native Windows GUI (`mcaster1win.exe`), Visual Studio build infrastructure, and
> Windows-specific runtime fixes.  Cross-platform server changes are in [CHANGELOG.md](CHANGELOG.md).

---

## [2.5.2-beta] — 2026-02-24 — Windows Beta Release

### Summary
First Windows beta release of Mcaster1DNAS 2.5.2. Ships as a self-contained NSIS installer
(`mcaster1dnas_win64_v2.5.2-beta_setup.exe`) with a fully functional Windows Service,
GUI application (sidecar mode), SSL support, and automated build pipeline.

### New: NSIS Installer (`windows/installer/mcaster1dnas.nsi`)
Full NSIS 3.x installer replacing the legacy Inno Setup scripts:

| Section | Contents |
|---------|----------|
| Core Application | All EXEs, vcpkg DLLs, both YAML configs, `ssl/localhost.pem`, `logs/` dir |
| Web Interface | `web/` + `admin/` directories |
| Local Documentation | `docs/` + key Markdown files |
| SSL Test Certs | `ssl/temp/` (optional) |
| Windows Service | Registers `mcaster1Service.exe` as auto-start service |
| Start Menu | `Start Menu > Mcaster1 > Mcaster1DNAS` hierarchy |
| Desktop Shortcuts | GUI launcher, Start/Stop Service, Documentation |

- Firewall rules created for all 4 ports: HTTP 9330, HTTPS 9443, GUI HTTP 9033, GUI HTTPS 9344
- Service start/stop shortcuts use `PowerShell -Verb RunAs` for UAC elevation
- Documentation shortcut opens `docs/index.html` in the default browser
- LZMA compression; installer output ~5.3 MB

### New: Build Pipeline (`windows/installer/build-installer.ps1`)
Automated three-step build: MSBuild → DLL sync → NSIS compilation.

Key fixes in this release:
- **Em dash encoding crash** — `—` characters (UTF-8 byte 0x94 / Windows-1252 `"`) caused
  PowerShell 5.1 to treat them as string terminators; all replaced with ASCII hyphens.
  File saved UTF-8 without BOM.
- **vcpkg DLL sync step (Step 2a)** — explicitly copies all `x64-windows\bin\*.dll` to
  `windows\x64\Debug\` before the verification check, eliminating intermittent missing DLL
  failures from MSBuild post-build events.
- **PowerShell drive reference** — `"$Var:"` → `"${Var}:"` to prevent PS parsing `$Var:` as a
  drive letter reference.
- Build flags: `-SkipBuild`, `-SkipImages`, `-SkipNSIS` for incremental rebuilds.

### Bug Fix: Windows Service Binary
**Before:** NSIS `sc create` used `mcaster1.exe` (console app). Console app never calls
`StartServiceCtrlDispatcher`; SCM timed out after 30 s.
**After:** `sc create ... binPath= "\"$INSTDIR\mcaster1Service.exe\" \"$INSTDIR\""`.

### Bug Fix: Windows Service Default Config
**Before:** `ServiceMain` defaulted to `mcaster1.xml`.
**After:** Default set to `mcaster1dnas-console.yaml`.

### Bug Fix: GUI Working Directory
**Before:** Relative paths in YAML config (`./logs`, `./web`, `./ssl`) resolved to wrong
location when `mcaster1win.exe` was launched from Start Menu shortcuts or explorer.
**After:** `SetCurrentDirectoryA(exedir)` added as the first statement in `InitInstance()`.

### New: Documentation Landing Page (`docs/index.html`)
Dark-theme HTML page with links to all docs and a port reference table. Installed to
`$INSTDIR\docs\index.html`; opened by the Documentation desktop/Start Menu shortcut.

### New: GitHub Actions Workflow (windows-dev)
Added CI/CD workflow for the `windows-dev` branch targeting Windows x64 builds.

### Verified Working (2026-02-24)

| Test | Result |
|------|--------|
| NSIS installer builds (5.3 MB LZMA) | PASS |
| `mcaster1Service.exe` as Windows Service, auto-start | PASS |
| `http://127.0.0.1:9330/status.xsl` | HTTP 200 |
| `https://127.0.0.1:9443/status.xsl` | HTTP 200 (TLS) |
| `https://127.0.0.1:9443/admin/` | HTTP 401 (correct auth enforcement) |
| GUI (`mcaster1win.exe`) starts on 9033/9344, coexists with service | PASS |
| Log files writing to `$INSTDIR\logs\` | PASS |
| Start Menu shortcuts (Launch, Start/Stop Service, Docs) | PASS |
| UAC elevation on service shortcuts | PASS |

---

## [2.5.1-beta.2-win] — 2026-02-22 — Windows-Dev Branch

### Critical Bug Fixes

#### HTTP Admin Authentication — `WWW-Authenticate` Header Missing on Windows
**Root cause:** A struct field order mismatch in the MSVC-specific code path in `src/params.c`.
The `mc_param_t` struct is declared as `{next, flags, name_len, value_len, name, value, …}` but
both `mc_params_printf()` and `mc_http_printf()` MSVC blocks used a positional initializer:
```c
mc_param_t hdr = { NULL, (char*)name, content, flags };
//                 ^next  ^flags(!)   ^name_len(!) ...
```
This left `hdr.name = NULL` and `hdr.value = NULL`.  `mc_http_apply()` checked for both null
and returned `-1` immediately — silently discarding **every response header set via
`mc_http_printf()`** on Windows.  Affected headers included: `WWW-Authenticate`, `Content-Type`,
`Content-Length`, `Location`, `Content-Range`, `User-Agent`, and all per-response headers.

**Effect:** Admin 401 responses had no `WWW-Authenticate` header, so browsers displayed
an error page instead of a credentials dialog.  Other endpoints lost `Content-Type` etc.

**Fix (`src/params.c`):** Replaced wrong positional initializers with `memset` + explicit field
assignments in both `mc_params_printf` and `mc_http_printf` MSVC blocks:
```c
mc_param_t hdr;
memset (&hdr, 0, sizeof hdr);
hdr.name  = (char*)name;
hdr.value = content;
hdr.flags = (uint32_t)flags;
```
Affects both `Mcaster1Win.vcxproj` and `Mcaster1Console.vcxproj` builds.

---

### New Features

#### Per-Listener SSL Enforcement (`ssl: true/false`)
- Added `int ssl` field to `listener_t` / `_listener_t` struct (`src/cfgfile.h`)
- XML config: `<ssl>0</ssl>` or `<ssl>1</ssl>` inside `<listen-socket>` block
- YAML config: `ssl: false` or `ssl: true` per entry in `listen-sockets:`
- `global.ssl_on_sock[]` parallel array tracks per-socket SSL requirement
- `connection_peek()` enforces policy strictly:
  - Port with `ssl: true` → rejects plain HTTP, accepts TLS only
  - Port with `ssl: false` → rejects TLS, accepts plain HTTP only
  - Port with no `ssl:` key → auto-detects as before (backwards-compatible)
- WARN log entries when policy is violated: `"TLS rejected on plain-HTTP port"` /
  `"Plain HTTP rejected on SSL-only port"`

**Example YAML:**
```yaml
listen-sockets:
  - port: 9330
    bind-address: "127.0.0.1"
    ssl: false      # plain HTTP only
  - port: 9443
    bind-address: "127.0.0.1"
    ssl: true       # TLS only
```

#### SSL Certificate Generation CLI (`src/ssl_gen.h` / `src/ssl_gen.c`)
New cross-platform OpenSSL-based certificate/CSR generator guarded by `#ifdef HAVE_OPENSSL`.

**New flags (all platforms):**

| Flag | Description |
|------|-------------|
| `--ssl-gencert` | Trigger SSL generation mode (server does not start) |
| `--ssl-gentype=selfsigned\|csr` | Output type: self-signed cert or CSR |
| `--subj="/C=.../CN=..."` | X.509 subject string (standard openssl format) |
| `--ssl-gencert-savepath=<dir>` | Output directory (created if absent) |
| `--ssl-gencert-addtoconfig=true` | Patch `ssl-certificate`/`ssl-private-key` paths in the `-c` config file |

**Self-signed output (`--ssl-gentype=selfsigned`):**
- `<savepath>/selfsigned.key` — PEM private key
- `<savepath>/selfsigned.crt` — PEM certificate
- `<savepath>/selfsigned.pem` — combined cert+key (for server config)

**CSR output (`--ssl-gentype=csr`):**
- `<savepath>/server.key` — PEM private key
- `<savepath>/server.csr` — PEM certificate signing request

**Windows C++ wrapper (`windows/SslGen.h` / `windows/SslGen.cpp`):**
- `CSslGen::Run(SslGenParams)` — calls `ssl_gen_run()` from C code
- `win_ssl_gencert(argc, argv)` — C entry point for `Mcaster1Win.cpp`
- When `--ssl-gencert` present in WinUI args: generates cert, shows `MessageBox` with result,
  then exits without launching the server GUI

**Example usage (console):**
```
mcaster1.exe --ssl-gencert --ssl-gentype=selfsigned \
  --subj="/C=US/ST=TX/L=Dallas/O=MyStation/CN=localhost" \
  --ssl-gencert-savepath=ssl/certs \
  --ssl-gencert-addtoconfig=true -c windows/mcaster1dnas.yaml
```

See [docs/SSL_CERT_GENERATION.md](docs/SSL_CERT_GENERATION.md) for full guide.

#### Config File Auto-Discovery
When no `-c <file>` argument is given the server searches in this order:
1. `<exe-directory>/mcaster1dnas.yaml`
2. `./mcaster1dnas.yaml` (current working directory)
3. If neither found → prompt user

- **Console / Linux / macOS prompt:** reads a path from stdin
- **WinUI dialog:** opens a `GetOpenFileName` file-picker dialog (`*.yaml;*.xml`)

#### Windows GUI Log Viewer Tabs (4 new tabs)
Added `LogTab.h` / `LogTab.cpp` implementing `CLogTab : CTabPageSSL`:

| Tab | Log file |
|-----|----------|
| Access Log | `cfg->access_log.name` in `cfg->log_dir` |
| Error Log | `cfg->error_log.name` in `cfg->log_dir` |
| YP Health | `cfg->yp_logfile[0]` (first YP directory) |
| Playlist Log | `cfg->playlist_log.name` in `cfg->log_dir` |

- Black background / white monospace (Consolas 9pt) rich edit control
- Error log colour-coding: `EROR` → red, `WARN` → yellow, `INFO` → cyan, `DBUG` → grey
- Real-time tail via `WM_TIMER` (2-second poll)
- Uses `_fsopen(path, "rb", _SH_DENYNO)` — reads files while the server has them open for writing
- Path resolved from `config_get_config_unlocked()` after server starts
- Log rotation detection: resets `m_fileOffset` when file shrinks
- `Mcaster1Win.vcxproj` updated: `LogTab.cpp` + `LogTab.h` added

#### Uptime Clock
- New `IDC_UPTIME` (`CStatic`) added to `IDD_MCASTER1WIN_DIALOG` RC and `resource.h`
- Positioned below the Server Status label/bitmap in the header strip
- Updates every 500 ms (reuses timer 1) while server is running
- Format: `"Up: HH:MM:SS"` / `"Stopped"` when halted
- Anchored `TOP_RIGHT → TOP_RIGHT` via ResizableLib

#### System Time Status Bar
- New `IDC_SYSCLOCK` (`CStatic`) added to `IDD_MCASTER1WIN_DIALOG` RC and `resource.h`
- Positioned at bottom of dialog below the tab control (full width)
- Dedicated timer 3 fires every 1 second via `SetTimer(3, 1000)` in `OnInitDialog`
- Timer 3 killed in `OnDestroy`
- Format: `"System Time:  Sat Feb 22 2026   10:30:45 AM"`
- Anchored `BOTTOM_LEFT → BOTTOM_RIGHT` via ResizableLib

#### Portable Windows Dev Config (`windows/mcaster1dnas.yaml`)
Updated to use relative paths (`"."` as basedir) and full ICY 2.2 mount metadata:
- Paths resolve relative to exe working directory (no hard-coded user paths)
- Per-listener `ssl: true/false` flags applied
- Example mounts for live broadcast, podcast, and social media types
- All ICY 2.2 metadata fields documented per mount

---

### Bug Fixes

#### ResizableLib Anchoring
- `IDC_STATICBLACK` (banner bitmap): renamed from generic `IDC_STATIC` to give it a unique
  ID addressable by `AddAnchor()`; all header-strip controls now correctly pin on maximize
- `IDC_STATIC_SLS` ("Click source to view statistics" label in Source Level Stats tab):
  renamed from `IDC_STATIC` to `IDC_STATIC_SLS`; added `AddAnchor(IDC_STATIC_SLS, BOTTOM_LEFT, BOTTOM_LEFT)` in `CStatsTab::OnInitDialog` — label now stays at the bottom of the tab on resize
- `IDC_UPTIME`, `IDC_SYSCLOCK` anchors: new controls registered with ResizableLib

#### About → Help
- `OnAboutHelp()` now calls `ShellExecuteA` to open `https://mcaster1.com/mcaster1dnas/` in
  the default browser instead of launching a CHM file that no longer existed

#### Tab Header Label Overlap
- Removed oversized "Source Level Statistics" label from `IDD_STATSDIALOG` (was rendered
  behind the tab strip; `CStatic` font still being applied at 10pt Bold causing visual noise)
- Removed "Global Statistics" label from `IDD_SSTATUS` for same reason
- List controls repositioned to `y=7` filling available height; hint text moved to `y=182`/`y=183`

---

### Build Verification

| Test | Result |
|------|--------|
| MSBuild x64 Debug `Mcaster1Win` — 0 errors | PASS |
| MSBuild x64 Debug `Mcaster1Console` — 0 errors | PASS |
| HTTP admin auth (browser login dialog appears) | PASS (after `mc_http_printf` fix) |
| Log tabs real-time tailing while server running | PASS |
| Uptime clock ticking | PASS |
| System time clock updating | PASS |
| Resize / Maximize — all controls anchored | PASS |

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
