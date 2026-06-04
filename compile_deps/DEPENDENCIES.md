# Dependency manifest

This file is the **single source of truth** for what `setup_deps.ps1` /
`setup_deps.sh` fetches. SHA-256 hashes here are verified at download time;
the scripts will refuse to proceed on a mismatch. To bump a dependency,
update this file AND the matching constants at the top of each setup script.

All SHA-256 hashes below were obtained by either (a) re-downloading the
archive in a clean sandbox and running `sha256sum` against it, or (b) using
the project's officially-published checksum file (preferred when available).
Both methods were used and agreed before publishing.

---

## Vendored in this repo (committed to git)

| Component | Version | File | SHA-256 | Source (verified) |
|---|---|---|---|---|
| MariaDB Connector/C source | 3.4.8 | `downloads/mariadb-connector-c-3.4.8-src.zip` | `1d774cff8150b243a1f620d46d5eec69a3eff6be6133bbe374d9c02625416bff` | `archive.mariadb.org/connector-c-3.4.8/sha256sums.txt` |

Already extracted into `mariadb/source/` so the project will build the
connector as a sub-project if no system MariaDB is found.

---

## Fetched on first run

Boost and OpenSSL are too large to vendor in-tree (the repo would balloon
to ~250 MB). `setup_deps.{ps1,sh}` downloads them from the project's
official mirror, verifies SHA-256 against the values below, and unpacks
them under `compile_deps/`.

### Boost

| Platform | Version | File | SHA-256 | Source |
|---|---|---|---|---|
| Cross-platform source (CMake-friendly archive) | **1.83.0** | `boost-1.83.0.tar.xz` | `c5a0688e1f0c05f354bbd0b32244d36085d9ffc9f932e8a18983a9908096f614` | <https://github.com/boostorg/boost/releases/download/boost-1.83.0/boost-1.83.0.tar.xz> |
| Windows x64 (prebuilt MSVC 14.3 / VS 2022) | 1.83.0 | `boost_1_83_0-msvc-14.3-64.exe` | `67975ce4a8799f17728ddba8e64e9b450a6bda7762643e829a96ccbbd1ca17d2` | <https://sourceforge.net/projects/boost/files/boost-binaries/1.83.0/> |
| Linux source (legacy non-modular) | 1.83.0 | `boost_1_83_0.tar.bz2` | `79e6d3f986444e5a80afbeccdaf2d1c1cf964baa8d766d20859d653a16c39848` | <https://archives.boost.io/release/1.83.0/source/> |

> Linux setup builds Boost from the **CMake-friendly GitHub tarball** by
> default (smaller download, faster build); pass `--legacy-boost` to
> setup_deps.sh to use the classic `.tar.bz2` + `b2` flow.

### OpenSSL

| Platform | Version | File | SHA-256 | Source |
|---|---|---|---|---|
| Source (all platforms) | **3.5.0 LTS** | `openssl-3.5.0.tar.gz` | `344d0a79f1a9b08029b0744e2cc401a43f9c90acd1044d09a530b4885a8e9fc0` | <https://github.com/openssl/openssl/releases/download/openssl-3.5.0/openssl-3.5.0.tar.gz> |
| Windows x64 (prebuilt by Shining Light) | 3.5.0 | `Win64OpenSSL-3_5_0.exe` | _signature-verified at runtime by the installer_ | <https://slproweb.com/products/Win32OpenSSL.html> |

> **OpenSSL 3.5 is the current Long-Term-Support series**, supported through
> April 2030 by the OpenSSL Foundation. We deliberately track the LTS series
> rather than the latest minor.
>
> Linux setup auto-skips the OpenSSL fetch if `pkg-config --atleast-version=3.0 libssl`
> succeeds for the system OpenSSL.

---

## Why these specific versions

- **Boost 1.83** — matches the floor declared in `dep/boost/CMakeLists.txt`
  and is the version verified to build `bnetserver`. Boost 1.83 still ships
  the legacy Boost.Asio `boost/asio/io_service.hpp` header used by the current
  bnetserver REST code. Boost 1.85+ may require a small Asio include
  modernization before it can be used reliably.
- **OpenSSL 3.5.0 LTS** — current LTS, ships with PQC algorithms
  (ML-KEM, ML-DSA, SLH-DSA), QUIC, integrity-only TLS suites. Drops support
  for OpenSSL 1.1 (already EOL).
- **MariaDB Connector/C 3.4.8** — current stable, supports MariaDB 10.5
  through 11.x. Matches the project's declared 10.6.3 floor.

---

## Re-running from a known-good state

```bash
rm -rf compile_deps/{boost,openssl,mariadb/windows-x64,mariadb/linux-x86_64}
# (downloads/ cache is kept so SHA-verified archives don't re-download)
./compile_deps/setup_deps.sh        # or .ps1 on Windows
```

To force a re-download (e.g. if you suspect cache corruption):

```bash
./compile_deps/setup_deps.sh --force
```
