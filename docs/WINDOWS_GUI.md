# Windows GUI User Guide — Mcaster1Win

**Application:** `mcaster1win.exe`
**Applies to:** Mcaster1DNAS v2.5.1-beta.2+ (windows-dev branch)
**Build:** Visual Studio 2022 (v17), MSVC v143, x64, Windows 10/11

---

## Overview

`mcaster1win.exe` is the native Windows GUI front-end for Mcaster1DNAS. It wraps the same
server core (`mcaster1.exe`) in an MFC dialog-based application with:

- Resizable main window (ResizableLib by Paolo Messina)
- Four tabbed panels: Status, Stats, Config, Log
- Server start/stop with auto-start option
- System tray minimize
- Real-time uptime clock and system clock
- Real-time log tailing for all log files
- Full command-line interface for scripted use
- SSL certificate generation (see [SSL_CERT_GENERATION.md](SSL_CERT_GENERATION.md))
- Config file auto-discovery with `GetOpenFileName` fallback dialog

---

## First Launch

On first launch, the application searches for a config file in this order:

1. `<exe directory>\mcaster1dnas.yaml`
2. `<exe directory>\mcaster1dnas.xml`
3. `.\mcaster1dnas.yaml` (current working directory)
4. `.\mcaster1dnas.xml`
5. If none found → **File Open dialog** (`GetOpenFileName`) prompts you to locate a
   `.yaml` or `.xml` config file

You can also pass the config path explicitly:

```
mcaster1win.exe -c C:\mcaster1\mcaster1dnas.yaml
```

A sample Windows config is included at `windows\mcaster1dnas.yaml` and at
`windows\x64\Debug\mcaster1dnas.yaml`. Edit it before first launch to set your passwords.

---

## Main Window Layout

```
┌─────────────────────────────────────────────────────────────────────┐
│ [mcaster1dnas title bar bitmap]                                     │
│ [Start Server] [Start on Startup checkbox]     [●] Server Status   │
│                                            [Stopped/Up HH:MM:SS]  │
│                                            [Hide To Systray]       │
├─────────────────────────────────────────────────────────────────────┤
│  Status │ Stats │ Config │ Log                                     │
│ ┌───────────────────────────────────────────────────────────────┐  │
│ │  (tab content)                                                │  │
│ └───────────────────────────────────────────────────────────────┘  │
│ System Time:  Sun Feb 22 2026   06:45:12 PM                        │
└─────────────────────────────────────────────────────────────────────┘
```

### Header Area

| Control | Description |
|---------|-------------|
| **Start Server** button | Starts (or stops) the Mcaster1DNAS server process |
| **Start Server on Startup** checkbox | If checked, server starts automatically when the app launches |
| **Server Status** bitmap | Green dot = running, red dot = stopped |
| **Uptime / Status text** | Shows `Stopped` when idle; `Up: HH:MM:SS` when running |
| **Hide To Systray** button | Minimizes the window to the system notification tray |

### Status Bar (Bottom)

The bottom status bar always shows the current local system time in the format:
```
System Time:  Mon Feb 22 2026   06:45:12 PM
```
Updates every second regardless of server state.

---

## Tab Panels

### Status Tab

Displays global server statistics as a list view with two columns (Statistic / Value).
Stats include: server uptime, total bytes sent, active listeners, active sources,
connected clients, and more.

The list auto-refreshes every 500ms while the server is running.

### Stats Tab

Shows per-source statistics with two list views:

- **Left (Sources):** All currently connected sources / mount points
- **Right (Statistics):** Statistics for the selected source

**Usage:**
- Click a source in the left list → statistics for that source appear on the right
- Double-click a source → refreshes the statistics list
- Right-click a statistic → context menu to add it to the global stats list

### Config Tab

Displays the raw contents of the loaded config file (YAML or XML) in a multi-line text
editor. You can edit the config directly and save; changes take effect on next server restart.

Access via the menu: **Configuration → Edit Configuration**

### Log Tab

Real-time log viewer with sub-tabs for each log file:
- **Access Log** (`access.log`) — HTTP request log, one line per request
- **Error Log** (`error.log`) — server events, warnings, and errors
- **Playlist Log** (`playlist.log`) — track changes and metadata events

Logs are polled every 500ms using `_fsopen` with `_SH_DENYNO` (shared read mode) so the
viewer works even while the server process holds the files open for writing.

**Color coding (Error Log):**
- Red text — `ERROR` and `WARN` level messages
- Yellow/amber — `INFO` level messages
- Grey — `DEBUG` level messages
- White — unclassified messages

---

## Command-Line Flags

```
mcaster1win.exe [options]

  -c <file>        Use specified config file (YAML or XML, auto-detected)
  -s               Auto-start the server on launch
  -m               Start minimized to the system tray
  -v               Print version and exit
  -h               Print help text and exit

SSL Generation (exits after generating, no GUI shown):
  --ssl-gencert                     Generate SSL certificate/CSR
  --ssl-gentype=selfsigned|csr      Output type (default: selfsigned)
  --subj="<subject>"                X.509 subject string
  --ssl-gencert-savepath=<dir>      Output directory
  --ssl-gencert-addtoconfig=true    Patch -c config file after generation
```

**Examples:**

```
# Launch with specific config, auto-start, minimized to tray
mcaster1win.exe -c "C:\mcaster1\mcaster1dnas.yaml" -s -m

# Generate self-signed cert and patch the config
mcaster1win.exe --ssl-gencert --ssl-gentype=selfsigned ^
    --subj="/C=US/ST=TX/O=My Station/CN=stream.example.com" ^
    --ssl-gencert-savepath="C:\mcaster1\ssl" ^
    --ssl-gencert-addtoconfig=true ^
    -c "C:\mcaster1\mcaster1dnas.yaml"

# Print version
mcaster1win.exe -v
```

---

## System Tray

Click **Hide To Systray** (or minimize the window) to hide the GUI to the Windows
notification area (system tray). The Mcaster1DNAS icon appears in the tray.

- **Left double-click** tray icon → restore window
- **Right-click** tray icon → context menu with **Restore** option

The server continues running in the background while the window is hidden.

---

## Menu Bar

| Menu | Item | Action |
|------|------|--------|
| **File** | Exit | Graceful shutdown — stops server and exits |
| **Configuration** | Edit Configuration | Opens config text in the Config tab |
| **About** | Help | Opens help information |
| **About** | Credits | Opens the credits/about dialog with version info |

---

## Window Resizing

The main window is fully resizable. All controls anchor correctly:

| Control | Anchor behavior |
|---------|-----------------|
| Tab control (main content area) | Stretches in all directions |
| Status, Stats, Config, Log (tab content) | Stretch with tab control |
| Start Server, Auto-start checkbox | Fixed top-left |
| Server Status bitmap + Uptime text | Fixed top-right |
| Hide To Systray button | Fixed top-right |
| System time status bar | Stretches full width at bottom |

Window size and position are saved to the Windows registry on exit and restored on
next launch.

---

## Auto-Version Stamping

The window title bar shows the build version at compile time:
```
Mcaster1DNAS v2.5.1-dev.b73b861 — windows-dev
```

The commit hash and branch name are embedded by a pre-build PowerShell script that
reads `git describe --tags --long --always`. This happens automatically on every build.

---

## Building mcaster1win.exe

### Prerequisites

1. **Visual Studio 2022** (Professional or Community) with:
   - "Desktop development with C++" workload
   - MSVC v143 toolset
   - Windows 10/11 SDK

2. **vcpkg** with x64-windows packages:
   ```
   vcpkg install libxml2 libxslt curl openssl libyaml ogg vorbis theora speex pthreads
   ```

3. Set `VCPKG_ROOT` environment variable to your vcpkg install path.

### Build

```powershell
# From Developer PowerShell or cmd
cd C:\Users\dstjohn\dev\00_mcaster1.com\mcaster1dnas\windows

# Build just mcaster1win.exe (Debug x64)
& "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" `
    Mcaster1Win.vcxproj /p:Configuration=Debug /p:Platform=x64 /t:Build /v:minimal

# Build all three projects (win + console + service)
& "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" `
    Mcaster1DNAS.sln /p:Configuration=Debug /p:Platform=x64 /t:Build /v:minimal
```

Or open `windows\Mcaster1DNAS.sln` in Visual Studio 2022 and press **Ctrl+Shift+B**.

### Output

```
windows\x64\Debug\mcaster1win.exe
windows\x64\Debug\mcaster1.exe       (console server)
windows\x64\Debug\mcaster1svc.exe    (Windows Service — planned)
```

---

## Known Issues and Limitations (Beta)

| Issue | Status |
|-------|--------|
| Config dialog is read-only (no save-to-disk button) | Planned |
| Log tab does not auto-scroll to bottom on new lines | Known |
| Windows Service install/start not yet wired to GUI | Planned |
| Dark mode (Windows 10/11) | Planned |
| Podcast file manager | Planned |
| Statistics charts / graphs | Planned |

---

## See Also

- [SSL_CERT_GENERATION.md](SSL_CERT_GENERATION.md) — generate TLS certs from the CLI or GUI
- [CHANGELOG-WIN.md](../CHANGELOG-WIN.md) — full Windows build change history
- [ICY2_PROTOCOL.md](ICY2_PROTOCOL.md) — ICY2 metadata protocol used by the server
- [STATIC_MOUNTS.md](STATIC_MOUNTS.md) — podcast, socialcast, on-demand mount types
