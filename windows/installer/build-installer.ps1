# windows/installer/build-installer.ps1
# Builds the Mcaster1DNAS NSIS installer.
#
# Usage (from repo root or windows/installer/):
#   powershell -ExecutionPolicy Bypass -File windows\installer\build-installer.ps1
#   powershell -ExecutionPolicy Bypass -File windows\installer\build-installer.ps1 -Config Release
#   powershell -ExecutionPolicy Bypass -File windows\installer\build-installer.ps1 -SkipBuild -SkipImages

param(
    [string]$Config      = "Debug",     # Debug or Release
    [switch]$SkipBuild   = $false,      # Skip MSBuild step
    [switch]$SkipImages  = $false,      # Skip image preparation step
    [switch]$SkipNSIS    = $false       # Dry-run: prepare only, don't run makensis
)

$ErrorActionPreference = "Stop"

$InstallerDir = $PSScriptRoot
$RepoRoot     = [System.IO.Path]::GetFullPath((Join-Path $InstallerDir "..\.."))
$WindowsDir   = Join-Path $RepoRoot "windows"
$BuildDir     = Join-Path $WindowsDir "x64\$Config"
$NsiFile      = Join-Path $InstallerDir "mcaster1dnas.nsi"
$MakensisExe  = "C:\Program Files (x86)\NSIS\makensis.exe"
$MsbuildExe   = "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
$SolutionFile = Join-Path $WindowsDir "Mcaster1DNAS.sln"

# Read version from VERSION file
$VersionFile = Join-Path $RepoRoot "VERSION"
$Version     = if (Test-Path $VersionFile) { (Get-Content $VersionFile -Raw).Trim() } else { "2.5.2-dev" }

function Write-Banner($msg) {
    Write-Host ""
    Write-Host "==========================================" -ForegroundColor Magenta
    Write-Host "  $msg" -ForegroundColor Magenta
    Write-Host "==========================================" -ForegroundColor Magenta
}
function Write-Step($msg)  { Write-Host "  --> $msg" -ForegroundColor Cyan }
function Write-Ok($msg)    { Write-Host "  [OK] $msg" -ForegroundColor Green }
function Write-Warn($msg)  { Write-Host "  [!!] $msg" -ForegroundColor Yellow }
function Write-Err($msg)   { Write-Host "  [ERR] $msg" -ForegroundColor Red }

Write-Banner "Mcaster1DNAS Installer Builder v$Version"
Write-Host "  Config   : $Config"
Write-Host "  BuildDir : $BuildDir"
Write-Host "  RepoRoot : $RepoRoot"
Write-Host ""

# =============================================================================
# Step 1 - Build the solution (optional)
# =============================================================================
if (-not $SkipBuild) {
    Write-Step "Building Mcaster1DNAS.sln ($Config | x64)..."
    if (-not (Test-Path $MsbuildExe)) {
        # Try Community edition fallback
        $MsbuildExe = "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    }
    if (-not (Test-Path $MsbuildExe)) {
        Write-Warn "MSBuild not found at expected paths. Skipping build step."
        Write-Warn "Set -SkipBuild or fix MsbuildExe path in this script."
    } else {
        & $MsbuildExe $SolutionFile /p:Configuration=$Config /p:Platform=x64 /t:Build /v:minimal /nologo
        if ($LASTEXITCODE -ne 0) {
            Write-Err "MSBuild failed with exit code $LASTEXITCODE"
            exit $LASTEXITCODE
        }
        Write-Ok "Build complete"
    }
} else {
    Write-Warn "Skipping MSBuild step (-SkipBuild)"
}

# =============================================================================
# Step 2a - Ensure vcpkg runtime DLLs are in the build output directory.
# MSBuild post-build events copy these, but sometimes only a subset lands
# (AppLocal selective copy vs. our copy-vcpkg-deps full copy).
# We always sync the full release bin here so the installer is never missing DLLs.
# =============================================================================
$VcpkgBin = "$env:USERPROFILE\dev\vcpkg\installed\x64-windows\bin"
if (Test-Path $VcpkgBin) {
    $vcpkgDlls = Get-ChildItem -Path $VcpkgBin -Filter '*.dll' -ErrorAction SilentlyContinue
    $synced = 0
    foreach ($dll in $vcpkgDlls) {
        $dst = Join-Path $BuildDir $dll.Name
        # Only copy if missing or older than source
        if (-not (Test-Path $dst) -or $dll.LastWriteTime -gt (Get-Item $dst).LastWriteTime) {
            Copy-Item $dll.FullName $BuildDir -Force
            $synced++
        }
    }
    if ($synced -gt 0) { Write-Ok "Synced $synced vcpkg DLL(s) to $BuildDir" }
}

# =============================================================================
# Step 2 - Verify required EXEs and DLLs exist
# =============================================================================
Write-Step "Verifying build output..."
$RequiredFiles = @(
    "mcaster1win.exe", "mcaster1.exe",
    "libcrypto-3-x64.dll", "libssl-3-x64.dll",
    "libxml2.dll", "libxslt.dll", "libcurl.dll",
    "ogg.dll", "vorbis.dll", "yaml.dll", "zlib1.dll",
    "pthreadVSE3.dll"
)
$Missing = @()
foreach ($f in $RequiredFiles) {
    $p = Join-Path $BuildDir $f
    if (-not (Test-Path $p)) { $Missing += $f }
}
if ($Missing.Count -gt 0) {
    Write-Err "Missing required files in ${BuildDir}:"
    $Missing | ForEach-Object { Write-Host "    - $_" -ForegroundColor Red }
    Write-Err "Run the build first or use -Config Release to point at a Release build."
    exit 1
}
Write-Ok "All required files present in $BuildDir"

# =============================================================================
# Step 3 - Prepare installer images (PNG -> BMP)
# =============================================================================
if (-not $SkipImages) {
    Write-Step "Preparing installer images..."
    $PrepScript = Join-Path $InstallerDir "prepare-images.ps1"
    if (Test-Path $PrepScript) {
        & powershell -ExecutionPolicy Bypass -File $PrepScript
        if ($LASTEXITCODE -ne 0) {
            Write-Warn "Image preparation returned exit code $LASTEXITCODE - continuing anyway."
        }
    } else {
        Write-Warn "prepare-images.ps1 not found - skipping image prep."
        Write-Warn "Installer will use existing .bmp files in $InstallerDir"
    }
    # Verify BMP files exist
    foreach ($bmp in @("splash.bmp", "sidebar.bmp", "header.bmp")) {
        if (-not (Test-Path (Join-Path $InstallerDir $bmp))) {
            Write-Warn "Missing $bmp - installer header/splash may show blank areas."
        }
    }
} else {
    Write-Warn "Skipping image preparation (-SkipImages)"
}

# =============================================================================
# Step 4 - Run makensis
# =============================================================================
if (-not $SkipNSIS) {
    Write-Step "Running makensis..."
    if (-not (Test-Path $MakensisExe)) {
        Write-Err "makensis.exe not found at: $MakensisExe"
        Write-Err "Install NSIS from https://nsis.sourceforge.io and re-run."
        exit 1
    }

    # Change to installer dir so relative paths in .nsi resolve correctly
    Push-Location $InstallerDir
    try {
        & $MakensisExe /V3 $NsiFile
        $nsisExit = $LASTEXITCODE
    } finally {
        Pop-Location
    }

    if ($nsisExit -ne 0) {
        Write-Err "makensis failed with exit code $nsisExit"
        exit $nsisExit
    }

    # Find the generated installer
    $OutExe = Join-Path $InstallerDir "mcaster1dnas_win64_v${Version}_setup.exe"
    if (Test-Path $OutExe) {
        $size = [math]::Round((Get-Item $OutExe).Length / 1MB, 1)
        Write-Host ""
        Write-Ok "Installer created: $OutExe  ($size MB)"
    } else {
        Write-Warn "Expected output not found: $OutExe"
        Write-Warn "Check the .nsi OutFile definition and makensis output above."
    }
} else {
    Write-Warn "Skipping makensis step (-SkipNSIS)"
}

Write-Banner "Done  v$Version"
Write-Host "  Installer: windows\installer\mcaster1dnas_win64_v${Version}_setup.exe"
Write-Host ""
Write-Host "  IMPORTANT: Before distributing, edit passwords in:"
Write-Host "    ${RepoRoot}\mcaster1dnas-console.yaml"
Write-Host "    ${WindowsDir}\mcaster1dnas.yaml"
Write-Host ""
