# windows/installer/prepare-images.ps1
# Converts PNG source images to BMP files required by NSIS MUI2.
#
# NSIS MUI2 image requirements:
#   Sidebar BMP  (MUI_WELCOMEFINISHPAGE_BITMAP): 164 x 314 px - shown on Welcome/Finish pages
#   Header BMP   (MUI_HEADERIMAGE_BITMAP):       150 x  57 px - shown in page header bar
#   Splash BMP   (AdvSplash):                    600 x 400 px - full-screen splash at startup
#
# Source images (at repo root):
#   mcaster1dnas-splash1.png      -> sidebar + splash BMP
#   mcaster1dnas-header-banner.png -> header BMP
#
# Requires: .NET System.Drawing (built into Windows PowerShell 5.1+)
#           No external tools needed.

$ErrorActionPreference = "Stop"

Add-Type -AssemblyName System.Drawing

$RepoRoot      = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$InstallerDir  = $PSScriptRoot
$SplashSrc     = Join-Path $RepoRoot "mcaster1dnas-splash1.png"
$BannerSrc     = Join-Path $RepoRoot "mcaster1dnas-header-banner.png"

function Convert-ImageToBmp {
    param(
        [string]$SourcePath,
        [string]$DestPath,
        [int]$Width,
        [int]$Height
    )
    if (-not (Test-Path $SourcePath)) {
        Write-Host "  [SKIP] Source not found: $SourcePath" -ForegroundColor Yellow
        return
    }
    $src = [System.Drawing.Image]::FromFile($SourcePath)
    $dst = New-Object System.Drawing.Bitmap($Width, $Height)
    $g   = [System.Drawing.Graphics]::FromImage($dst)
    $g.InterpolationMode  = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $g.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
    $g.SmoothingMode      = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
    $g.DrawImage($src, 0, 0, $Width, $Height)
    $g.Dispose()
    $src.Dispose()
    $dst.Save($DestPath, [System.Drawing.Imaging.ImageFormat]::Bmp)
    $dst.Dispose()
    Write-Host "  [OK] $([System.IO.Path]::GetFileName($DestPath))  ($Width x $Height)" -ForegroundColor Green
}

Write-Host ""
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "  Mcaster1DNAS Installer Image Prep"       -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

# -- Sidebar (164x314) from splash image --
Convert-ImageToBmp -SourcePath $SplashSrc -DestPath (Join-Path $InstallerDir "sidebar.bmp") -Width 164 -Height 314

# -- Splash (600x400) from splash image --
Convert-ImageToBmp -SourcePath $SplashSrc -DestPath (Join-Path $InstallerDir "splash.bmp") -Width 600 -Height 400

# -- Header (150x57) from header-banner image --
Convert-ImageToBmp -SourcePath $BannerSrc -DestPath (Join-Path $InstallerDir "header.bmp") -Width 150 -Height 57

Write-Host ""
Write-Host "  Images prepared in: $InstallerDir" -ForegroundColor Cyan
Write-Host ""
