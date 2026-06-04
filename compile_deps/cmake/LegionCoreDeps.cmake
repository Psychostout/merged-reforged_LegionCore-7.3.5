# ------------------------------------------------------------------------------
# LegionCoreDeps.cmake
#
# Auto-includes the contents of compile_deps/ into the build.
#
# Sets BOOST_ROOT, MYSQL_ROOT_DIR, OPENSSL_ROOT_DIR to point inside
# compile_deps/ ONLY when the user hasn't already set them on the command line
# or in the environment. External -D<VAR>=... overrides always win.
#
# Also adds the vendored MariaDB Connector/C source as a sub-build target when
# no usable connector is found on the system.
# ------------------------------------------------------------------------------

if(NOT DEFINED LEGIONCORE_DEPS_DIR)
  set(LEGIONCORE_DEPS_DIR "${CMAKE_CURRENT_LIST_DIR}/.." CACHE PATH "Root of compile_deps/")
endif()
get_filename_component(LEGIONCORE_DEPS_DIR "${LEGIONCORE_DEPS_DIR}" ABSOLUTE)

message(STATUS "LegionCoreDeps: scanning ${LEGIONCORE_DEPS_DIR}")

# -- helper: only set a variable if neither cache, command line, nor env has it
function(_lc_set_if_unset var path)
  if(DEFINED ${var} AND NOT "${${var}}" STREQUAL "")
    return()
  endif()
  if(DEFINED ENV{${var}} AND NOT "$ENV{${var}}" STREQUAL "")
    return()
  endif()
  if(EXISTS "${path}")
    set(${var} "${path}" CACHE PATH "Auto-set by compile_deps/cmake/LegionCoreDeps.cmake" FORCE)
    message(STATUS "LegionCoreDeps: ${var} = ${path}")
  endif()
endfunction()

# ------------------------------------------------------------------------------
# Boost
# ------------------------------------------------------------------------------
# Detection layout (the setup scripts produce one of these):
#   compile_deps/boost/boost_1_83_0/                   <- Windows prebuilt root
#   compile_deps/boost/install/                        <- Linux from-source install prefix
set(_lc_boost_candidates
  "${LEGIONCORE_DEPS_DIR}/boost/boost_1_83_0"
  "${LEGIONCORE_DEPS_DIR}/boost/install"
  "${LEGIONCORE_DEPS_DIR}/boost")  # last-ditch fallback
foreach(_p IN LISTS _lc_boost_candidates)
  if(EXISTS "${_p}/include/boost/version.hpp"
     OR EXISTS "${_p}/boost/version.hpp")
    _lc_set_if_unset(BOOST_ROOT "${_p}")
    break()
  endif()
endforeach()
unset(_lc_boost_candidates)

# ------------------------------------------------------------------------------
# OpenSSL
# ------------------------------------------------------------------------------
set(_lc_openssl_candidates
  "${LEGIONCORE_DEPS_DIR}/openssl/OpenSSL-Win64"   # slproweb default install dir we mirror
  "${LEGIONCORE_DEPS_DIR}/openssl/install"          # Linux from-source install prefix
  "${LEGIONCORE_DEPS_DIR}/openssl")
foreach(_p IN LISTS _lc_openssl_candidates)
  if(EXISTS "${_p}/include/openssl/ssl.h")
    _lc_set_if_unset(OPENSSL_ROOT_DIR "${_p}")
    break()
  endif()
endforeach()
unset(_lc_openssl_candidates)

# ------------------------------------------------------------------------------
# MariaDB Connector/C
# ------------------------------------------------------------------------------
# Priority order:
#   1. user-supplied MYSQL_ROOT_DIR  (already wins via _lc_set_if_unset semantics)
#   2. compile_deps/mariadb/<platform>/  (prebuilt drop-in)
#   3. compile_deps/mariadb/source/      (build from vendored source as a sub-project)
if(WIN32)
  set(_lc_mdb_prebuilt "${LEGIONCORE_DEPS_DIR}/mariadb/windows-x64")
else()
  set(_lc_mdb_prebuilt "${LEGIONCORE_DEPS_DIR}/mariadb/linux-x86_64")
endif()

set(_lc_mdb_source "${LEGIONCORE_DEPS_DIR}/mariadb/source")

if(EXISTS "${_lc_mdb_prebuilt}/include/mysql.h"
   OR EXISTS "${_lc_mdb_prebuilt}/include/mariadb/mysql.h")
  _lc_set_if_unset(MYSQL_ROOT_DIR "${_lc_mdb_prebuilt}")
elseif(EXISTS "${_lc_mdb_source}/CMakeLists.txt"
       AND NOT DEFINED MYSQL_ROOT_DIR
       AND NOT DEFINED ENV{MYSQL_ROOT_DIR})
  # No prebuilt and no user override -> build the vendored source as a sub-project.
  # We do this BEFORE the main find_package(MySQL) so MYSQL_ROOT_DIR can point at
  # the to-be-built install tree.
  message(STATUS "LegionCoreDeps: using vendored MariaDB Connector/C source at ${_lc_mdb_source}")
  set(LEGIONCORE_DEPS_BUILD_MARIADB ON CACHE BOOL "Build vendored MariaDB Connector/C" FORCE)
  set(_mdb_build_dir "${CMAKE_BINARY_DIR}/_deps/mariadb-connector-c-build")
  set(_mdb_install_dir "${CMAKE_BINARY_DIR}/_deps/mariadb-connector-c-install")

  # Configure once (idempotent thanks to CMakeCache) at project-load time so the
  # main find_package(MySQL) will see a populated install tree.
  if(NOT EXISTS "${_mdb_install_dir}/include/mysql.h"
     AND NOT EXISTS "${_mdb_install_dir}/include/mariadb/mysql.h")
    message(STATUS "LegionCoreDeps: building vendored MariaDB Connector/C (one-time, ~1 min)")
    file(MAKE_DIRECTORY "${_mdb_build_dir}")
    execute_process(
      COMMAND ${CMAKE_COMMAND}
              -S "${_lc_mdb_source}"
              -B "${_mdb_build_dir}"
              -DCMAKE_INSTALL_PREFIX=${_mdb_install_dir}
              -DCMAKE_BUILD_TYPE=Release
              -DWITH_UNIT_TESTS=OFF
              -DWITH_EXTERNAL_ZLIB=OFF
              -DWITH_MSI=OFF
              -DWITH_SIGNCODE=OFF
              -DWITH_CURL=OFF
              -DCLIENT_PLUGIN_AUTH_GSSAPI_CLIENT=OFF
      RESULT_VARIABLE _mdb_cfg_rv)
    if(NOT _mdb_cfg_rv EQUAL 0)
      message(FATAL_ERROR "LegionCoreDeps: vendored MariaDB Connector/C configure failed (exit ${_mdb_cfg_rv})")
    endif()
    execute_process(
      COMMAND ${CMAKE_COMMAND} --build "${_mdb_build_dir}" --config Release --target install --parallel
      RESULT_VARIABLE _mdb_bld_rv)
    if(NOT _mdb_bld_rv EQUAL 0)
      message(FATAL_ERROR "LegionCoreDeps: vendored MariaDB Connector/C build failed (exit ${_mdb_bld_rv})")
    endif()
  endif()
  _lc_set_if_unset(MYSQL_ROOT_DIR "${_mdb_install_dir}")
  unset(_mdb_build_dir)
  unset(_mdb_install_dir)
endif()
unset(_lc_mdb_prebuilt)
unset(_lc_mdb_source)

message(STATUS "LegionCoreDeps: done")
