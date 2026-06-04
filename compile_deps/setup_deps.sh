#!/usr/bin/env bash
# ------------------------------------------------------------------------------
# setup_deps.sh — Linux/macOS dependency setup for LegionCore
#
# Idempotent. After this finishes you can build with no env-vars set:
#   cmake --preset linux-gcc-release
#   cmake --build --preset linux-gcc-release
#
# Flags:
#   --force          re-download even if cached file's SHA matches
#   --skip-boost
#   --skip-openssl   (default on Linux if system libssl >= 3.0 is found)
#   --skip-mariadb
#   --legacy-boost   use the classic boost_1_83_0.tar.bz2 + b2 build
#                    (default = use the smaller CMake-friendly GitHub archive)
# ------------------------------------------------------------------------------
set -euo pipefail

# ----- locate ourselves regardless of CWD -----
DEPS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DOWNLOADS_DIR="$DEPS_DIR/downloads"
mkdir -p "$DOWNLOADS_DIR"
cd "$DEPS_DIR"

FORCE=0; SKIP_BOOST=0; SKIP_OPENSSL=0; SKIP_MARIADB=0; LEGACY_BOOST=0
for arg in "$@"; do
  case "$arg" in
    --force)         FORCE=1 ;;
    --skip-boost)    SKIP_BOOST=1 ;;
    --skip-openssl)  SKIP_OPENSSL=1 ;;
    --skip-mariadb)  SKIP_MARIADB=1 ;;
    --legacy-boost)  LEGACY_BOOST=1 ;;
    -h|--help)
      sed -n '1,20p' "$0"; exit 0 ;;
    *) echo "Unknown flag: $arg" >&2; exit 2 ;;
  esac
done

# Auto-skip OpenSSL on Linux if a sufficient system version is present.
if [[ $SKIP_OPENSSL -eq 0 ]] && command -v pkg-config >/dev/null 2>&1; then
  if pkg-config --atleast-version=3.0 libssl 2>/dev/null; then
    SYS_OPENSSL_VER="$(pkg-config --modversion libssl)"
    echo "System OpenSSL $SYS_OPENSSL_VER detected (>= 3.0) — will use system OpenSSL."
    SKIP_OPENSSL=1
  fi
fi

# ----- Manifest (must match compile_deps/DEPENDENCIES.md) ----------------------
BOOST_VERSION="1.83.0"
# CMake-friendly GitHub archive (default — smaller download, faster build)
BOOST_MODULAR_FILE="boost-1.83.0.tar.xz"
BOOST_MODULAR_URL="https://github.com/boostorg/boost/releases/download/boost-${BOOST_VERSION}/${BOOST_MODULAR_FILE}"
BOOST_MODULAR_SHA256="c5a0688e1f0c05f354bbd0b32244d36085d9ffc9f932e8a18983a9908096f614"
BOOST_MODULAR_DIR="boost-1.83.0"
# Legacy non-modular archive
BOOST_LEGACY_UNDERSCORED="boost_1_83_0"
BOOST_LEGACY_FILE="${BOOST_LEGACY_UNDERSCORED}.tar.bz2"
BOOST_LEGACY_URL_PRIMARY="https://archives.boost.io/release/${BOOST_VERSION}/source/${BOOST_LEGACY_FILE}"
BOOST_LEGACY_URL_FALLBACK="https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VERSION}/source/${BOOST_LEGACY_FILE}"
BOOST_LEGACY_SHA256="79e6d3f986444e5a80afbeccdaf2d1c1cf964baa8d766d20859d653a16c39848"

OPENSSL_VERSION="3.5.0"
OPENSSL_FILE="openssl-${OPENSSL_VERSION}.tar.gz"
OPENSSL_URL="https://github.com/openssl/openssl/releases/download/openssl-${OPENSSL_VERSION}/${OPENSSL_FILE}"
OPENSSL_SHA256="344d0a79f1a9b08029b0744e2cc401a43f9c90acd1044d09a530b4885a8e9fc0"

MARIADB_VERSION="3.4.8"
MARIADB_FILE="mariadb-connector-c-${MARIADB_VERSION}-src.zip"
MARIADB_URL="https://archive.mariadb.org/connector-c-${MARIADB_VERSION}/${MARIADB_FILE}"
MARIADB_SHA256="1d774cff8150b243a1f620d46d5eec69a3eff6be6133bbe374d9c02625416bff"

# ----- helpers ---------------------------------------------------------------
sha256_check() {
  local path="$1" expected="$2"
  [[ -z "$expected" ]] && return 0
  [[ -f "$path" ]] || return 1
  local actual
  actual="$(sha256sum "$path" | awk '{print $1}')"
  [[ "$actual" == "$expected" ]]
}

fetch_verified() {
  local url="$1" url2="${2:-}" dest="$3" sha="$4" name="$5"
  if [[ $FORCE -eq 0 ]] && sha256_check "$dest" "$sha"; then
    echo "  [cache] $name already present, SHA OK"
    return 0
  fi
  echo "  [download] $name"
  echo "    from: $url"
  echo "    to:   $dest"
  if ! curl -fL --retry 3 --retry-delay 2 -o "$dest" "$url"; then
    if [[ -n "$url2" ]]; then
      echo "    primary failed, trying fallback: $url2"
      curl -fL --retry 3 --retry-delay 2 -o "$dest" "$url2"
    else
      return 1
    fi
  fi
  if [[ -n "$sha" ]]; then
    if ! sha256_check "$dest" "$sha"; then
      local got; got="$(sha256sum "$dest" | awk '{print $1}')"
      rm -f "$dest"
      echo "ERROR: SHA-256 mismatch for $name. Deleted." >&2
      echo "  expected: $sha" >&2
      echo "  got:      $got" >&2
      return 1
    fi
    echo "    SHA-256 verified"
  fi
}

# ----- Boost ----------------------------------------------------------------
if [[ $SKIP_BOOST -eq 1 ]]; then
  echo "Skipping Boost."
else
  BOOST_INSTALL="$DEPS_DIR/boost/install"

  if [[ -f "$BOOST_INSTALL/include/boost/version.hpp" ]] && [[ $FORCE -eq 0 ]]; then
    echo "Boost: already installed at $BOOST_INSTALL"
  elif [[ $LEGACY_BOOST -eq 0 ]]; then
    # ---- Default: CMake-friendly GitHub archive ----
    echo "===== Boost $BOOST_VERSION (CMake-friendly archive) ====="
    BOOST_SRC="$DEPS_DIR/boost/$BOOST_MODULAR_DIR"
    fetch_verified "$BOOST_MODULAR_URL" "" \
                   "$DOWNLOADS_DIR/$BOOST_MODULAR_FILE" "$BOOST_MODULAR_SHA256" "Boost $BOOST_VERSION (CMake-friendly archive)"
    if [[ ! -d "$BOOST_SRC" ]]; then
      echo "  [extract] $BOOST_MODULAR_FILE -> $DEPS_DIR/boost/"
      mkdir -p "$DEPS_DIR/boost"
      tar -xJf "$DOWNLOADS_DIR/$BOOST_MODULAR_FILE" -C "$DEPS_DIR/boost"
    fi
    echo "  [build]  CMake + Ninja (faster than b2; ~5-10 minutes)"
    cmake -S "$BOOST_SRC" -B "$DEPS_DIR/boost/build" -G Ninja \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_INSTALL_PREFIX="$BOOST_INSTALL" \
          -DBOOST_INCLUDE_LIBRARIES="system;filesystem;thread;program_options;iostreams;regex;locale;process;asio;date_time;atomic;chrono;random" \
          -DBUILD_SHARED_LIBS=OFF
    cmake --build "$DEPS_DIR/boost/build" --target install --parallel "$(nproc 2>/dev/null || echo 4)"
    echo "  Boost installed to $BOOST_INSTALL"
  else
    # ---- Legacy: classic tarball + bootstrap.sh + b2 ----
    echo "===== Boost $BOOST_VERSION (legacy b2 build) ====="
    BOOST_SRC="$DEPS_DIR/boost/$BOOST_LEGACY_UNDERSCORED"
    fetch_verified "$BOOST_LEGACY_URL_PRIMARY" "$BOOST_LEGACY_URL_FALLBACK" \
                   "$DOWNLOADS_DIR/$BOOST_LEGACY_FILE" "$BOOST_LEGACY_SHA256" "Boost $BOOST_VERSION (legacy)"
    if [[ ! -d "$BOOST_SRC" ]]; then
      echo "  [extract] $BOOST_LEGACY_FILE -> $DEPS_DIR/boost/"
      mkdir -p "$DEPS_DIR/boost"
      tar -xjf "$DOWNLOADS_DIR/$BOOST_LEGACY_FILE" -C "$DEPS_DIR/boost"
    fi
    echo "  [build]  bootstrap + b2 (this takes ~10-20 minutes on first run)"
    pushd "$BOOST_SRC" >/dev/null
    ./bootstrap.sh --prefix="$BOOST_INSTALL" \
                   --with-libraries=system,filesystem,thread,program_options,iostreams,regex,locale
    ./b2 -j"$(nproc 2>/dev/null || echo 4)" \
         link=static threading=multi runtime-link=shared variant=release \
         --prefix="$BOOST_INSTALL" install
    popd >/dev/null
    echo "  Boost installed to $BOOST_INSTALL"
  fi
fi

# ----- OpenSSL --------------------------------------------------------------
if [[ $SKIP_OPENSSL -eq 1 ]]; then
  echo "Skipping OpenSSL."
else
  OPENSSL_INSTALL="$DEPS_DIR/openssl/install"
  OPENSSL_SRC="$DEPS_DIR/openssl/openssl-${OPENSSL_VERSION}"

  if [[ -f "$OPENSSL_INSTALL/include/openssl/ssl.h" ]] && [[ $FORCE -eq 0 ]]; then
    echo "OpenSSL: already installed at $OPENSSL_INSTALL"
  else
    echo "===== OpenSSL $OPENSSL_VERSION LTS ====="
    fetch_verified "$OPENSSL_URL" "" \
                   "$DOWNLOADS_DIR/$OPENSSL_FILE" "$OPENSSL_SHA256" "OpenSSL $OPENSSL_VERSION"
    if [[ ! -d "$OPENSSL_SRC" ]]; then
      mkdir -p "$DEPS_DIR/openssl"
      tar -xzf "$DOWNLOADS_DIR/$OPENSSL_FILE" -C "$DEPS_DIR/openssl"
    fi
    echo "  [build]  ./Configure + make (this takes ~5-10 minutes on first run)"
    pushd "$OPENSSL_SRC" >/dev/null
    ./Configure --prefix="$OPENSSL_INSTALL" --openssldir="$OPENSSL_INSTALL/ssl" \
                no-tests no-docs no-shared
    make -j"$(nproc 2>/dev/null || echo 4)"
    make install_sw
    popd >/dev/null
    echo "  OpenSSL $OPENSSL_VERSION installed to $OPENSSL_INSTALL"
  fi
fi

# ----- MariaDB Connector/C (vendored source) ---------------------------------
if [[ $SKIP_MARIADB -eq 1 ]]; then
  echo "Skipping MariaDB Connector/C."
else
  echo "===== MariaDB Connector/C $MARIADB_VERSION (vendored) ====="
  if sha256_check "$DOWNLOADS_DIR/$MARIADB_FILE" "$MARIADB_SHA256"; then
    echo "  Vendored archive present and SHA OK."
  else
    fetch_verified "$MARIADB_URL" "" \
                   "$DOWNLOADS_DIR/$MARIADB_FILE" "$MARIADB_SHA256" "MariaDB Connector/C $MARIADB_VERSION"
  fi
  if [[ ! -f "$DEPS_DIR/mariadb/source/CMakeLists.txt" ]]; then
    echo "  [extract] $MARIADB_FILE -> mariadb/source/"
    mkdir -p "$DEPS_DIR/mariadb"
    tmp="$(mktemp -d)"
    unzip -q "$DOWNLOADS_DIR/$MARIADB_FILE" -d "$tmp"
    rm -rf "$DEPS_DIR/mariadb/source"
    mv "$tmp"/* "$DEPS_DIR/mariadb/source"
    rm -rf "$tmp"
  else
    echo "  Source tree already extracted."
  fi
  echo "  (CMake will build it on demand during the main build.)"
fi

echo ""
echo "All requested dependencies are in compile_deps/."
echo "You can now run:"
echo "    cmake --preset linux-gcc-release"
echo "    cmake --build --preset linux-gcc-release"
