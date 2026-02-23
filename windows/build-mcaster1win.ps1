<#
.SYNOPSIS
    Build Mcaster1Win — the MFC Windows GUI application (mcaster1win.exe).

.PARAMETER Config
    Build configuration: Debug (default) or Release.

.PARAMETER Platform
    Target platform: x64 (default) or Win32.

.PARAMETER Run
    After a successful build, launch mcaster1win.exe with the dev config.

.EXAMPLE
    .\build-mcaster1win.ps1
    .\build-mcaster1win.ps1 -Config Release -Run
#>
param(
    [ValidateSet('Debug','Release')][string]$Config   = 'Debug',
    [ValidateSet('x64','Win32')]    [string]$Platform = 'x64',
    [switch]$Run
)

$ErrorActionPreference = 'Stop'
$SolutionDir = $PSScriptRoot

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) { Write-Error "vswhere.exe not found." }
$msbuild = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild `
    -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
if (-not $msbuild) { Write-Error "MSBuild not found via vswhere." }

Write-Host "`n=== Building Mcaster1Win (MFC) [$Config|$Platform] ===" -ForegroundColor Cyan

$sw = [System.Diagnostics.Stopwatch]::StartNew()

& $msbuild "$SolutionDir\Mcaster1Win.vcxproj" `
    /p:Configuration=$Config `
    /p:Platform=$Platform `
    /p:SolutionDir="$SolutionDir\\" `
    /nologo `
    /verbosity:minimal

$sw.Stop()
if ($LASTEXITCODE -ne 0) {
    Write-Host "`nBuild FAILED (exit $LASTEXITCODE)" -ForegroundColor Red
    exit $LASTEXITCODE
}

$outDir = "$SolutionDir\$Platform\$Config"
Write-Host "`nBuild SUCCEEDED in $($sw.Elapsed.ToString('mm\:ss'))" -ForegroundColor Green
Write-Host "Binary : $outDir\mcaster1win.exe"

if ($Run) {
    $cfg = "$SolutionDir\mcaster1-windev.yaml"
    Write-Host "Launching: $outDir\mcaster1win.exe -c `"$cfg`"" -ForegroundColor Yellow
    & "$outDir\mcaster1win.exe" -c $cfg
}
