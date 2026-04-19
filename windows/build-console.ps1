<#
.SYNOPSIS
    Build Mcaster1Console — the standalone mcaster1.exe server binary.

.PARAMETER Config
    Build configuration: Debug (default) or Release.

.PARAMETER Platform
    Target platform: x64 (default) or Win32.

.EXAMPLE
    .\build-console.ps1
    .\build-console.ps1 -Config Release
#>
param(
    [ValidateSet('Debug','Release')][string]$Config   = 'Debug',
    [ValidateSet('x64','Win32')]    [string]$Platform = 'x64'
)

$ErrorActionPreference = 'Stop'
$SolutionDir = $PSScriptRoot

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) { Write-Error "vswhere.exe not found." }
$msbuild = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild `
    -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
if (-not $msbuild) { Write-Error "MSBuild not found via vswhere." }

Write-Host "`n=== Building Mcaster1Console [$Config|$Platform] ===" -ForegroundColor Cyan

$sw = [System.Diagnostics.Stopwatch]::StartNew()

& $msbuild "$SolutionDir\Mcaster1Console.vcxproj" `
    /p:Configuration=$Config `
    /p:Platform=$Platform `
    /p:SolutionDir="$SolutionDir\\" `
    /nologo `
    /verbosity:minimal

$sw.Stop()
if ($LASTEXITCODE -eq 0) {
    $outDir = "$SolutionDir\$Platform\$Config"
    Write-Host "`nBuild SUCCEEDED in $($sw.Elapsed.ToString('mm\:ss'))" -ForegroundColor Green
    Write-Host "Binary : $outDir\mcaster1.exe"
    Write-Host "Run    : & `"$outDir\mcaster1.exe`" -c `"$SolutionDir\mcaster1-windev.yaml`""
} else {
    Write-Host "`nBuild FAILED (exit $LASTEXITCODE)" -ForegroundColor Red
    exit $LASTEXITCODE
}
