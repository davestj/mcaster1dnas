# Claude Code Session Resume — Windows Dev Branch

**Branch:** `windows-dev`
**Base:** `development`
**Last Updated:** 2026-02-22
**Session ID (last):** 64a9bcd5-f537-4c9e-8c3f-a1bde6d7803e

---

## Quick Resume

```
claude --resume 64a9bcd5-f537-4c9e-8c3f-a1bde6d7803e
```

Or start a new session on the `windows-dev` branch. This file contains all context
needed to continue without needing the old session.

---

## Branch Purpose

`windows-dev` adds native Windows support to Mcaster1DNAS:

1. **`mcaster1win.exe`** — MFC dialog-based GUI wrapping the core server
2. **`mcaster1.exe`** — Windows console build (same core as Linux)
3. **`mcaster1svc.exe`** — Windows Service (stub, planned)
4. All three built from `windows\Mcaster1DNAS.sln` (VS2022, MSVC v143, x64)

---

## Current State (as of 2026-02-22)

### What Works (Debug x64 Build Passing)

| Feature | Status |
|---------|--------|
| Server starts and serves HTTP + HTTPS | Working |
| YAML config parsing (Windows CRT fix) | Working |
| XML config parsing | Working |
| Admin HTTP authentication (401 / WWW-Authenticate) | Fixed this session |
| ResizableLib — all four tabs resize correctly | Working |
| Log tabs — real-time tailing (_fsopen _SH_DENYNO) | Fixed this session |
| Uptime clock (Up: HH:MM:SS) | Added this session |
| System time status bar (always-on, 1s timer) | Added this session |
| Stats tab anchor — "Click source to view statistics" | Fixed this session |
| Auto-version stamping (git hash in title bar) | Working |
| System tray hide/restore | Working |
| YAML config auto-discovery + GetOpenFileName fallback | Planned (Item 4 of plan) |
| Per-listener ssl: flag enforcement | Planned (Item 1 of plan) |
| SSL cert generation CLI (--ssl-gencert flags) | Planned (Items 2+3 of plan) |
| src/ssl_gen.c + windows/SslGen.cpp | Planned (Item 2 of plan) |

### Critical Bug Fixed This Session

**`src/params.c` — MSVC struct initializer field order**

Both `mc_http_printf` and `mc_params_printf` had MSVC-specific blocks with:
```c
mc_param_t hdr = { NULL, (char*)name, content, flags };
```
The `mc_param_t` field order is `{next, flags, name_len, value_len, name, value, ...}` so
this positional initializer mapped `name→flags`, `content→name_len`, `flags→value_len`
leaving `name=NULL, value=NULL`. The `mc_http_apply()` function returns -1 when
`name==NULL && value==NULL`, silently dropping **every** header set via `mc_http_printf` on
Windows. This broke WWW-Authenticate (HTTP 401 auth), Content-Type, Location, and all
other headers built through this path.

**Fix applied:**
```c
mc_param_t hdr;
memset (&hdr, 0, sizeof hdr);
hdr.name  = (char*)name;
hdr.value = content;
hdr.flags = (uint32_t)flags;
```
Both instances in `src/params.c` were fixed identically.

---

## Build Commands

```powershell
# Build mcaster1win.exe (Debug x64)
& "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" `
    "C:\Users\dstjohn\dev\00_mcaster1.com\mcaster1dnas\windows\Mcaster1Win.vcxproj" `
    /p:Configuration=Debug /p:Platform=x64 /t:Build /v:minimal

# Build mcaster1.exe (console, Debug x64)
& "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" `
    "C:\Users\dstjohn\dev\00_mcaster1.com\mcaster1dnas\windows\Mcaster1Console.vcxproj" `
    /p:Configuration=Debug /p:Platform=x64 /t:Build /v:minimal

# Build all three projects
& "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" `
    "C:\Users\dstjohn\dev\00_mcaster1.com\mcaster1dnas\windows\Mcaster1DNAS.sln" `
    /p:Configuration=Debug /p:Platform=x64 /t:Build /v:minimal
```

**Output directory:** `windows\x64\Debug\`

---

## File Index — What Was Changed / Created (This Session + Previous Sessions)

### This Session (2026-02-22)

| File | Change |
|------|--------|
| `src/params.c` | **CRITICAL BUG FIX** — mc_http_printf + mc_params_printf MSVC struct initializer; all HTTP headers were silently dropped on Windows |
| `windows/LogTab.cpp` | Fixed real-time log tailing: replaced `fopen_s` with `_fsopen(path, "rb", _SH_DENYNO)` |
| `windows/resource.h` | Added `IDC_UPTIME 1038`, `IDC_SYSCLOCK 1039`; updated next value to 1040 |
| `windows/Mcaster1Win.rc` | Added `IDC_UPTIME` and `IDC_SYSCLOCK` controls; changed `IDC_STATIC` → `IDC_STATIC_SLS` for "Click source to view statistics" |
| `windows/Mcaster1WinDlg.cpp` | Added timer 3 (1s, system clock); uptime display in timer 1; anchors for IDC_UPTIME + IDC_SYSCLOCK + IDC_STATIC_SLS; KillTimer(3) in OnDestroy |
| `windows/StatsTab.cpp` | Added `AddAnchor(IDC_STATIC_SLS, BOTTOM_LEFT, BOTTOM_LEFT)` |
| `CHANGELOG-WIN.md` | Prepended `[2.5.1-beta.2-win] — 2026-02-22` section |
| `docs/SSL_CERT_GENERATION.md` | New — SSL cert generation guide (all platforms) |
| `docs/ICY2_PROTOCOL.md` | New — ICY2 protocol implementation guide |
| `docs/STATIC_MOUNTS.md` | New — podcast, socialcast, on-demand mount type guide |
| `docs/SONG_HISTORY_API.md` | New — song history ring buffer and XML API |
| `docs/WINDOWS_GUI.md` | New — Windows GUI user guide |
| `CLAUDE-RESUME-WINDOWS-DEV.md` | New — this file |
| `README.md` | Updated badges (ICY-META v2.2), Windows features section, docs index |

### Previous Sessions (windows-dev branch history)

| File | Change |
|------|--------|
| `windows/ResizableLib/` | Added ResizableLib by Paolo Messina (Artistic License 2.0) |
| `windows/CTabPageSSL.h/.cpp` | Tab page base class (ResizableLib integration) |
| `windows/LogTab.h/.cpp` | New — real-time log viewer tab (CRichEditCtrl, color-coded) |
| `windows/Mcaster1WinDlg.cpp` | Full rewrite: ResizableLib, 4-tab panel, start/stop, version stamping |
| `windows/Mcaster1Win.cpp` | ParseMcasterCmdLine (-c -s -m -v -h flags), config discovery |
| `windows/Mcaster1Win.vcxproj` | Added all new source files, vcpkg includes, disabled warnings |
| `windows/Mcaster1Console.vcxproj` | Console build: vcpkg includes, all src/ files |
| `windows/Mcaster1Service.vcxproj` | Service stub project |
| `windows/prebuild.ps1` | Git version stamping pre-build script |
| `windows/config.h` | Windows-specific defines (_WIN32_WINNT, HAVE_STRUCT_TIMESPEC guards) |
| `windows/mcaster1_subsys.c` | Windows subsystem stub |
| `windows/mcaster1dnas.yaml` | Windows sample config (portable relative paths) |
| `src/main.c` | Windows-compatible cmd-line parsing, abort/signal handlers |
| `src/compat.h` | Windows compat: FD_t, access() → _access() guards |

---

## Next Planned Items (from Implementation Plan)

See `C:\Users\dstjohn\.claude\plans\sorted-snuggling-stearns.md` for the full plan.

### Item 1 — Per-listener SSL flag (strict enforcement)
- Add `int ssl` to `_listener_t` in `src/cfgfile.h`
- Parse in `src/cfgfile.c` (XML) and `src/cfgfile_yaml.c` (YAML)
- Add `int *ssl_on_sock` array in `src/global.h` / `src/global.c`
- Add `int ssl_required` to `connection_t` in `src/connection.h`
- Enforce in `connection_peek()` in `src/connection.c`

### Item 2+3 — SSL cert generation core + CLI
- Create `src/ssl_gen.h` + `src/ssl_gen.c` (cross-platform, `#ifdef HAVE_OPENSSL`)
- Create `windows/SslGen.h` + `windows/SslGen.cpp` (C++ wrapper for WinUI)
- Add `--ssl-gencert` flags in `src/main.c` and `windows/Mcaster1Win.cpp`

### Item 4 — Config file auto-discovery
- `get_exe_dir()` helper in `src/main.c` (platform-specific)
- Console: stdin prompt when no config found
- WinUI: `GetOpenFileName` dialog when no config found

### Item 5 — windows/mcaster1dnas.yaml (portable)
- Replace hard-coded paths with relative `../../web`, `../../admin`, `../../ssl` paths

### Item 6 — Test SSL files in ssl/temp/
- Run vcpkg openssl.exe to generate `selfsigned.{key,crt,pem}` + `test.{key,csr}`

---

## Testing Checklist

```powershell
# 1. Clean build
& msbuild windows\Mcaster1DNAS.sln /p:Configuration=Debug /p:Platform=x64 /t:Build /v:minimal

# 2. Start server, verify HTTP (should return HTML page)
curl http://127.0.0.1:9330/

# 3. Verify HTTPS
curl -sk https://127.0.0.1:9443/

# 4. Verify admin auth (should return 401 with WWW-Authenticate header)
curl -sv http://127.0.0.1:9330/admin/stats
# Expected: HTTP/1.1 401 Authorization Required
# Expected header: WWW-Authenticate: Basic realm="Mcaster1 Server"

# 5. Verify admin auth with credentials
curl -sk -u admin:changeme https://127.0.0.1:9443/admin/stats.xsl

# 6. Verify song history API
curl http://127.0.0.1:9330/mcaster1songdata

# 7. Test GUI: start mcaster1win.exe, verify uptime clock increments, system clock updates
windows\x64\Debug\mcaster1win.exe -c windows\mcaster1dnas.yaml
```

---

## Key Config Files

| File | Purpose |
|------|---------|
| `windows\mcaster1dnas.yaml` | Windows development config (used with Debug build) |
| `windows\x64\Debug\mcaster1dnas.yaml` | Copy deployed to build output directory |
| `windows\Mcaster1DNAS.sln` | Visual Studio solution |
| `windows\Mcaster1Win.vcxproj` | WinUI project |
| `windows\Mcaster1Console.vcxproj` | Console project |
| `windows\resource.h` | MFC resource IDs (IDC_*, IDB_*, IDR_*, IDS_*) |
| `windows\Mcaster1Win.rc` | Dialog and resource definitions |

---

## Known Issues / Gotchas

1. **MSVC struct initializer** — MSVC does not support GNU designated initializers
   (`{ .name = val }`). Always use `memset + field assignment` for structs with non-trivial
   field order in `#ifdef _MSC_VER` blocks.

2. **Windows file locking** — `fopen_s` fails on files held open by another process.
   Always use `_fsopen(path, mode, _SH_DENYNO)` for reading log files.

3. **CRT DLL boundary (libyaml)** — `yaml_parser_set_input_file()` crosses CRT boundaries.
   Always use `yaml_parser_set_input_string()` with in-memory buffer read by the exe's own CRT.

4. **ResizableLib + IDC_STATIC** — Controls using `IDC_STATIC` (0xFFFF) cannot be anchored.
   Give every anchored control a unique numeric ID in `resource.h`.

5. **MSBuild path** — VS2022 Professional is at:
   `C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe`
   (not Community; adjust if using Community edition).

6. **vcpkg openssl** location for manual testing:
   `%USERPROFILE%\dev\vcpkg\installed\x64-windows\tools\openssl\openssl.exe`

---

## Contacts / References

- **Maintainer:** David St John (Saint John) — davestj@gmail.com
- **Plan file:** `C:\Users\dstjohn\.claude\plans\sorted-snuggling-stearns.md`
- **Full session transcript:** `C:\Users\dstjohn\.claude\projects\C--Users-dstjohn-dev-00-mcaster1-com-mcaster1dnas\64a9bcd5-f537-4c9e-8c3f-a1bde6d7803e.jsonl`
