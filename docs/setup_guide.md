# LegionCore Quick Setup Guide

Short version: follow each step in order; open the linked document when you need details.

| Step | Do this | Details |
|---|---|---|
| 1 | Clone the repo and use the `main` branch. | [README.md](../README.md) |
| 2 | Install the build toolchain: CMake, compiler, Git, Ninja/VS2022. | [BUILD_WINDOWS.md](BUILD_WINDOWS.md) for Windows; [README.md](../README.md) for Linux notes |
| 3 | Install/fetch dependencies with `compile_deps/setup_deps.ps1` on Windows or `compile_deps/setup_deps.sh` on Linux. | [DEPENDENCY_SETUP.md](DEPENDENCY_SETUP.md), [compile_deps/README.md](../compile_deps/README.md), [DEPENDENCIES.md](../compile_deps/DEPENDENCIES.md) |
| 4 | Configure with CMake presets. | [CMakePresets.json](../CMakePresets.json), [BUILD_WINDOWS.md](BUILD_WINDOWS.md) |
| 5 | For the currently verified path, build only `bnetserver` first. | [README.md](../README.md) build status |
| 6 | Build `worldserver` only after you explicitly decide to test the larger target. | Not yet verified in this recovery pass |
| 7 | Create a portable server folder with `bin/`, `configs/`, `data/ClientData/`, `database/mariadb/`, and `logs/`. | [PORTABLE_SERVER_SETUP.md](PORTABLE_SERVER_SETUP.md) |
| 8 | Copy compiled `bnetserver`/`worldserver` binaries and required DLLs/libs into the portable `bin/` folder. | [PORTABLE_SERVER_SETUP.md](PORTABLE_SERVER_SETUP.md) |
| 9 | Copy `.conf.dist` files to real `.conf` files under `configs/`. | [PORTABLE_SERVER_SETUP.md](PORTABLE_SERVER_SETUP.md) |
| 10 | Extract client data from a Legion 7.3.5 build 26972 client into `data/ClientData/`. | [PORTABLE_SERVER_SETUP.md](PORTABLE_SERVER_SETUP.md) |
| 11 | Initialize MariaDB and create `auth`, `characters`, `world`, and `hotfixes` databases. | [PORTABLE_SERVER_SETUP.md](PORTABLE_SERVER_SETUP.md) |
| 12 | Create a database user/password, grant permissions, and import SQL. | [PORTABLE_SERVER_SETUP.md](PORTABLE_SERVER_SETUP.md) |
| 13 | Update DB login strings and ports in `bnetserver.conf` and `worldserver.conf`. | [PORTABLE_SERVER_SETUP.md](PORTABLE_SERVER_SETUP.md) |
| 14 | Start MariaDB, then `bnetserver`, then `worldserver`. | [PORTABLE_SERVER_SETUP.md](PORTABLE_SERVER_SETUP.md) |
| 15 | Create a game account from the worldserver console. | [PORTABLE_SERVER_SETUP.md](PORTABLE_SERVER_SETUP.md) |

## Current verified build fact

`bnetserver` was successfully built on Linux with Boost 1.83 using a targeted build. A full `worldserver`/scripts build has not yet been verified in this recovery pass.

## Most important docs

- [README.md](../README.md) — GitHub front page / current status.
- [BUILD_WINDOWS.md](BUILD_WINDOWS.md) — Windows build instructions.
- [DEPENDENCY_SETUP.md](DEPENDENCY_SETUP.md) — required tools/dependencies and compile_deps placement.
- [PORTABLE_SERVER_SETUP.md](PORTABLE_SERVER_SETUP.md) — portable server folder, MariaDB, configs, SQL, and startup.
- [compile_deps/README.md](../compile_deps/README.md) — dependency setup.

## Maintainer-only references

Development history, the detailed changelog, translation workspace, and build logs live in [Dev_referance/](../Dev_referance/).
