# scripts/bump-version.ps1
# Mcaster1DNAS Windows version bump script
#
# Usage:
#   .\scripts\bump-version.ps1                  # show current version
#   .\scripts\bump-version.ps1 -Set 2.5.3-dev   # set explicit version
#   .\scripts\bump-version.ps1 -Bump patch       # bump patch: 2.5.2 -> 2.5.3
#   .\scripts\bump-version.ps1 -Bump minor       # bump minor: 2.5.2 -> 2.6.0
#   .\scripts\bump-version.ps1 -Bump major       # bump major: 2.5.2 -> 3.0.0
#   .\scripts\bump-version.ps1 -Bump patch -Suffix dev  # -> 2.5.3-dev
#
# Files updated:
#   VERSION                          (canonical source)
#   windows/config.h                 (PACKAGE_VERSION, PACKAGE_STRING, VERSION)
#   windows/git-version.ps1          ($baseVer fallback)
#   GIT-VERSION-GEN                  (BASE_VER fallback)
#   windows/mcaster1dnas.iss         (AppVerName, OutputBaseFilename, VersionInfoVersion)
#   windows/Mcaster1Win.rc           (FILEVERSION, PRODUCTVERSION, FileVersion, ProductVersion)
#   build.sh                         (display line)
#   README.md                        (version badge)

param(
    [string]$Set    = "",
    [string]$Bump   = "",
    [string]$Suffix = ""
)

$ErrorActionPreference = "Stop"

$RepoRoot   = Split-Path $PSScriptRoot
$VersionFile = Join-Path $RepoRoot "VERSION"

function Write-Step($msg) { Write-Host "  --> $msg" -ForegroundColor Cyan }
function Write-Ok($msg)   { Write-Host "  [OK] $msg" -ForegroundColor Green }
function Write-Warn($msg) { Write-Host "  [!!] $msg" -ForegroundColor Yellow }

# ── Read current version ────────────────────────────────────────────────────
$current = (Get-Content $VersionFile -Raw).Trim()
Write-Host ""
Write-Host "=======================================" -ForegroundColor Magenta
Write-Host "  Mcaster1DNAS Version Manager (Win)"  -ForegroundColor Magenta
Write-Host "=======================================" -ForegroundColor Magenta
Write-Host "  Current VERSION: $current"
Write-Host ""

if (-not $Set -and -not $Bump) {
    # Show-only mode
    Write-Host "  Use -Set <version> or -Bump [major|minor|patch] to change." -ForegroundColor Yellow
    Write-Host ""
    exit 0
}

# ── Compute new version ─────────────────────────────────────────────────────
if ($Set) {
    $newVer = $Set.Trim()
} else {
    # Strip any pre-release suffix for arithmetic
    $baseOnly = $current -replace '-.*$', ''
    $parts    = $baseOnly.Split('.')
    $major    = [int]$parts[0]
    $minor    = [int]$parts[1]
    $patch    = if ($parts.Length -ge 3) { [int]$parts[2] } else { 0 }

    switch ($Bump.ToLower()) {
        "major" { $major++; $minor = 0; $patch = 0 }
        "minor" { $minor++; $patch = 0 }
        "patch" { $patch++ }
        default { Write-Host "  Unknown -Bump value '$Bump'. Use major, minor, or patch." -ForegroundColor Red; exit 1 }
    }

    $newVer = "$major.$minor.$patch"
    if ($Suffix) { $newVer = "$newVer-$Suffix" }
}

Write-Host "  New VERSION    : $newVer" -ForegroundColor Green
Write-Host ""

# ── Numeric quad for .rc / .iss (strip pre-release suffix) ─────────────────
$numOnly = $newVer -replace '-.*$', ''
$numParts = $numOnly.Split('.')
$v1 = if ($numParts.Length -ge 1) { $numParts[0] } else { "0" }
$v2 = if ($numParts.Length -ge 2) { $numParts[1] } else { "0" }
$v3 = if ($numParts.Length -ge 3) { $numParts[2] } else { "0" }
$quad    = "$v1,$v2,$v3,0"       # e.g. 2,5,2,0  for FILEVERSION
$dotquad = "$v1.$v2.$v3.0"      # e.g. 2.5.2.0  for VersionInfoVersion in .iss
$spaced  = "$v1, $v2, $v3, 0"   # e.g. 2, 5, 2, 0  for RC string values

# ── 1. VERSION ──────────────────────────────────────────────────────────────
Write-Step "VERSION"
Set-Content -Path $VersionFile -Value $newVer -NoNewline
Write-Ok "VERSION = $newVer"

# ── 2. windows/config.h ─────────────────────────────────────────────────────
Write-Step "windows/config.h"
$f = Join-Path $RepoRoot "windows\config.h"
$c = Get-Content $f -Raw
$c = $c -replace '#define PACKAGE_VERSION "[^"]*"', "#define PACKAGE_VERSION `"$newVer`""
$c = $c -replace '#define PACKAGE_STRING  "[^"]*"', "#define PACKAGE_STRING  `"Mcaster1DNAS $newVer`""
$c = $c -replace '#define VERSION         "[^"]*"', "#define VERSION         `"$newVer`""
[System.IO.File]::WriteAllText($f, $c, [System.Text.Encoding]::UTF8)
Write-Ok "config.h updated"

# ── 3. windows/git-version.ps1 ──────────────────────────────────────────────
Write-Step "windows/git-version.ps1"
$f = Join-Path $RepoRoot "windows\git-version.ps1"
$c = Get-Content $f -Raw
$c = $c -replace '\$baseVer = "[^"]*"', "`$baseVer = `"$newVer`""
[System.IO.File]::WriteAllText($f, $c, [System.Text.Encoding]::ASCII)
Write-Ok "git-version.ps1 fallback updated"

# ── 4. GIT-VERSION-GEN ──────────────────────────────────────────────────────
Write-Step "GIT-VERSION-GEN"
$f = Join-Path $RepoRoot "GIT-VERSION-GEN"
$c = Get-Content $f -Raw
$c = $c -replace 'BASE_VER="[^"]*"', "BASE_VER=`"$newVer`""
[System.IO.File]::WriteAllText($f, $c, [System.Text.Encoding]::ASCII)
Write-Ok "GIT-VERSION-GEN fallback updated"

# ── 5. windows/mcaster1dnas.iss ─────────────────────────────────────────────
Write-Step "windows/mcaster1dnas.iss"
$f = Join-Path $RepoRoot "windows\mcaster1dnas.iss"
if (Test-Path $f) {
    $c = Get-Content $f -Raw
    $c = $c -replace 'AppVerName=Mcaster1DNAS v[^\r\n]*',      "AppVerName=Mcaster1DNAS v$newVer"
    $c = $c -replace 'OutputBaseFilename=mcaster1dnas_win32_v[^\r\n]*', "OutputBaseFilename=mcaster1dnas_win32_v${newVer}_setup"
    $c = $c -replace 'VersionInfoVersion=[^\r\n]*',             "VersionInfoVersion=$dotquad"
    [System.IO.File]::WriteAllText($f, $c, [System.Text.Encoding]::UTF8)
    Write-Ok "mcaster1dnas.iss updated"
} else {
    Write-Warn "mcaster1dnas.iss not found — skipped"
}

# ── 6. windows/Mcaster1Win.rc ───────────────────────────────────────────────
Write-Step "windows/Mcaster1Win.rc"
$f = Join-Path $RepoRoot "windows\Mcaster1Win.rc"
if (Test-Path $f) {
    $c = Get-Content $f -Raw
    $c = $c -replace ' FILEVERSION [0-9,]+',              " FILEVERSION $quad"
    $c = $c -replace ' PRODUCTVERSION [0-9,]+',           " PRODUCTVERSION $quad"
    $c = $c -replace 'VALUE "FileVersion", "[^"]*"',      "VALUE `"FileVersion`", `"$spaced`""
    $c = $c -replace 'VALUE "ProductVersion", "[^"]*"',   "VALUE `"ProductVersion`", `"$spaced`""
    [System.IO.File]::WriteAllText($f, $c, [System.Text.Encoding]::UTF8)
    Write-Ok "Mcaster1Win.rc updated"
} else {
    Write-Warn "Mcaster1Win.rc not found — skipped"
}

# ── 7. build.sh ─────────────────────────────────────────────────────────────
Write-Step "build.sh"
$f = Join-Path $RepoRoot "build.sh"
if (Test-Path $f) {
    $c = Get-Content $f -Raw
    $c = $c -replace 'Mcaster1DNAS Build Script v[^\s"\\n]*', "Mcaster1DNAS Build Script v$newVer"
    [System.IO.File]::WriteAllText($f, $c, [System.Text.Encoding]::UTF8)
    Write-Ok "build.sh updated"
}

# ── 8. README.md version badge ───────────────────────────────────────────────
Write-Step "README.md"
$f = Join-Path $RepoRoot "README.md"
if (Test-Path $f) {
    $c = Get-Content $f -Raw
    # Badge: version-X.Y.Z--suffix  (shields.io uses -- for a literal dash)
    $badgeVer = $newVer -replace '-', '--'
    $c = $c -replace '\[!\[Version\]\(https://img\.shields\.io/badge/version-[^)]+\)\]\([^)]+\)',
        "[![Version](https://img.shields.io/badge/version-$badgeVer-brightgreen.svg)](https://github.com/davestj/mcaster1dnas/releases)"
    [System.IO.File]::WriteAllText($f, $c, [System.Text.Encoding]::UTF8)
    Write-Ok "README.md badge updated"
}

# ── Summary ──────────────────────────────────────────────────────────────────
Write-Host ""
Write-Host "=======================================" -ForegroundColor Magenta
Write-Host "  Done!  New version: $newVer"          -ForegroundColor Green
Write-Host "=======================================" -ForegroundColor Magenta
Write-Host ""
Write-Host "  Next steps:" -ForegroundColor Yellow
Write-Host "    1. Rebuild:  msbuild windows\Mcaster1DNAS.sln /p:Configuration=Debug /p:Platform=x64"
Write-Host "    2. Review:   git diff"
Write-Host "    3. Commit:   git add VERSION windows/config.h windows/git-version.ps1 GIT-VERSION-GEN"
Write-Host "                 git add windows/mcaster1dnas.iss windows/Mcaster1Win.rc build.sh README.md"
Write-Host "                 git commit -m `"chore: bump version to $newVer`""
Write-Host ""
