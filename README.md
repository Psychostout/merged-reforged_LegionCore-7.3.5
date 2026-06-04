# LegionCore 7.3.5 — Reforged + Modernized

!!!!IMPORTANT REMINDER: CURRENTLY UNDER DEVOLEPMENT BY A COMPLETE NOOB WITH A INTERNET CONNECTION!!!!

[![Status: bnetserver builds](https://img.shields.io/badge/bnetserver-builds-green)](#build-status)
[![CMake](https://img.shields.io/badge/CMake-3.18…4.3.2-blue)](#requirements)
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue)](#requirements)
[![Boost](https://img.shields.io/badge/Boost-1.83-orange)](#requirements)
[![OpenSSL](https://img.shields.io/badge/OpenSSL-3.5%20LTS-orange)](#requirements)
[![MariaDB](https://img.shields.io/badge/MariaDB-10.6.3+-orange)](#requirements)
[![Client](https://img.shields.io/badge/Legion-7.3.5%20%2826972%29-purple)](#what-this-is)

A modernized integration of [Psychostout/LegionCore-Reforged](https://github.com/Psychostout/LegionCore-Reforged)
on top of [The-Legion-Preservation-Project/LegionCore-7.3.5](https://github.com/The-Legion-Preservation-Project/LegionCore-7.3.5),
with a refreshed C++20 toolchain, a vendored/auto-fetched dependency tree, restored
SoloCraft, and an English-translated config file.

---

## What this is

A **World of Warcraft: Legion (build 26972)** private-server emulator, descended from
the **UWOW core leak of 2020 → TrinityCore**. Default `Game.Build.Version` is
**26972** (final retail 7.3.5), `CURRENT_EXPANSION` is `EXPANSION_LEGION`.

This is *not* a binary release — you compile it yourself from this repository
against your own MariaDB and (eventually) feed it the data files extracted from a
real Legion 7.3.5 WoW client.

---

## Highlights of this branch

| Area | Change |
|---|---|
| **Merge** | 64 commits from `Psychostout/LegionCore-Reforged` integrated verbatim on top of `The-Legion-Preservation-Project/LegionCore-7.3.5`. Zero text conflicts (Reforged was branched from the exact current tip of upstream main). |
| **CMake** | `cmake_minimum_required(VERSION 3.18…4.3.2)` — supports the brand-new CMake 4.3 series with all modern policies on, while keeping the existing 3.18 floor. |
| **Boost** | Floor set to **1.83** on both Linux and Windows. This is the verified version for the current `bnetserver` build path; `StartProcess.cpp` still contains compatibility for Boost 1.86+ `boost/process/v1/*` headers. |
| **OpenSSL** | Floor raised to **3.5 LTS** (current Long-Term-Support series, supported through April 2030). Built-source path uses `no-tests no-docs no-shared`. |
| **MariaDB** | Floor raised to **10.6.3** (LTS). `FindMySQL.cmake` now actually *probes* the connector version and warns at configure time if you're below it. Windows registry hints expanded to 10.6 / 10.11. |
| **Windows toolchain** | MSVC floor raised to 19.30 (VS 2022 v143 — required by the project's C++20). Added `/Zc:__cplusplus /Zc:preprocessor /utf-8 /EHsc /permissive-`. `_WIN32_WINNT` bumped from `0x0601` (Win 7, EOL) to `0x0A00` (Win 10 1809+). |
| **`compile_deps/` folder** | Vendored MariaDB Connector/C 3.4.8 source; `setup_deps.{ps1,sh}` scripts that download + SHA-256-verify Boost & OpenSSL from official sources; CMake glue that auto-points `BOOST_ROOT`/`MYSQL_ROOT_DIR`/`OPENSSL_ROOT_DIR` into the folder. **Zero-env-var build flow.** |
| **`CMakePresets.json`** | Ready-made `windows-msvc-release`, `windows-msvc-debug`, `linux-gcc-release` presets so contributors don't have to memorise flags. |
| **SoloCraft restored** | `src/server/scripts/Custom/Solocraft.cpp` + `custom_script_loader.cpp` byte-restored from upstream; ships disabled (`Solocraft.Enable = 0`); 244 config keys documented in `worldserver.conf.dist`. |
| **Config in English** | `worldserver.conf.dist` translated French → English; the current live config contains **479 setting lines** and settings live on non-comment lines so they are easy to preserve during comment cleanup. |
| **`docs/BUILD_WINDOWS.md`** | Step-by-step Windows build guide with VS 2022 setup, troubleshooting for the 7 most common Windows-only build failures. |

See the user documentation list below for setup, Windows builds, dependency setup, and portable server layout.

---


## User documentation

Start here if you are building or running the server:

| Document | Purpose |
|---|---|
| [`docs/setup_guide.md`](docs/setup_guide.md) | Short step-by-step setup index. |
| [`docs/BUILD_WINDOWS.md`](docs/BUILD_WINDOWS.md) | Windows / Visual Studio 2022 build guide. |
| [`docs/DEPENDENCY_SETUP.md`](docs/DEPENDENCY_SETUP.md) | Required tools/dependencies and where `compile_deps` places them. |
| [`docs/PORTABLE_SERVER_SETUP.md`](docs/PORTABLE_SERVER_SETUP.md) | Standalone portable server folder, MariaDB, configs, SQL import, startup. |
| [`compile_deps/README.md`](compile_deps/README.md) | How to fetch/build third-party dependencies. |
| [`compile_deps/DEPENDENCIES.md`](compile_deps/DEPENDENCIES.md) | Dependency versions, URLs, and SHA-256 checksums. |
| [`docs/README.md`](docs/README.md) | Documentation index. |

Maintainer-only development notes, old transcript summaries, translation work,
and build logs are stored separately under `Dev_referance/` and are not needed
for normal setup.

---

## Quick start

### Linux (Debian/Ubuntu/Fedora)

```bash
# 1. Clone
git clone <your fork URL> LegionCore && cd LegionCore

# 2. System prereqs (Debian 13 example)
sudo apt-get install -y build-essential cmake git ninja-build \
    libboost1.83-all-dev \
    libssl-dev libmariadb-dev zlib1g-dev libbz2-dev libreadline-dev

# (Or skip system Boost/OpenSSL/MariaDB and let compile_deps/setup_deps.sh fetch them)

# 3. Configure + build
cmake --preset linux-gcc-release
cmake --build --preset linux-gcc-release -j$(nproc)
```

### Windows (VS 2022)

```powershell
# 1. Clone
git clone <your fork URL> LegionCore
cd LegionCore

# 2. Fetch dependencies (one-time, ~15-20 min)
.\compile_deps\setup_deps.ps1

# 3. Configure + build
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
```

See [`docs/BUILD_WINDOWS.md`](docs/BUILD_WINDOWS.md) for the full Windows build guide.
After compiling, see [`docs/PORTABLE_SERVER_SETUP.md`](docs/PORTABLE_SERVER_SETUP.md) for
standalone folder layout, configs, MariaDB setup, SQL imports, and startup order.

---

## Requirements

| Component | Minimum | Tested with |
|---|---|---|
| **CMake** | 3.18 | 4.3.2 |
| **C++ standard** | C++20 | – |
| **GCC** (Linux) | 8 | 14.2.0 |
| **Clang** (Linux/macOS) | 7 | – |
| **MSVC** (Windows) | 19.30 (VS 2022 v143) | – |
| **Windows** | 10 1809 (build 17763) | – |
| **Boost** | 1.83 | 1.83 |
| **OpenSSL** | 3.0 (3.5 LTS recommended) | 3.5.6 |
| **MariaDB** | 10.6.3 LTS | 11.8.6 |
| **MySQL** alternative | 8.0+ | – |
| **ZLib** | any recent | 1.3.1 |
| **Boost.Process** | v1 API (uses `boost/process/v1/` paths on Boost 1.86+) | – |

---

## Build status

### Verified targeted build

✅ `bnetserver` builds successfully in the Linux test environment when using
**Boost 1.83** and a targeted build:

```bash
cmake -S . -B build/bnetserver-boost183 -G Ninja \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DTOOLS=0 \
      -DSCRIPTS=none
cmake --build build/bnetserver-boost183 --target bnetserver -- -j1
```

The successful test produced:

```text
build/bnetserver-boost183/src/server/bnetserver/bnetserver
```

The linked binary used Boost 1.83 libraries:

```text
libboost_filesystem.so.1.83.0
libboost_program_options.so.1.83.0
libboost_thread.so.1.83.0
```

### What currently builds cleanly in the targeted path

✅ `dep/*` required by `bnetserver`
✅ `common`
✅ `database`
✅ `proto` including the committed BNet `Client/*.pb.{h,cc}` descriptors
✅ `shared`
✅ `bnetserver`

### Important Boost note

Boost **1.83** is the supported floor for this branch because the current
`bnetserver` REST code includes the legacy Boost.Asio header:

```cpp
#include <boost/asio/io_service.hpp>
```

Boost 1.83 provides that header. Boost 1.88 does not, which is why the earlier
Boost 1.88 test failed before compiling `bnetserver`.

### What is not yet verified

⚠️ A full `worldserver` / all-scripts build has not been validated in this
recovery pass. The successful test was intentionally limited to `bnetserver`.

### BNet protobuf status

The previously missing BNet generated protobuf files are now present under:

```text
src/server/proto/Client/
```

`src/server/proto/CMakeLists.txt` includes that directory in the `proto` target,
and `src/server/proto/PrecompiledHeaders/protoPCH.h` references the committed
service/type descriptors.

---

## Repository layout

```
LegionCore-7.3.5/
├── CMakeLists.txt              Top-level build (CMake 3.18…4.3.2)
├── CMakePresets.json           windows-msvc-release / debug, linux-gcc-release
├── README.md                   GitHub front page
├── docs/                       Detailed project documentation
│   ├── setup_guide.md          Short step-by-step index
│   ├── BUILD_WINDOWS.md        Windows build guide
│   ├── DEPENDENCY_SETUP.md      Dependency/toolchain setup guide
│   └── PORTABLE_SERVER_SETUP.md Standalone server layout/deployment guide
├── Dev_referance/              Maintainer-only references (not needed for normal setup)
├── cmake/                      Find* macros, compiler/platform settings
├── dep/                        Vendored 3rd-party dependencies (CascLib, fmt, …)
├── compile_deps/               Build-time dependency manager (see its own README.md)
│   ├── setup_deps.{ps1,sh}     SHA-verified download + build scripts
│   ├── DEPENDENCIES.md         Manifest of every URL + SHA-256
│   ├── cmake/LegionCoreDeps.cmake   Auto-included; sets BOOST_ROOT etc.
│   ├── mariadb/source/         Vendored MariaDB Connector/C 3.4.8 source (6.1 MB)
│   ├── boost/                  Populated by setup_deps
│   └── openssl/                Populated by setup_deps (Linux skips if system OpenSSL>=3)
├── src/
│   ├── common/                 Logging, threading, crypto, utilities
│   ├── server/
│   │   ├── bnetserver/         Battle.net auth server
│   │   ├── worldserver/        Game world server
│   │   ├── database/           DB abstraction layer
│   │   ├── proto/              protobuf message definitions
│   │   ├── shared/             Shared between bnetserver / worldserver
│   │   ├── game/               Game logic (entities, spells, AI, …)
│   │   └── scripts/            Pluggable script modules
│   │       ├── BattlePay/      Reforged: WoW Token, character boost 90, class trial
│   │       ├── BrawlersGuild/  Reforged: null-pointer crash fixes
│   │       ├── Commands/       GM commands incl. .gps, .legionassault
│   │       ├── Custom/         SoloCraft (restored, ships disabled)
│   │       ├── Draenor/        Reforged: Skyreach beam event, Iron Demolisher fix
│   │       ├── EasternKingdoms/
│   │       ├── Kalimdor/
│   │       ├── Legion/         Reforged: Demon Invasion Pre-Patch (phases 1-4)
│   │       ├── … (Maelstrom, Northrend, OutdoorPvP, Outland, Pandaria, Scenario, Spells, World)
│   │       └── ScriptLoader.cpp.in.cmake   Auto-discovers modules via file(GLOB)
│   └── tools/                  Map/vmap/mmap extractors, connection_patcher
├── sql/                        Schema + update SQL files
└── revision_data.h.in.cmake    Embeds git SHA into the binary
```

---

## Configuration

Two config files (both English):

* **`worldserver.conf.dist`** — 2,881 lines, 479 settings, every category from
  game rules to network buffers to AHBot tuning. Copy to `worldserver.conf` and
  edit. Defaults are sensible Legion-blizzlike with some quality-of-life
  changes (talents free, world quests on, durability loss ≈ 0).
* **`bnetserver.conf.dist`** — Battle.net auth server config. Was already
  English in the source.

Key knobs you'll touch first:

```ini
# Database (4 connection strings)
LoginDatabaseInfo     = "127.0.0.1;3306;root;admin;auth"
CharacterDatabaseInfo = "127.0.0.1;3306;root;admin;characters"
HotfixDatabaseInfo    = "127.0.0.1;3306;root;admin;hotfixes"
WorldDatabaseInfo     = "127.0.0.1;3306;root;admin;world"

# Where extracted client data lives
DataDir = "./ClientData"

# Network
WorldServerPort     = 8085
InstanceServerPort  = 8086

# SoloCraft (custom — opt in)
Solocraft.Enable = 0   # set to 1 to enable scaling in dungeons/raids
```

---

## Contributing

Patches are welcome. Please:

1. Open an issue first for non-trivial changes.
2. Keep commits focused — one logical change per commit.
3. Don't reformat unrelated code.
4. If your change touches the build system, run `cmake --preset linux-gcc-release`
   end-to-end and report the result.
5. Conform to the existing TrinityCore-style naming.

The most useful contribution right now would be validating and fixing the full `worldserver` / scripts build after the now-verified `bnetserver` target.

---

## License

GPL-2.0-or-later (inherited from TrinityCore — see `COPYING`).

The vendored MariaDB Connector/C in `compile_deps/mariadb/source/` is LGPL-2.1
(see `compile_deps/mariadb/source/COPYING.LIB`).

---

## Credits

* **TrinityCore** — the upstream lineage
* **UWOW** — original Legion 7.3.5 server work (2020 leak)
* **The Legion Preservation Project** — kept the source alive
* **Psychostout (LegionCore-Reforged)** — many fixes and the toolchain modernisation foundations
* **Boost, OpenSSL, MariaDB** — the standing-on-shoulders dependencies
