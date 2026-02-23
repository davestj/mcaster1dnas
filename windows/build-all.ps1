<#
.SYNOPSIS
    Build all three Mcaster1DNAS projects: Console, Service, Win (MFC).

.PARAMETER Config
    Build configuration: Debug (default) or Release.

.PARAMETER Platform
    Target platform: x64 (default) or Win32.

.EXAMPLE
    .\build-all.ps1
    .\build-all.ps1 -Config Release -Platform x64
#>
param(
    [ValidateSet('Debug','Release')][string]$Config   = 'Debug',
    [ValidateSet('x64','Win32')]    [string]$Platform = 'x64'
)

$ErrorActionPreference = 'Stop'
$SolutionDir = $PSScriptRoot

# Locate MSBuild via vswhere
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) {
    Write-Error "vswhere.exe not found. Is Visual Studio 2022 installed?"
}
$msbuild = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild `
    -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
if (-not $msbuild) { Write-Error "MSBuild not found via vswhere." }

Write-Host "`n=== Building ALL projects [$Config|$Platform] ===" -ForegroundColor Cyan
Write-Host "MSBuild : $msbuild"
Write-Host "Solution: $SolutionDir\Mcaster1DNAS.sln`n"

$sw = [System.Diagnostics.Stopwatch]::StartNew()

& $msbuild "$SolutionDir\Mcaster1DNAS.sln" `
    /p:Configuration=$Config `
    /p:Platform=$Platform `
    /m `
    /nologo `
    /verbosity:minimal

$sw.Stop()
if ($LASTEXITCODE -eq 0) {
    Write-Host "`nBuild SUCCEEDED in $($sw.Elapsed.ToString('mm\:ss'))" -ForegroundColor Green
    Write-Host "Output: $SolutionDir\$Platform\$Config\"
} else {
    Write-Host "`nBuild FAILED (exit $LASTEXITCODE)" -ForegroundColor Red
    exit $LASTEXITCODE
}
