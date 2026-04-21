<#
.SYNOPSIS
    Post-build helper: copies vcpkg dependency DLLs to the VS output directory.

.DESCRIPTION
    Called automatically by the post-build event in each vcxproj.
    Selects the correct bin path (x64/x86, debug/release) and copies
    every runtime DLL the projects need to run in-place.

.PARAMETER OutDir
    MSBuild $(OutDir) — the build output directory (trailing backslash OK).

.PARAMETER Platform
    MSBuild $(Platform): "x64" or "Win32".

.PARAMETER Config
    MSBuild $(Configuration): "Debug" or "Release".

.PARAMETER VcpkgRoot
    Path to vcpkg root.  Defaults to %USERPROFILE%\dev\vcpkg.

.EXAMPLE
    .\copy-vcpkg-deps.ps1 -OutDir "x64\Debug\" -Platform x64 -Config Debug
#>
param(
    [Parameter(Mandatory)][string]$OutDir,
    [Parameter(Mandatory)][string]$Platform,
    [Parameter(Mandatory)][string]$Config,
    [string]$VcpkgRoot = "$env:USERPROFILE\dev\vcpkg"
)

$ErrorActionPreference = 'Stop'

# Resolve output directory (strip trailing backslash to avoid xcopy quirks)
$outDir = $OutDir.TrimEnd('\', '/')
if (-not (Test-Path $outDir)) {
    New-Item -ItemType Directory -Path $outDir -Force | Out-Null
}

# Determine vcpkg arch triplet
$arch = if ($Platform -eq 'x64') { 'x64-windows' } else { 'x86-windows' }

# All projects link against release import libs (libcurl.lib, pthreadVSE3.lib, etc.)
# so we always source DLLs from the release bin regardless of Debug/Release config.
$binDir = Join-Path $VcpkgRoot "installed\$arch\bin"

if (-not (Test-Path $binDir)) {
    Write-Warning "copy-vcpkg-deps: bin dir not found: $binDir"
    exit 0
}

# Copy ALL DLLs from the vcpkg release bin — no hardcoded list to maintain.
# This automatically picks up legacy.dll, any new packages, and future updates.
$copied = 0
$dlls   = Get-ChildItem -Path $binDir -Filter '*.dll' -ErrorAction SilentlyContinue

foreach ($dll in $dlls) {
    Copy-Item $dll.FullName $outDir -Force
    $copied++
}

Write-Host "copy-vcpkg-deps: [$Platform|$Config] copied $copied DLL(s) from $binDir -> $outDir"
