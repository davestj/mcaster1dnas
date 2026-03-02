; =============================================================================
; Mcaster1DNAS NSIS Installer Script
; =============================================================================
; Build with: makensis.exe windows\installer\mcaster1dnas.nsi
;             OR use windows\installer\build-installer.ps1
;
; Requires NSIS 3.x (https://nsis.sourceforge.io)
; NSIS plugins used: MUI2 (built-in), AdvSplash (built-in Contrib)
;
; Image files expected in the same directory as this .nsi:
;   splash.bmp   - 600x400  splash screen
;   sidebar.bmp  - 164x314  welcome/finish page left panel
;   header.bmp   - 150x57   inner-page header banner
; Run prepare-images.ps1 first to generate these from the PNG sources.
; =============================================================================

Unicode True

; -----------------------------------------------------------------------------
; Product constants
; -----------------------------------------------------------------------------
!define PRODUCT_NAME        "Mcaster1DNAS"
!define PRODUCT_VERSION     "2.5.3-beta"
!define PRODUCT_PUBLISHER   "MediaCast1"
!define PRODUCT_URL         "https://mcaster1.com"
!define PRODUCT_SUPPORT_URL "https://github.com/davestj/mcaster1dnas/issues"
!define PRODUCT_REG_KEY     "Software\Mcaster1\Mcaster1DNAS"
!define PRODUCT_UNINST_KEY  "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define SERVICE_NAME        "Mcaster1DNAS"
!define SERVICE_DISPLAY     "Mcaster1DNAS Streaming Server"
!define SERVICE_DESC        "Mcaster1DNAS Digital Network Audio Server - Professional streaming media server platform (mcaster1.com)"
!define HTTP_PORT           "9330"
!define HTTPS_PORT          "9443"
!define GUI_HTTP_PORT       "9033"
!define GUI_HTTPS_PORT      "9344"

; Source paths relative to this .nsi file location (windows\installer\)
!define BUILD_DIR   "..\x64\Debug"
!define REPO_ROOT   "..\.."

; Default install path: C:\Program Files (x86)\Mcaster1\Mcaster1DNAS
InstallDir "$PROGRAMFILES\Mcaster1\Mcaster1DNAS"
InstallDirRegKey HKLM "${PRODUCT_REG_KEY}" "InstallPath"

; -----------------------------------------------------------------------------
; Installer attributes
; -----------------------------------------------------------------------------
Name                "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile             "mcaster1dnas_win64_v${PRODUCT_VERSION}_setup.exe"
RequestExecutionLevel admin
SetCompressor       /SOLID lzma
SetCompressorDictSize 32
BrandingText        "Mcaster1DNAS ${PRODUCT_VERSION} | mcaster1.com"
ShowInstDetails     show
ShowUnInstDetails   show

; -----------------------------------------------------------------------------
; MUI2 Modern UI
; -----------------------------------------------------------------------------
!include "MUI2.nsh"
!include "Sections.nsh"
!include "LogicLib.nsh"
!include "WinMessages.nsh"

; MUI images
!define MUI_ICON                        "${REPO_ROOT}\windows\mcaster1.ico"
!define MUI_UNICON                      "${REPO_ROOT}\windows\mcaster1.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP    "sidebar.bmp"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP          "header.bmp"
!define MUI_HEADERIMAGE_RIGHT

; MUI behaviour
!define MUI_ABORTWARNING
!define MUI_UNABORTWARNING
!define MUI_FINISHPAGE_RUN              "$INSTDIR\mcaster1win.exe"
!define MUI_FINISHPAGE_RUN_TEXT         "Launch Mcaster1DNAS GUI now"
!define MUI_FINISHPAGE_LINK             "Visit mcaster1.com"
!define MUI_FINISHPAGE_LINK_LOCATION    "${PRODUCT_URL}"

; -----------------------------------------------------------------------------
; Installer pages
; -----------------------------------------------------------------------------
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE           "${REPO_ROOT}\LICENSE.md"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

; -----------------------------------------------------------------------------
; Version info embedded in setup.exe
; -----------------------------------------------------------------------------
VIProductVersion                "2.5.3.0"
VIAddVersionKey "ProductName"   "${PRODUCT_NAME}"
VIAddVersionKey "ProductVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "FileVersion"   "2.5.3.0"
VIAddVersionKey "FileDescription" "${PRODUCT_NAME} ${PRODUCT_VERSION} Installer"
VIAddVersionKey "LegalCopyright" "Copyright (C) 2001-2026 MediaCast1"
VIAddVersionKey "CompanyName"   "${PRODUCT_PUBLISHER}"

; =============================================================================
; SPLASH SCREEN (.onInit runs before the installer GUI appears)
; =============================================================================
Function .onInit
    ; Show splash for 3 seconds (3000ms), 600ms fade-in, 400ms fade-out
    ; AdvSplash looks for splash.bmp in the same directory as the .exe
    SetOutPath $TEMP
    File "splash.bmp"
    advsplash::show 3000 600 400 -1 "$TEMP\splash"
    Pop $0   ; $0 = 1 (user clicked), 0 (timed out), -1 (cancelled)
    Delete "$TEMP\splash.bmp"
FunctionEnd

; =============================================================================
; SECTIONS
; =============================================================================

; ----------------------------------------------------------------------------
; Section 1 — Core Application (required)
; ----------------------------------------------------------------------------
Section "!Mcaster1DNAS Core Application" SEC_CORE
    SectionIn RO   ; Read-only — cannot be deselected

    SetOutPath "$INSTDIR"
    SetOverwrite on

    ; --- Main executables ---
    File "${BUILD_DIR}\mcaster1win.exe"
    File "${BUILD_DIR}\mcaster1.exe"
    File /nonfatal "${BUILD_DIR}\mcaster1Service.exe"
    File "${REPO_ROOT}\windows\mcaster1.ico"

    ; --- Configuration files ---
    File /oname=mcaster1dnas.yaml        "${REPO_ROOT}\windows\mcaster1dnas.yaml"
    File /oname=mcaster1dnas-console.yaml "${REPO_ROOT}\mcaster1dnas-console.yaml"

    ; --- vcpkg runtime DLLs (OpenSSL, libxml2, libxslt, curl, codecs, etc.) ---
    File "${BUILD_DIR}\libcrypto-3-x64.dll"
    File "${BUILD_DIR}\libssl-3-x64.dll"
    File "${BUILD_DIR}\libxml2.dll"
    File "${BUILD_DIR}\libxslt.dll"
    File "${BUILD_DIR}\libexslt.dll"
    File "${BUILD_DIR}\libcurl.dll"
    File /nonfatal "${BUILD_DIR}\libcurl-d.dll"
    File "${BUILD_DIR}\zlib1.dll"
    File "${BUILD_DIR}\iconv-2.dll"
    File /nonfatal "${BUILD_DIR}\charset-1.dll"
    File /nonfatal "${BUILD_DIR}\legacy.dll"
    File "${BUILD_DIR}\ogg.dll"
    File "${BUILD_DIR}\vorbis.dll"
    File "${BUILD_DIR}\vorbisenc.dll"
    File "${BUILD_DIR}\vorbisfile.dll"
    File "${BUILD_DIR}\theora.dll"
    File /nonfatal "${BUILD_DIR}\theoradec.dll"
    File /nonfatal "${BUILD_DIR}\theoraenc.dll"
    File "${BUILD_DIR}\speex-1.dll"
    File "${BUILD_DIR}\yaml.dll"
    File "${BUILD_DIR}\pthreadVSE3.dll"
    File /nonfatal "${BUILD_DIR}\pthreadVC3.dll"
    File /nonfatal "${BUILD_DIR}\pthreadVCE3.dll"
    File /nonfatal "${BUILD_DIR}\pthreadVSE3d.dll"

    ; --- SSL certificate (localhost self-signed, key+cert combined PEM) ---
    SetOutPath "$INSTDIR\ssl"
    File "${REPO_ROOT}\ssl\localhost.pem"
    SetOutPath "$INSTDIR"

    ; --- Create required directories ---
    CreateDirectory "$INSTDIR\logs"
    CreateDirectory "$INSTDIR\ssl\temp"

    ; --- Write registry ---
    WriteRegStr   HKLM "${PRODUCT_REG_KEY}" "InstallPath"      "$INSTDIR"
    WriteRegStr   HKLM "${PRODUCT_REG_KEY}" "Version"          "${PRODUCT_VERSION}"
    WriteRegStr   HKLM "${PRODUCT_REG_KEY}" "ConsoleConfig"    "$INSTDIR\mcaster1dnas-console.yaml"
    WriteRegStr   HKLM "${PRODUCT_REG_KEY}" "GUIConfig"        "$INSTDIR\mcaster1dnas.yaml"
    WriteRegDWORD HKLM "${PRODUCT_REG_KEY}" "HTTPPort"         ${HTTP_PORT}
    WriteRegDWORD HKLM "${PRODUCT_REG_KEY}" "HTTPSPort"        ${HTTPS_PORT}
    WriteRegDWORD HKLM "${PRODUCT_REG_KEY}" "GUIHTTPPort"      ${GUI_HTTP_PORT}
    WriteRegDWORD HKLM "${PRODUCT_REG_KEY}" "GUIHTTPSPort"     ${GUI_HTTPS_PORT}

    ; --- Add/Remove Programs entry ---
    WriteRegStr   HKLM "${PRODUCT_UNINST_KEY}" "DisplayName"          "${PRODUCT_NAME} ${PRODUCT_VERSION}"
    WriteRegStr   HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion"       "${PRODUCT_VERSION}"
    WriteRegStr   HKLM "${PRODUCT_UNINST_KEY}" "Publisher"            "${PRODUCT_PUBLISHER}"
    WriteRegStr   HKLM "${PRODUCT_UNINST_KEY}" "DisplayIcon"          "$INSTDIR\mcaster1win.exe"
    WriteRegStr   HKLM "${PRODUCT_UNINST_KEY}" "InstallLocation"      "$INSTDIR"
    WriteRegStr   HKLM "${PRODUCT_UNINST_KEY}" "UninstallString"      "$INSTDIR\Uninstall.exe"
    WriteRegStr   HKLM "${PRODUCT_UNINST_KEY}" "URLInfoAbout"         "${PRODUCT_URL}"
    WriteRegStr   HKLM "${PRODUCT_UNINST_KEY}" "URLUpdateInfo"        "${PRODUCT_URL}"
    WriteRegStr   HKLM "${PRODUCT_UNINST_KEY}" "HelpLink"             "${PRODUCT_SUPPORT_URL}"
    WriteRegDWORD HKLM "${PRODUCT_UNINST_KEY}" "NoModify"             1
    WriteRegDWORD HKLM "${PRODUCT_UNINST_KEY}" "NoRepair"             1

    ; --- Firewall rules for service ports (9330/9443) and GUI ports (9033/9344) ---
    DetailPrint "Adding Windows Firewall rules..."
    nsExec::ExecToLog 'netsh advfirewall firewall delete rule name="Mcaster1DNAS HTTP"'
    nsExec::ExecToLog 'netsh advfirewall firewall delete rule name="Mcaster1DNAS HTTPS"'
    nsExec::ExecToLog 'netsh advfirewall firewall delete rule name="Mcaster1DNAS GUI HTTP"'
    nsExec::ExecToLog 'netsh advfirewall firewall delete rule name="Mcaster1DNAS GUI HTTPS"'
    nsExec::ExecToLog 'netsh advfirewall firewall add rule name="Mcaster1DNAS HTTP" protocol=TCP dir=in localport=${HTTP_PORT} action=allow description="Mcaster1DNAS service HTTP port"'
    nsExec::ExecToLog 'netsh advfirewall firewall add rule name="Mcaster1DNAS HTTPS" protocol=TCP dir=in localport=${HTTPS_PORT} action=allow description="Mcaster1DNAS service HTTPS port"'
    nsExec::ExecToLog 'netsh advfirewall firewall add rule name="Mcaster1DNAS GUI HTTP" protocol=TCP dir=in localport=${GUI_HTTP_PORT} action=allow description="Mcaster1DNAS GUI HTTP port"'
    nsExec::ExecToLog 'netsh advfirewall firewall add rule name="Mcaster1DNAS GUI HTTPS" protocol=TCP dir=in localport=${GUI_HTTPS_PORT} action=allow description="Mcaster1DNAS GUI HTTPS port"'

    ; --- Write uninstaller ---
    WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

; ----------------------------------------------------------------------------
; Section 2 — Web Content (web/ + admin/)
; ----------------------------------------------------------------------------
Section "Web Interface & Admin UI" SEC_WEB
    SetOutPath "$INSTDIR\web"
    File /r "${REPO_ROOT}\web\*.*"

    SetOutPath "$INSTDIR\admin"
    File /r "${REPO_ROOT}\admin\*.*"
SectionEnd

; ----------------------------------------------------------------------------
; Section 3 — Local Documentation
; ----------------------------------------------------------------------------
Section "Local Documentation" SEC_DOCS
    SetOutPath "$INSTDIR\docs"
    File /r "${REPO_ROOT}\docs\*.*"
    ; Also include key root-level docs
    SetOutPath "$INSTDIR\docs"
    File /oname=README.md              "${REPO_ROOT}\README.md"
    File /oname=CHANGELOG-WIN.md       "${REPO_ROOT}\CHANGELOG-WIN.md"
    File /oname=ICY2_PROTOCOL_SPEC.md  "${REPO_ROOT}\ICY2_PROTOCOL_SPEC.md"
    File /oname=YAML_IMPLEMENTATION.md "${REPO_ROOT}\YAML_IMPLEMENTATION.md"
SectionEnd

; ----------------------------------------------------------------------------
; Section 4 — SSL Test Certificates (optional)
; ----------------------------------------------------------------------------
Section /o "SSL Test Certificates (localhost)" SEC_SSL
    SetOutPath "$INSTDIR\ssl\temp"
    File /nonfatal /r "${REPO_ROOT}\ssl\temp\*.*"
SectionEnd

; ----------------------------------------------------------------------------
; Section 5 — Windows Service (optional)
; Installs mcaster1Service.exe as a Windows Service using sc.exe
; ----------------------------------------------------------------------------
Section /o "Install as Windows Service (auto-start)" SEC_SERVICE
    DetailPrint "Stopping any existing ${SERVICE_NAME} service..."
    nsExec::ExecToLog 'sc stop "${SERVICE_NAME}"'
    nsExec::ExecToLog 'sc delete "${SERVICE_NAME}"'
    Sleep 1500

    DetailPrint "Creating ${SERVICE_NAME} Windows Service..."
    nsExec::ExecToStack 'sc create "${SERVICE_NAME}" binPath= "\"$INSTDIR\mcaster1Service.exe\" \"$INSTDIR\"" DisplayName= "${SERVICE_DISPLAY}" start= auto obj= LocalSystem'
    Pop $0
    ${If} $0 != 0
        MessageBox MB_ICONEXCLAMATION "Service creation returned code $0. The service may already exist or sc.exe failed.$\n$\nYou can install the service manually:$\n$\nsc create ${SERVICE_NAME} binPath= $\"$INSTDIR\mcaster1.exe -c $INSTDIR\mcaster1dnas-console.yaml$\" DisplayName= $\"${SERVICE_DISPLAY}$\" start= auto"
    ${Else}
        nsExec::ExecToLog 'sc description "${SERVICE_NAME}" "${SERVICE_DESC}"'
        nsExec::ExecToLog 'sc failure "${SERVICE_NAME}" reset= 86400 actions= restart/5000/restart/10000/restart/30000'
        WriteRegStr HKLM "${PRODUCT_REG_KEY}" "ServiceInstalled" "1"
        DetailPrint "Service installed successfully. Use Services.msc or 'sc start ${SERVICE_NAME}' to start."
    ${EndIf}
SectionEnd

; ----------------------------------------------------------------------------
; Section 6 — Start Menu Shortcuts
; Placed under Start Menu > Mcaster1 > Mcaster1DNAS
; ----------------------------------------------------------------------------
Section "Start Menu Shortcuts" SEC_STARTMENU
    CreateDirectory "$SMPROGRAMS\Mcaster1"
    CreateDirectory "$SMPROGRAMS\Mcaster1\${PRODUCT_NAME}"

    ; --- Launch applications ---
    CreateShortCut "$SMPROGRAMS\Mcaster1\${PRODUCT_NAME}\Mcaster1DNAS Server (GUI).lnk" \
        "$INSTDIR\mcaster1win.exe" "" "$INSTDIR\mcaster1.ico" 0 \
        SW_SHOWNORMAL "" "Launch Mcaster1DNAS GUI streaming server (ports ${GUI_HTTP_PORT}/${GUI_HTTPS_PORT})"

    CreateShortCut "$SMPROGRAMS\Mcaster1\${PRODUCT_NAME}\Mcaster1DNAS Console.lnk" \
        "$INSTDIR\mcaster1.exe" "-c $\"$INSTDIR\mcaster1dnas-console.yaml$\"" "$INSTDIR\mcaster1.ico" 0 \
        SW_SHOWMINIMIZED "" "Launch Mcaster1DNAS console server (ports ${HTTP_PORT}/${HTTPS_PORT})"

    ; --- Service control (UAC elevation via PowerShell) ---
    CreateShortCut "$SMPROGRAMS\Mcaster1\${PRODUCT_NAME}\Start DNAS Service.lnk" \
        "powershell.exe" \
        "-NoProfile -NonInteractive -WindowStyle Hidden -Command $\"Start-Process sc.exe -ArgumentList 'start ${SERVICE_NAME}' -Verb RunAs$\"" \
        "$INSTDIR\mcaster1.ico" 0 SW_SHOWMINIMIZED "" "Start the Mcaster1DNAS Windows Service (requires admin)"

    CreateShortCut "$SMPROGRAMS\Mcaster1\${PRODUCT_NAME}\Stop DNAS Service.lnk" \
        "powershell.exe" \
        "-NoProfile -NonInteractive -WindowStyle Hidden -Command $\"Start-Process sc.exe -ArgumentList 'stop ${SERVICE_NAME}' -Verb RunAs$\"" \
        "$INSTDIR\mcaster1.ico" 0 SW_SHOWMINIMIZED "" "Stop the Mcaster1DNAS Windows Service (requires admin)"

    ; --- Config editors ---
    CreateShortCut "$SMPROGRAMS\Mcaster1\${PRODUCT_NAME}\Edit GUI Config.lnk" \
        "notepad.exe" "$INSTDIR\mcaster1dnas.yaml" "" 0 \
        SW_SHOWNORMAL "" "Edit the GUI server configuration file"

    CreateShortCut "$SMPROGRAMS\Mcaster1\${PRODUCT_NAME}\Edit Console Config.lnk" \
        "notepad.exe" "$INSTDIR\mcaster1dnas-console.yaml" "" 0 \
        SW_SHOWNORMAL "" "Edit the console/service configuration file"

    ; --- Utilities ---
    CreateShortCut "$SMPROGRAMS\Mcaster1\${PRODUCT_NAME}\Open Logs Folder.lnk" \
        "$INSTDIR\logs" "" "" 0 \
        SW_SHOWNORMAL "" "Open the Mcaster1DNAS logs folder"

    ; --- Documentation (opens index.html in default browser) ---
    WriteIniStr "$SMPROGRAMS\Mcaster1\${PRODUCT_NAME}\Documentation.url" \
        "InternetShortcut" "URL" "file:///$INSTDIR\docs\index.html"

    WriteIniStr "$SMPROGRAMS\Mcaster1\${PRODUCT_NAME}\Visit mcaster1.com.url" \
        "InternetShortcut" "URL" "${PRODUCT_URL}"

    ; --- Uninstaller ---
    CreateShortCut "$SMPROGRAMS\Mcaster1\${PRODUCT_NAME}\Uninstall ${PRODUCT_NAME}.lnk" \
        "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0

    WriteRegStr HKLM "${PRODUCT_REG_KEY}" "StartMenuInstalled" "1"
SectionEnd

; ----------------------------------------------------------------------------
; Section 7 — Desktop Shortcuts (optional)
; ----------------------------------------------------------------------------
Section /o "Desktop Shortcuts" SEC_DESKTOP
    ; GUI launcher
    CreateShortCut "$DESKTOP\Mcaster1DNAS.lnk" \
        "$INSTDIR\mcaster1win.exe" "" "$INSTDIR\mcaster1.ico" 0 \
        SW_SHOWNORMAL "" "Mcaster1DNAS Digital Network Audio Server (GUI)"

    ; Start Service (elevated)
    CreateShortCut "$DESKTOP\Start Mcaster1DNAS Service.lnk" \
        "powershell.exe" \
        "-NoProfile -NonInteractive -WindowStyle Hidden -Command $\"Start-Process sc.exe -ArgumentList 'start ${SERVICE_NAME}' -Verb RunAs$\"" \
        "$INSTDIR\mcaster1.ico" 0 SW_SHOWMINIMIZED "" "Start the Mcaster1DNAS Windows Service (requires admin)"

    ; Stop Service (elevated)
    CreateShortCut "$DESKTOP\Stop Mcaster1DNAS Service.lnk" \
        "powershell.exe" \
        "-NoProfile -NonInteractive -WindowStyle Hidden -Command $\"Start-Process sc.exe -ArgumentList 'stop ${SERVICE_NAME}' -Verb RunAs$\"" \
        "$INSTDIR\mcaster1.ico" 0 SW_SHOWMINIMIZED "" "Stop the Mcaster1DNAS Windows Service (requires admin)"

    ; Documentation (opens index.html in default browser)
    WriteIniStr "$DESKTOP\Mcaster1DNAS Documentation.url" \
        "InternetShortcut" "URL" "file:///$INSTDIR\docs\index.html"

    WriteRegStr HKLM "${PRODUCT_REG_KEY}" "DesktopShortcut" "1"
SectionEnd

; =============================================================================
; SECTION DESCRIPTIONS (shown in Components page)
; =============================================================================
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SEC_CORE}     "Core application files: mcaster1win.exe (GUI), mcaster1.exe (console), all runtime DLLs, and default configuration files. Required."
    !insertmacro MUI_DESCRIPTION_TEXT ${SEC_WEB}      "Web interface and admin UI files (web/ and admin/ directories). Required for the browser-based admin dashboard, public status pages, and web player. Recommended."
    !insertmacro MUI_DESCRIPTION_TEXT ${SEC_DOCS}     "Local copy of documentation (docs/ directory plus key README and protocol spec files). View offline without internet access."
    !insertmacro MUI_DESCRIPTION_TEXT ${SEC_SSL}      "Additional SSL test certificate files in ssl\temp\ (selfsigned.key, selfsigned.crt, selfsigned.pem). The main localhost.pem cert is always installed to ssl\. For development and testing only."
    !insertmacro MUI_DESCRIPTION_TEXT ${SEC_SERVICE}  "Register mcaster1Service.exe as a Windows Service that starts automatically with Windows. Uses mcaster1dnas-console.yaml config on ports ${HTTP_PORT}/${HTTPS_PORT}. Requires changing passwords in the config before starting."
    !insertmacro MUI_DESCRIPTION_TEXT ${SEC_STARTMENU} "Add Mcaster1DNAS shortcuts under Start Menu > Mcaster1 > Mcaster1DNAS (GUI launcher, service start/stop with UAC elevation, config editors, docs, logs folder)."
    !insertmacro MUI_DESCRIPTION_TEXT ${SEC_DESKTOP}  "Add Desktop shortcuts for the GUI application, Start/Stop Service (with UAC elevation), and Documentation."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; =============================================================================
; UNINSTALLER
; =============================================================================
Section "Uninstall"
    ; --- Stop and remove service if installed ---
    ReadRegStr $0 HKLM "${PRODUCT_REG_KEY}" "ServiceInstalled"
    ${If} $0 == "1"
        DetailPrint "Stopping Mcaster1DNAS service..."
        nsExec::ExecToLog 'sc stop "${SERVICE_NAME}"'
        Sleep 2000
        nsExec::ExecToLog 'sc delete "${SERVICE_NAME}"'
    ${EndIf}

    ; --- Remove firewall rules (service ports + GUI ports) ---
    DetailPrint "Removing Windows Firewall rules..."
    nsExec::ExecToLog 'netsh advfirewall firewall delete rule name="Mcaster1DNAS HTTP"'
    nsExec::ExecToLog 'netsh advfirewall firewall delete rule name="Mcaster1DNAS HTTPS"'
    nsExec::ExecToLog 'netsh advfirewall firewall delete rule name="Mcaster1DNAS GUI HTTP"'
    nsExec::ExecToLog 'netsh advfirewall firewall delete rule name="Mcaster1DNAS GUI HTTPS"'

    ; --- Remove Start Menu shortcuts (Mcaster1 > Mcaster1DNAS hierarchy) ---
    ReadRegStr $0 HKLM "${PRODUCT_REG_KEY}" "StartMenuInstalled"
    ${If} $0 == "1"
        RMDir /r "$SMPROGRAMS\Mcaster1\${PRODUCT_NAME}"
        RMDir "$SMPROGRAMS\Mcaster1"
    ${EndIf}

    ; --- Remove Desktop shortcuts ---
    ReadRegStr $0 HKLM "${PRODUCT_REG_KEY}" "DesktopShortcut"
    ${If} $0 == "1"
        Delete "$DESKTOP\Mcaster1DNAS.lnk"
        Delete "$DESKTOP\Start Mcaster1DNAS Service.lnk"
        Delete "$DESKTOP\Stop Mcaster1DNAS Service.lnk"
        Delete "$DESKTOP\Mcaster1DNAS Documentation.url"
    ${EndIf}

    ; --- Remove installed files ---
    ; EXEs and DLLs
    Delete "$INSTDIR\mcaster1win.exe"
    Delete "$INSTDIR\mcaster1.exe"
    Delete "$INSTDIR\mcaster1Service.exe"
    Delete "$INSTDIR\mcaster1.ico"
    Delete "$INSTDIR\libcrypto-3-x64.dll"
    Delete "$INSTDIR\libssl-3-x64.dll"
    Delete "$INSTDIR\libxml2.dll"
    Delete "$INSTDIR\libxslt.dll"
    Delete "$INSTDIR\libexslt.dll"
    Delete "$INSTDIR\libcurl.dll"
    Delete "$INSTDIR\libcurl-d.dll"
    Delete "$INSTDIR\zlib1.dll"
    Delete "$INSTDIR\iconv-2.dll"
    Delete "$INSTDIR\charset-1.dll"
    Delete "$INSTDIR\legacy.dll"
    Delete "$INSTDIR\ogg.dll"
    Delete "$INSTDIR\vorbis.dll"
    Delete "$INSTDIR\vorbisenc.dll"
    Delete "$INSTDIR\vorbisfile.dll"
    Delete "$INSTDIR\theora.dll"
    Delete "$INSTDIR\theoradec.dll"
    Delete "$INSTDIR\theoraenc.dll"
    Delete "$INSTDIR\speex-1.dll"
    Delete "$INSTDIR\yaml.dll"
    Delete "$INSTDIR\pthreadVSE3.dll"
    Delete "$INSTDIR\pthreadVC3.dll"
    Delete "$INSTDIR\pthreadVCE3.dll"
    Delete "$INSTDIR\pthreadVSE3d.dll"
    Delete "$INSTDIR\Uninstall.exe"

    ; Config files — prompt before deleting (user may have customised them)
    MessageBox MB_YESNO|MB_ICONQUESTION \
        "Do you want to remove the configuration files?$\n$\n$INSTDIR\mcaster1dnas.yaml$\n$INSTDIR\mcaster1dnas-console.yaml$\n$\nChoose No to keep your configuration." \
        IDNO skip_configs
        Delete "$INSTDIR\mcaster1dnas.yaml"
        Delete "$INSTDIR\mcaster1dnas-console.yaml"
    skip_configs:

    ; Remove web, admin, docs, ssl directories
    RMDir /r "$INSTDIR\web"
    RMDir /r "$INSTDIR\admin"
    RMDir /r "$INSTDIR\docs"
    RMDir /r "$INSTDIR\ssl"

    ; Logs — prompt before deleting
    MessageBox MB_YESNO|MB_ICONQUESTION \
        "Do you want to remove the log files?$\n$\n$INSTDIR\logs\$\n$\nChoose No to keep your logs." \
        IDNO skip_logs
        RMDir /r "$INSTDIR\logs"
    skip_logs:

    ; Remove install dir if empty
    RMDir "$INSTDIR"
    RMDir "$PROGRAMFILES\Mcaster1"

    ; --- Remove registry keys ---
    DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
    DeleteRegKey HKLM "${PRODUCT_REG_KEY}"
    DeleteRegKey /ifempty HKLM "Software\Mcaster1"

    SetAutoClose true
SectionEnd
