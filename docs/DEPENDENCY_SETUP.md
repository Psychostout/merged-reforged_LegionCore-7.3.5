# Dependency Setup Guide

This guide explains what a user must install, what `compile_deps/` downloads, and where everything should be placed so CMake finds it automatically.

Recommended rule: **use `compile_deps/` unless you know you want system-installed libraries.**

---

## 1. What `compile_deps/` provides

After running the setup script, CMake is intended to find dependencies here:

```text
compile_deps/
├── boost/
│   ├── boost_1_83_0/       # Windows prebuilt Boost root
│   └── install/            # Linux/source-built Boost install prefix
├── openssl/
│   ├── OpenSSL-Win64/      # Windows OpenSSL install root
│   └── install/            # Linux/source-built OpenSSL install prefix
└── mariadb/
    └── source/             # vendored MariaDB Connector/C source
```

CMake integration is automatic through:

```text
compile_deps/cmake/LegionCoreDeps.cmake
```

The top-level `CMakeLists.txt` includes that file. It sets these only if the user has not already provided them:

```text
BOOST_ROOT
MYSQL_ROOT_DIR
OPENSSL_ROOT_DIR
```

Lookup order:

```text
1. Explicit -DBOOST_ROOT / -DMYSQL_ROOT_DIR / -DOPENSSL_ROOT_DIR
2. Real environment variables set by the user
3. compile_deps/ auto-detection
4. Normal system package discovery
```

---

## 2. Required toolchain — Windows

Install these manually first:

| Tool | Required version / notes |
|---|---|
| Visual Studio | Visual Studio 2022 with MSVC v143 x64/x86 build tools |
| Windows SDK | Windows 10/11 SDK, at least 10.0.17763 |
| CMake | 3.18 minimum; 4.3.2 recommended/targeted by docs |
| Git | Recent Git for Windows |
| PowerShell | Windows PowerShell 5.1+ or PowerShell 7+ |

Then run:

```powershell
cd compile_deps
.\setup_deps.ps1
cd ..
cmake --preset windows-msvc-release
```

The setup script places dependencies as follows:

```text
Boost   -> compile_deps/boost/boost_1_83_0/
OpenSSL -> compile_deps/openssl/OpenSSL-Win64/
MariaDB -> compile_deps/mariadb/source/ is already vendored; CMake builds Connector/C as needed
```

You should **not** need to set `BOOST_ROOT`, `MYSQL_ROOT_DIR`, or `OPENSSL_ROOT_DIR` when using the default `compile_deps` flow.

If you manually install dependencies somewhere else, override paths like this:

```powershell
cmake --preset windows-msvc-release `
  -DBOOST_ROOT="D:\boost_1_83_0" `
  -DOPENSSL_ROOT_DIR="D:\OpenSSL-Win64" `
  -DMYSQL_ROOT_DIR="D:\MariaDBConnector"
```

---

## 3. Required toolchain — Linux Debian/Ubuntu style

Install the build tools first:

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  ninja-build \
  git \
  curl \
  tar \
  xz-utils \
  bzip2 \
  unzip \
  perl \
  pkg-config \
  zlib1g-dev \
  libbz2-dev \
  libreadline-dev
```

Then either use system Boost 1.83 if your distro provides it:

```bash
sudo apt-get install -y libboost1.83-all-dev
```

or let `compile_deps` fetch/build Boost:

```bash
cd compile_deps
./setup_deps.sh
cd ..
cmake --preset linux-gcc-release
```

The setup script places dependencies as follows:

```text
Boost   -> compile_deps/boost/install/
OpenSSL -> compile_deps/openssl/install/ if system OpenSSL >= 3.0 is not found
MariaDB -> compile_deps/mariadb/source/ is already vendored; CMake builds Connector/C as needed
```

If your Linux distro already provides good system packages, CMake may use those instead. For the verified bnetserver path, Boost 1.83 is the known-good version.

---

## 4. Boost requirement

Current supported Boost version:

```text
Boost 1.83
```

Why 1.83?

`bnetserver` currently includes:

```cpp
#include <boost/asio/io_service.hpp>
```

Boost 1.83 provides this header. Boost 1.88 does not. The targeted `bnetserver` build succeeded with Boost 1.83.

If you want to use Boost 1.85+ later, first modernize the bnetserver Boost.Asio include/code and retest.

---

## 5. MariaDB requirement

Runtime/database server recommendation:

```text
MariaDB 10.6.3+ or 10.11 LTS
```

Build-time connector:

```text
compile_deps/mariadb/source/  # vendored MariaDB Connector/C 3.4.8 source
```

You do not normally need to place anything into `compile_deps/mariadb/windows-x64` or `compile_deps/mariadb/linux-x86_64`. Those are optional drop-in override folders for advanced users who already have a prebuilt Connector/C package.

---

## 6. OpenSSL requirement

Recommended:

```text
OpenSSL 3.x, preferably 3.5 LTS
```

Windows setup script:

```text
compile_deps/openssl/OpenSSL-Win64/
```

Linux setup script:

```text
Uses system OpenSSL >= 3.0 if available;
otherwise builds into compile_deps/openssl/install/
```

---

## 7. How to confirm CMake sees compile_deps

When configuring, look for messages like:

```text
LegionCoreDeps: scanning .../compile_deps
LegionCoreDeps: BOOST_ROOT = .../compile_deps/boost/...
LegionCoreDeps: OPENSSL_ROOT_DIR = .../compile_deps/openssl/...
LegionCoreDeps: MYSQL_ROOT_DIR = .../_deps/mariadb-connector-c-install
```

If CMake finds a wrong system dependency, delete the build directory and reconfigure:

```bash
rm -rf build/linux-gcc-release
cmake --preset linux-gcc-release
```

On Windows, remove the preset build folder:

```powershell
Remove-Item -Recurse -Force build\windows-msvc-release
cmake --preset windows-msvc-release
```

---

## 8. Common mistakes

### Setting BOOST_ROOT globally to the wrong version

If `BOOST_ROOT` points to Boost 1.85/1.88, CMake may ignore `compile_deps` and use the wrong Boost. Remove the environment variable or pass the correct override.

### Running CMake before setup_deps

Run setup first:

```bash
./compile_deps/setup_deps.sh
```

or:

```powershell
.\compile_deps\setup_deps.ps1
```

### Reusing an old build folder

CMake caches dependency paths. If you change dependencies, delete the build folder before configuring again.

### Confusing `dep/` and `compile_deps/`

`dep/` is part of the source tree and contains vendored libraries/CMake wrappers. `compile_deps/` is the local dependency staging/download folder used to find external build requirements.
