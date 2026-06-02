# LegionCore 7.3.5 — Reforged + Modernized

!!!!IMPORTANT REMINDER: CURRENTLY UNDER DEVOLEPMENT BY A COMPLETE NOOB WITH A INTERNET CONNECTION!!!!

[![Status: build-system clean • bnetserver wip](https://img.shields.io/badge/build--system-clean-green)](#build-status)
[![CMake](https://img.shields.io/badge/CMake-3.18…4.3.2-blue)](#requirements)
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue)](#requirements)
[![Boost](https://img.shields.io/badge/Boost-1.85+-orange)](#requirements)
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
| **Boost** | Floor raised to **1.85** on both Linux and Windows. Compat fix in `StartProcess.cpp` for Boost 1.86+ where `boost/process/*.hpp` headers moved to `boost/process/v1/*.hpp`. |
| **OpenSSL** | Floor raised to **3.5 LTS** (current Long-Term-Support series, supported through April 2030). Built-source path uses `no-tests no-docs no-shared`. |
| **MariaDB** | Floor raised to **10.6.3** (LTS). `FindMySQL.cmake` now actually *probes* the connector version and warns at configure time if you're below it. Windows registry hints expanded to 10.6 / 10.11. |
| **Windows toolchain** | MSVC floor raised to 19.30 (VS 2022 v143 — required by the project's C++20). Added `/Zc:__cplusplus /Zc:preprocessor /utf-8 /EHsc /permissive-`. `_WIN32_WINNT` bumped from `0x0601` (Win 7, EOL) to `0x0A00` (Win 10 1809+). |
| **`compile_deps/` folder** | Vendored MariaDB Connector/C 3.4.8 source; `setup_deps.{ps1,sh}` scripts that download + SHA-256-verify Boost & OpenSSL from official sources; CMake glue that auto-points `BOOST_ROOT`/`MYSQL_ROOT_DIR`/`OPENSSL_ROOT_DIR` into the folder. **Zero-env-var build flow.** |
| **`CMakePresets.json`** | Ready-made `windows-msvc-release`, `windows-msvc-debug`, `linux-gcc-release` presets so contributors don't have to memorise flags. |
| **SoloCraft restored** | `src/server/scripts/Custom/Solocraft.cpp` + `custom_script_loader.cpp` byte-restored from upstream; ships disabled (`Solocraft.Enable = 0`); 244 config keys documented in `worldserver.conf.dist`. |
| **Config in English** | `worldserver.conf.dist` translated French → English while keeping every one of the **474 setting lines byte-identical** (settings live on non-comment lines so they're never touched). |
| **`BUILD_WINDOWS.md`** | Step-by-step Windows build guide with VS 2022 setup, troubleshooting for the 7 most common Windows-only build failures. |

See `CHANGELOG.txt` for the full per-commit list with rationales.

---

## Quick start

### Linux (Debian/Ubuntu/Fedora)

```bash
# 1. Clone
git clone <your fork URL> LegionCore && cd LegionCore

# 2. System prereqs (Debian 13 example)
sudo apt-get install -y build-essential cmake git ninja-build \
    libboost-all-dev libboost-charconv1.88-dev \
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

See [`BUILD_WINDOWS.md`](BUILD_WINDOWS.md) for the full Windows guide including
required Visual Studio components, common gotchas, and DLL deployment.

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
| **Boost** | 1.85 | 1.88 |
| **OpenSSL** | 3.0 (3.5 LTS recommended) | 3.5.5 |
| **MariaDB** | 10.6.3 LTS | 11.8.6 |
| **MySQL** alternative | 8.0+ | – |
| **ZLib** | any recent | 1.3.1 |
| **Boost.Process** | v1 API (uses `boost/process/v1/` paths on Boost 1.86+) | – |

---

## Build status

### What currently builds cleanly

✅ `dep/*` — all vendored deps (CascLib, fmt, g3dlite, gsoap, protobuf, rapidjson,
  recastnavigation, SFMT, utf8cpp, zlib, plus the boost/openssl/mysql/readline wrappers)
✅ `common` (`libcommon.a` ~3.7 MB, 100+ source files)
✅ `database` (`libdatabase.a`)
✅ `proto` (`libproto.a` — Login + RealmList descriptors)

Verified end-to-end with **CMake 4.3.2 + GCC 14.2.0 + Boost 1.88 + OpenSSL 3.5.5 +
MariaDB 11.8.6** on Debian 13.

### What does NOT yet build

❌ `shared` — blocked on missing BNet proto descriptors (see below)
❌ `scripts` / `bnetserver` / `worldserver` — transitively blocked on `shared`

### Known pre-existing limitation (BNet protos)

The upstream `Psychostout/LegionCore-Reforged` fork is missing **13 generated
Battle.net `.pb.h`/`.pb.cc` files** (account_service, authentication_service,
challenge_service, channel_service, connection_service, friends_service,
game_utilities_service, notification_types, presence_service,
profanity_filter_config, report_service, resource_service, user_manager_service)
along with their `.proto` source files. These are referenced by:

  * `src/server/bnetserver/Services/AccountService.h`
  * `src/server/bnetserver/Services/AuthenticationService.h`
  * `src/server/bnetserver/Services/ConnectionService.h`
  * `src/server/game/Services/WorldserverService.h`
  * `src/server/game/Services/WorldserverServiceDispatcher.h`
  * `src/server/shared/Realm/RealmList.cpp`

So `shared` (and everything downstream) doesn't link. The most direct fix is to
obtain the missing `.proto` sources from upstream TrinityCore master and add a
`protobuf_generate` step to `src/server/proto/CMakeLists.txt`. That is **not in
scope for this branch** — it's a clean-room re-implementation of a non-trivial
piece of the BNet protocol stack.

Several mitigations have been applied to make sure the parts that *can* build
do so cleanly:

  * `src/server/proto/PrecompiledHeaders/protoPCH.h` now only references the
    two descriptors that exist (`Login.pb.h`, `RealmList.pb.h`).
  * `src/server/proto/CMakeLists.txt` excludes the broken `Client/` subdirectory
    from the proto build.
  * `src/server/game/DataStores/DB2Stores.h` got a missing
    `#include <unordered_set>` (was relying on a transitive include the PCH
    happened to provide).

---

## Repository layout

```
LegionCore-7.3.5/
├── CMakeLists.txt              Top-level build (CMake 3.18…4.3.2)
├── CMakePresets.json           windows-msvc-release / debug, linux-gcc-release
├── CHANGELOG.txt               Full history of merge + modernization commits
├── BUILD_WINDOWS.md            Windows build guide
├── README.md                   You are here
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

* **`worldserver.conf.dist`** — 2,881 lines, 474 settings, every category from
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

The single most useful contribution right now would be **restoring the missing
BNet protocol `.proto` files** from upstream TrinityCore master (see
[Known limitation](#known-pre-existing-limitation-bnet-protos)) and wiring
`protobuf_generate_cpp` into `src/server/proto/CMakeLists.txt`. That unblocks
`shared` and the full server build.

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
