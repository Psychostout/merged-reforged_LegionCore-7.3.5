# compile_deps/ — vendored & fetched build dependencies

This directory exists so that, after running the appropriate setup script
**once**, you can build LegionCore on a clean machine with zero
system-installed C++ libraries (no `BOOST_ROOT`, no `MYSQL_ROOT_DIR`, no
`OPENSSL_ROOT_DIR` env-vars needed). The top-level `CMakeLists.txt` is
wired to look here first.

```
compile_deps/
├── README.md                  ← you are here
├── DEPENDENCIES.md            ← manifest: every URL, version, and SHA-256
├── setup_deps.ps1             ← Windows setup script (run once)
├── setup_deps.sh              ← Linux/macOS setup script (run once)
├── cmake/
│   └── LegionCoreDeps.cmake   ← auto-included by the top-level CMakeLists
├── mariadb/
│   ├── source/                ← MariaDB Connector/C 3.4.8 source (vendored, ✓)
│   ├── windows-x64/           ← optional drop-in prebuilt connector location
│   └── linux-x86_64/          ← optional drop-in prebuilt connector location
├── boost/                     ← Boost 1.83 (populated by setup script)
├── openssl/                   ← OpenSSL 3.x (populated by setup script)
└── downloads/                 ← raw archive cache (vendored sha256s, ✓)
```

## Why most binaries are downloaded, not committed

Boost (~500 MB unpacked) and OpenSSL Windows binaries are simply too large
to ship in-tree without making the repo painful to clone. The setup scripts
download them from each project's **official site** and verify the
SHA-256 against the manifest in `DEPENDENCIES.md`. Nothing is fetched from
unverified mirrors.

The MariaDB Connector/C **source** (6 MB) is small enough to vendor
verbatim, and CMake will build it as a sub-project if you do not have a
prebuilt connector. This guarantees a working DB layer on any clean
machine. The `windows-x64/` and `linux-x86_64/` folders are optional
drop-in override locations; they are not required for the default flow.

## Quick start

### Windows
```powershell
cd compile_deps
.\setup_deps.ps1            # downloads + verifies + extracts everything
cd ..
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
```

### Linux
```bash
cd compile_deps
./setup_deps.sh             # downloads + verifies + extracts everything
cd ..
cmake --preset linux-gcc-release
cmake --build --preset linux-gcc-release
```

After `setup_deps`, the tree is fully self-contained — no env-vars needed.

## CMake integration

`CMakeLists.txt` automatically does:

```cmake
include(compile_deps/cmake/LegionCoreDeps.cmake OPTIONAL)
```

`LegionCoreDeps.cmake` sets `BOOST_ROOT`, `MYSQL_ROOT_DIR`, `OPENSSL_ROOT_DIR`
to point inside `compile_deps/` **only if those variables are not already set
by the user**, so external `-D...` overrides still win.


## After building

After the server binaries are built, see `../docs/PORTABLE_SERVER_SETUP.md` for a
recommended standalone folder layout, config placement, MariaDB setup, database
user creation, SQL import order, and startup scripts.

For a complete dependency/toolchain checklist, see `../docs/DEPENDENCY_SETUP.md`.
