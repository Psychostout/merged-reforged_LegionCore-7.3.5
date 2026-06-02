# Building LegionCore-7.3.5 on Windows

This guide reflects the merged `merged-reforged` branch with the modernized
toolchain (CMake 4.3.2, Boost 1.85+, MariaDB 10.6.3+).

> If you are an end-user / server-admin and you just want a binary,
> ask the Discord. This document is for people who want to compile.

---

## 0. TL;DR (zero-config build, ~30 minutes total)

```powershell
git clone <your fork URL> LegionCore
cd LegionCore
.\compile_deps\setup_deps.ps1            # downloads & verifies Boost / OpenSSL / MariaDB
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
```

The `setup_deps.ps1` script downloads every C++ dependency into the
`compile_deps/` folder (with SHA-256 verification against
`compile_deps/DEPENDENCIES.md`). CMake automatically picks them up from
there via `compile_deps/cmake/LegionCoreDeps.cmake` — **you do not need to
set `BOOST_ROOT`, `OPENSSL_ROOT_DIR`, or `MYSQL_ROOT_DIR` yourself.**

The rest of this document is for users who want to use system-installed
libraries instead, or who want to understand the toolchain in detail.

---

## 1. Required toolchain (one-time setup)

| Component | Minimum version | Where to get it |
|---|---|---|
| Windows | Windows 10 1809 (build 17763) or Windows 11 | — |
| Visual Studio | **2022 17.0+** (toolset v143) | <https://visualstudio.microsoft.com/downloads/> — Community edition is fine |
| Workload | "Desktop development with C++" + **MSVC v143 - VS 2022 C++ x64/x86 build tools** + **Windows 11 SDK** | inside the VS Installer |
| CMake | **4.3.2** (the floor declared in `CMakeLists.txt` is 3.18, but this build was verified against 4.3.2) | <https://cmake.org/download/> — pick the Windows x64 Installer and tick "Add CMake to PATH" |
| Git | any recent | <https://git-scm.com/download/win> |
| Boost | **1.85.0+** (1.85, 1.86, 1.87, 1.88 all OK) | prebuilt binaries: <https://sourceforge.net/projects/boost/files/boost-binaries/> → pick `boost_1_85_0-msvc-14.3-64.exe` (or newer) |
| MariaDB | **10.6.3+** (10.6 LTS or 10.11 LTS recommended) | <https://mariadb.org/download/> — pick "Windows x64 MSI" |
| OpenSSL | 3.0+ (3.5 ships with everything you need) | <https://slproweb.com/products/Win32OpenSSL.html> → "Win64 OpenSSL v3.x" (NOT the Light edition) |

After installing Boost, set the environment variable so CMake finds it:

```powershell
[Environment]::SetEnvironmentVariable('BOOST_ROOT', 'C:\local\boost_1_85_0', 'User')
```

(Adjust the path to wherever the Boost installer put the files.)

---

## 2. Cloning

```powershell
git clone <your fork URL> LegionCore
cd LegionCore
git checkout merged-reforged    # or main, whichever you adopted
```

---

## 3. Configuring with the supplied preset (easiest)

This repo ships a `CMakePresets.json` with a ready-made Windows preset:

```powershell
cmake --preset windows-msvc-release
```

That command:
* Generates a **Visual Studio 17 2022** solution targeting **x64**
* Places it in `build/windows-msvc-release/`
* Pre-fills `BOOST_ROOT`, `MYSQL_ROOT_DIR`, `OPENSSL_ROOT_DIR` from environment if set, otherwise from the defaults baked into the preset
* Turns on `TOOLS=1` so map/vmap/mmap extractors are built alongside the server

If your install paths differ from the preset defaults, override them on the command line:

```powershell
cmake --preset windows-msvc-release `
      -DBOOST_ROOT="D:\boost_1_88_0" `
      -DMYSQL_ROOT_DIR="D:\Programs\MariaDB 10.11" `
      -DOPENSSL_ROOT_DIR="D:\Programs\OpenSSL-Win64"
```

---

## 3-alt. Configuring without the preset (classic way)

```powershell
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 `
      -DTOOLS=1 `
      -DCMAKE_INSTALL_PREFIX="C:/LegionCore" `
      -DBOOST_ROOT="C:/local/boost_1_85_0" `
      -DMYSQL_ROOT_DIR="C:/Program Files/MariaDB 10.6" `
      -DOPENSSL_ROOT_DIR="C:/Program Files/OpenSSL-Win64" `
      ..
```

You should see lines like:

```
-- Detected MySQL/MariaDB client version: 10.6.18
-- Found Boost: ... (found suitable version "1.85.0", minimum required is "1.85")
-- Configuring done
-- Generating done
```

If you see `MSVC: LegionCore requires version 19.30 ...` you do not have the
v143 toolset installed — open the Visual Studio Installer, modify your 2022
install, and enable "MSVC v143 - VS 2022 C++ x64/x86 build tools".

---

## 4. Building

```powershell
cmake --build build/windows-msvc-release --config RelWithDebInfo -j
```

or (if you used the preset):

```powershell
cmake --build --preset windows-msvc-release
```

or just open `build/windows-msvc-release/LegionCore.sln` in Visual Studio
and hit **Build Solution** (`Ctrl+Shift+B`). Expect 20–40 minutes on a fast
machine for the first build; subsequent incremental builds are seconds.

Outputs land in `build/<preset>/bin/<config>/`:

```
bnetserver.exe       # Battle.net auth server (Legion uses bnet, not the old "authserver")
worldserver.exe
mapextractor.exe     (if TOOLS=1)
vmap4extractor.exe   (if TOOLS=1)
vmap4assembler.exe   (if TOOLS=1)
mmaps_generator.exe  (if TOOLS=1)
connection_patcher.exe (if TOOLS=1) - patches a 7.3.5 client (build 26972) to connect to a custom realm
libmariadb.dll       # copied next to the .exes
```

> This is a **Legion 7.3.5 (build 26972)** core. Your WoW client must be exactly
> build 26972, and you must run `connection_patcher.exe` against `Wow.exe`
> (and the supporting DLLs) once before it will connect to a private realm.

---

## 5. Common Windows-only gotchas

1. **`cannot open file 'libmariadb.lib'`** — MariaDB's MSI installer hides the
   `C` connector behind an optional "Development Components" feature.
   Re-run the MSI in **Modify** mode and tick it.

2. **`The Boost C++ Libraries were not found`** — the Boost prebuilt installer
   doesn't set `BOOST_ROOT` for you. Either set it as an env-var (see §1) or
   pass `-DBOOST_ROOT=...` to CMake. Also note: Boost prebuilts ship as
   `boost_1_85_0-msvc-14.3-64.exe` — the `-14.3` part **must** match VS 2022.
   If you have VS 2019, download the `-14.2` variant.

3. **Linker error about `_WIN32_WINNT < 0x0A00`** — you have an old Windows
   SDK selected. Open `Project → Properties → General → Windows SDK Version`
   in Visual Studio and pick anything ≥ 10.0.17763.

4. **`fatal error C1189: WinSock2.h has already been included before windows.h`** —
   this can happen if your own additions include `windows.h` before any Boost
   networking header. Use `#include <boost/asio.hpp>` (or any Boost.Asio
   header) **first** in the affected source file.

5. **OpenSSL 3 vs 1.1** — the build accepts both, but **MariaDB 10.6+ on
   Windows is linked against OpenSSL 3**, so mixing 1.1.x will produce subtle
   TLS failures at runtime. Use OpenSSL 3.

6. **DLL hell on launch** — `worldserver.exe` needs `libmariadb.dll`,
   `libssl-3-x64.dll`, and `libcrypto-3-x64.dll` next to it (or on PATH).
   Either install them to the same dir or add `C:\Program Files\MariaDB
   10.6\lib` and `C:\Program Files\OpenSSL-Win64\bin` to your system PATH.

---

## 6. Verifying versions at runtime

```powershell
worldserver.exe --version
```

should print the LegionCore revision plus the linked-against
Boost, OpenSSL, and MariaDB client versions. If any of those are below the
floor declared in CMake, **rebuild** — don't try to swap DLLs.

---

## 7. CI / scripted builds

```powershell
# One-liner that works in a fresh "Developer PowerShell for VS 2022"
git clone <repo> && cd LegionCore
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
```

Add the corresponding install step:

```powershell
cmake --install build/windows-msvc-release --config RelWithDebInfo --prefix C:/LegionCore
```
