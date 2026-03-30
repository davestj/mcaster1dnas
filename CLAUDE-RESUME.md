# Claude Code Session Resume

## Resume Command

```
claude --resume 84e4715b-b52b-4680-ad8e-cda1970bc696
```

## Session Summary (2026-02-19 — continued)

### Confirmed This Session
1. **Crash is `abort()`** — `mcaster1win_start.log` showed `main()` was entered but never returned, and `__except` was NOT triggered → `abort()` in server code (not a structured exception)
2. **Server was running at 13:52** — `access.log` has entries serving both HTTP (9300) and HTTPS (9443) — server does start and serve; crash may be after startup or during shutdown
3. **`HideApplication()` is a real MFC function** — `CWinApp::HideApplication()` exists in MFC; no issue there

### Fixes Applied This Session

| Fix | Description |
|-----|-------------|
| `src/main.c` | Added step-by-step file traces `[1]...[6]` at each phase of `main()` so next run shows exactly where `abort()` fires |
| `windows/Mcaster1WinDlg.cpp` | Added `signal(SIGABRT, mcaster1_sigabrt_handler)` in `StartServer` to log abort before process dies |
| `windows/Mcaster1WinDlg.cpp` | Fixed `OnTrayNotify` WM_LBUTTONDBLCLK restore: sets `m_bHidden=FALSE`, calls `ShowWindow(SW_SHOW)`, forces `OnSize` layout |
| `windows/Mcaster1WinDlg.cpp` | Fixed `OnBlankRestore`: same pattern — resets `m_bHidden`, shows window, forces `OnSize` |
| `windows/Mcaster1WinDlg.cpp` | Fixed `OnSize`: guards against `SIZE_MINIMIZED`, zero cx/cy, and negative list dimensions |

### Pending Tasks
- **Diagnose Start Server crash** — After clicking Start Server, check `windows\x64\Debug\mcaster1win_start.log` for the highest `[N]` marker to see where `abort()` fires. The markers are:
  - `[1]` = entering initialize_subsystems
  - `[2]` = entering server_init (subsystems OK)
  - `[2b]` = server_init failed
  - `[3]` = server_process starting (server_init OK)
  - `[4]` = server_process done (server ran and stopped)
  - `[5]` = entering shutdown_subsystems
  - `[6]` = main done cleanly (no crash)
  - `[SIGABRT]` = abort() was called, logged just before crash
- **Fix GUI logging** — access/error log panels in the Status tab not displaying file content
- **Hide To Systray** — button clickable; check tray icon and restore behavior after fix
- **Graceful shutdown** — signal `global.running = MC_HALTING` and join server thread on dialog close
- **ResizableLib** — long-term: derive all MFC dialogs from resizable base class

### Key Files Modified
| File | Change |
|------|--------|
| `src/main.c` | Step-by-step file traces in `main()` + `_fatal_error` fixed (MB_OK\|MB_ICONERROR) |
| `windows/Mcaster1WinDlg.cpp` | Crash diagnostics, hide/restore fix, OnSize fix |
| `windows/Mcaster1WinDlg.h` | `OnBnClickedAutostart` declaration |
| `windows/Mcaster1Win.cpp` | Removed double-init; default config `.\mcaster1.yaml` |
| `windows/Mcaster1Win.vcxproj` | Added `..` to includes, disabled warnings |
| `windows/config.h` | Guarded `_WIN32_WINNT` and `HAVE_STRUCT_TIMESPEC` |
| `windows/mcaster1_subsys.c` | Added `#include "logging.h"` |
| `src/compat.h` | Added `#ifndef FD_t / #define FD_t int` |

### Build Command
```powershell
powershell -Command "& 'C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe' 'C:\Users\dstjohn\dev\00_mcaster1.com\mcaster1dnas\windows\Mcaster1Win.vcxproj' /p:Configuration=Debug /p:Platform=x64 /t:Build /v:minimal"
```

### How to diagnose after clicking Start Server
1. Click Start Server in the GUI
2. Wait a few seconds (or until crash)
3. Open `windows\x64\Debug\mcaster1win_start.log`
4. The highest `[N]` marker before `[SIGABRT]` or before the file ends shows which phase crashed
