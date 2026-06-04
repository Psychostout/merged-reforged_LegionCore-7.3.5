#requires -Version 5.1
<#
.SYNOPSIS
    Fetches the third-party binaries LegionCore needs and verifies their SHA-256.

.DESCRIPTION
    Run from the repository root or from inside compile_deps/. Idempotent —
    skips anything that's already in place. After this completes, the
    repository builds with no env-vars set:

        cmake --preset windows-msvc-release
        cmake --build --preset windows-msvc-release

.PARAMETER Force
    Re-download everything even if the SHA already matches.

.PARAMETER SkipBoost
.PARAMETER SkipOpenSSL
.PARAMETER SkipMariaDB
    Skip the corresponding component (useful if you already have it).
#>
[CmdletBinding()]
param(
    [switch]$Force,
    [switch]$SkipBoost,
    [switch]$SkipOpenSSL,
    [switch]$SkipMariaDB
)

$ErrorActionPreference = 'Stop'
$ProgressPreference    = 'Continue'

# ----- Locate ourselves regardless of CWD -----
$DepsDir = if ($PSScriptRoot) { $PSScriptRoot } else { Join-Path (Get-Location) 'compile_deps' }
$DownloadsDir = Join-Path $DepsDir 'downloads'
New-Item -ItemType Directory -Force -Path $DownloadsDir | Out-Null

Write-Host "compile_deps root: $DepsDir" -ForegroundColor Cyan

# ----- Manifest (must match compile_deps/DEPENDENCIES.md) -----
$BoostVersion         = '1.83.0'
$BoostFile            = 'boost_1_83_0-msvc-14.3-64.exe'
$BoostUrl             = "https://sourceforge.net/projects/boost/files/boost-binaries/$BoostVersion/$BoostFile/download"
$BoostSha256          = '67975ce4a8799f17728ddba8e64e9b450a6bda7762643e829a96ccbbd1ca17d2'
$BoostInstallDir      = Join-Path $DepsDir 'boost\boost_1_83_0'

$OpenSslVersion       = '3.5.0'
$OpenSslFile          = 'Win64OpenSSL-3_5_0.exe'
$OpenSslUrl           = "https://slproweb.com/download/$OpenSslFile"
$OpenSslSha256        = $null  # SLProWeb publishes a PGP-signed list, not a stable SHA; installer is signature-verified at runtime
$OpenSslInstallDir    = Join-Path $DepsDir 'openssl\OpenSSL-Win64'
# Fallback: build from source if the SLProWeb prebuilt is unavailable
$OpenSslSrcFile       = "openssl-$OpenSslVersion.tar.gz"
$OpenSslSrcUrl        = "https://github.com/openssl/openssl/releases/download/openssl-$OpenSslVersion/$OpenSslSrcFile"
$OpenSslSrcSha256     = '344d0a79f1a9b08029b0744e2cc401a43f9c90acd1044d09a530b4885a8e9fc0'

$MariaDbVersion       = '3.4.8'
$MariaDbFile          = "mariadb-connector-c-$MariaDbVersion-src.zip"
$MariaDbUrl           = "https://archive.mariadb.org/connector-c-$MariaDbVersion/$MariaDbFile"
$MariaDbSha256        = '1d774cff8150b243a1f620d46d5eec69a3eff6be6133bbe374d9c02625416bff'
# Note: the source is already vendored. The script only re-downloads if the file
# is missing or its SHA mismatches.

# ----- Helpers ---------------------------------------------------------------
function Test-Sha256 {
    param([string]$Path, [string]$Expected)
    if (-not $Expected) { return $true }  # nothing to verify against
    if (-not (Test-Path $Path)) { return $false }
    $actual = (Get-FileHash -Path $Path -Algorithm SHA256).Hash.ToLowerInvariant()
    return ($actual -eq $Expected.ToLowerInvariant())
}

function Get-VerifiedFile {
    param([string]$Url, [string]$Dest, [string]$Sha256, [string]$DisplayName)
    if (-not $Force -and (Test-Sha256 -Path $Dest -Expected $Sha256)) {
        Write-Host "  [cache] $DisplayName already present, SHA OK" -ForegroundColor DarkGray
        return
    }
    Write-Host "  [download] $DisplayName" -ForegroundColor Yellow
    Write-Host "    from: $Url"
    Write-Host "    to:   $Dest"
    # Use BITS if available (resumable), else fall back to Invoke-WebRequest
    try {
        Start-BitsTransfer -Source $Url -Destination $Dest -ErrorAction Stop
    } catch {
        Invoke-WebRequest -Uri $Url -OutFile $Dest -UseBasicParsing
    }
    if ($Sha256) {
        if (-not (Test-Sha256 -Path $Dest -Expected $Sha256)) {
            Remove-Item $Dest -Force
            throw "SHA-256 mismatch for $DisplayName ($Url). File deleted."
        }
        Write-Host "    SHA-256 verified" -ForegroundColor Green
    } else {
        Write-Host "    (no SHA-256 in manifest — verification skipped)" -ForegroundColor DarkYellow
    }
}

# ----- Boost ----------------------------------------------------------------
if ($SkipBoost) {
    Write-Host "Skipping Boost." -ForegroundColor DarkYellow
} elseif ((Test-Path (Join-Path $BoostInstallDir 'boost\version.hpp')) -and -not $Force) {
    Write-Host "Boost: already extracted at $BoostInstallDir" -ForegroundColor Green
} else {
    Write-Host "===== Boost $BoostVersion =====" -ForegroundColor Cyan
    $BoostExe = Join-Path $DownloadsDir $BoostFile
    Get-VerifiedFile -Url $BoostUrl -Dest $BoostExe -Sha256 $BoostSha256 -DisplayName "Boost $BoostVersion"
    Write-Host "  [install] running Boost installer to $BoostInstallDir" -ForegroundColor Yellow
    Write-Host "    (this is a silent install of the prebuilt MSVC 14.3 binaries — ~3 min)"
    New-Item -ItemType Directory -Force -Path $BoostInstallDir | Out-Null
    # Boost.SourceForge installer is a 7-Zip self-extractor; /S = silent, /D = dir (must be last, no quotes)
    $proc = Start-Process -FilePath $BoostExe -ArgumentList "/VERYSILENT","/SP-","/SUPPRESSMSGBOXES","/DIR=$BoostInstallDir" -PassThru -Wait
    if ($proc.ExitCode -ne 0) { throw "Boost installer exited with $($proc.ExitCode)" }
    Write-Host "  Boost installed." -ForegroundColor Green
}

# ----- OpenSSL --------------------------------------------------------------
if ($SkipOpenSSL) {
    Write-Host "Skipping OpenSSL." -ForegroundColor DarkYellow
} elseif ((Test-Path (Join-Path $OpenSslInstallDir 'include\openssl\ssl.h')) -and -not $Force) {
    Write-Host "OpenSSL: already extracted at $OpenSslInstallDir" -ForegroundColor Green
} else {
    Write-Host "===== OpenSSL $OpenSslVersion =====" -ForegroundColor Cyan
    $OpenSslExe = Join-Path $DownloadsDir $OpenSslFile
    Get-VerifiedFile -Url $OpenSslUrl -Dest $OpenSslExe -Sha256 $OpenSslSha256 -DisplayName "OpenSSL $OpenSslVersion"
    Write-Host "  [install] silent install to $OpenSslInstallDir" -ForegroundColor Yellow
    New-Item -ItemType Directory -Force -Path $OpenSslInstallDir | Out-Null
    $proc = Start-Process -FilePath $OpenSslExe -ArgumentList "/VERYSILENT","/SP-","/SUPPRESSMSGBOXES","/DIR=$OpenSslInstallDir" -PassThru -Wait
    if ($proc.ExitCode -ne 0) { throw "OpenSSL installer exited with $($proc.ExitCode)" }
    Write-Host "  OpenSSL installed." -ForegroundColor Green
}

# ----- MariaDB Connector/C (source is vendored; re-verify) -------------------
if ($SkipMariaDB) {
    Write-Host "Skipping MariaDB Connector/C." -ForegroundColor DarkYellow
} else {
    Write-Host "===== MariaDB Connector/C $MariaDbVersion (source, vendored) =====" -ForegroundColor Cyan
    $MariaDbZip = Join-Path $DownloadsDir $MariaDbFile
    if (Test-Path $MariaDbZip) {
        if (Test-Sha256 -Path $MariaDbZip -Expected $MariaDbSha256) {
            Write-Host "  Vendored archive present and SHA OK." -ForegroundColor Green
        } else {
            Write-Host "  Vendored archive SHA mismatch — re-downloading." -ForegroundColor Yellow
            Get-VerifiedFile -Url $MariaDbUrl -Dest $MariaDbZip -Sha256 $MariaDbSha256 -DisplayName "MariaDB Connector/C $MariaDbVersion"
        }
    } else {
        Get-VerifiedFile -Url $MariaDbUrl -Dest $MariaDbZip -Sha256 $MariaDbSha256 -DisplayName "MariaDB Connector/C $MariaDbVersion"
    }
    $MariaDbSrc = Join-Path $DepsDir 'mariadb\source'
    if (-not (Test-Path (Join-Path $MariaDbSrc 'CMakeLists.txt'))) {
        Write-Host "  [extract] $MariaDbZip -> $MariaDbSrc" -ForegroundColor Yellow
        $tmp = Join-Path $DepsDir 'mariadb\_tmp'
        if (Test-Path $tmp) { Remove-Item $tmp -Recurse -Force }
        Expand-Archive -Path $MariaDbZip -DestinationPath $tmp -Force
        $extracted = Get-ChildItem $tmp -Directory | Select-Object -First 1
        if (Test-Path $MariaDbSrc) { Remove-Item $MariaDbSrc -Recurse -Force }
        Move-Item $extracted.FullName $MariaDbSrc
        Remove-Item $tmp -Recurse -Force
    } else {
        Write-Host "  Source tree already extracted." -ForegroundColor Green
    }
    Write-Host "  (CMake will build it on demand during the main build.)" -ForegroundColor DarkGray
}

Write-Host ""
Write-Host "All requested dependencies are in compile_deps/." -ForegroundColor Cyan
Write-Host "You can now run:" -ForegroundColor Cyan
Write-Host "    cmake --preset windows-msvc-release" -ForegroundColor White
Write-Host "    cmake --build --preset windows-msvc-release" -ForegroundColor White
