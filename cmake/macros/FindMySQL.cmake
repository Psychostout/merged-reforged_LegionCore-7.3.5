# This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>.

#[=======================================================================[.rst:
FindMySQL
-----------

Find MySQL.

Imported Targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` targets:

``MySQL::MySQL``
  MySQL client library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``MYSQL_FOUND``
  System has MySQL.
``MYSQL_INCLUDE_DIR``
  MySQL include directory.
``MYSQL_LIBRARY``
  MySQL library.
``MYSQL_EXECUTABLE``
  Path to mysql client binary.

Hints
^^^^^

Set ``MYSQL_ROOT_DIR`` to the root directory of MySQL installation.
#]=======================================================================]

set(MYSQL_FOUND 0)

set(_MYSQL_ROOT_HINTS
  ${MYSQL_ROOT_DIR}
  ENV MYSQL_ROOT_DIR
)

if(UNIX)
  set(MYSQL_CONFIG_PREFER_PATH "$ENV{MYSQL_HOME}/bin" CACHE FILEPATH
    "preferred path to MySQL (mysql_config)"
  )

  # Prefer mysql_config / mariadb_config from a user-set MYSQL_ROOT_DIR (e.g.
  # the vendored Connector/C built by compile_deps/) BEFORE falling back to
  # system paths. Otherwise FindMySQL would silently link the system libmariadb.
  find_program(MYSQL_CONFIG
    NAMES mariadb_config mysql_config
    HINTS
      ${MYSQL_ROOT_DIR}
      ENV MYSQL_ROOT_DIR
    PATH_SUFFIXES bin
    NO_DEFAULT_PATH
  )
  find_program(MYSQL_CONFIG
    NAMES mariadb_config mysql_config
    PATHS
      ${MYSQL_CONFIG_PREFER_PATH}
      /usr/local/mysql/bin/
      /usr/local/bin/
      /usr/bin/
  )

  if(MYSQL_CONFIG)
    message(STATUS "Using mysql-config: ${MYSQL_CONFIG}")
    # set INCLUDE_DIR
    execute_process(
      COMMAND "${MYSQL_CONFIG}" --include
      OUTPUT_VARIABLE MY_TMP
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    string(REGEX REPLACE "-I([^ ]*)( .*)?" "\\1" MY_TMP "${MY_TMP}")
    set(MYSQL_ADD_INCLUDE_PATH ${MY_TMP} CACHE FILEPATH INTERNAL)
    #message("[DEBUG] MYSQL ADD_INCLUDE_PATH : ${MYSQL_ADD_INCLUDE_PATH}")
    # set LIBRARY_DIR
    execute_process(
      COMMAND "${MYSQL_CONFIG}" --libs_r
      OUTPUT_VARIABLE MY_TMP
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(MYSQL_ADD_LIBRARIES "")
    string(REGEX MATCHALL "-l[^ ]*" MYSQL_LIB_LIST "${MY_TMP}")
    foreach(LIB ${MYSQL_LIB_LIST})
      string(REGEX REPLACE "[ ]*-l([^ ]*)" "\\1" LIB "${LIB}")
      list(APPEND MYSQL_ADD_LIBRARIES "${LIB}")
      #message("[DEBUG] MYSQL ADD_LIBRARIES : ${MYSQL_ADD_LIBRARIES}")
    endforeach(LIB ${MYSQL_LIB_LIST})

    set(MYSQL_ADD_LIBRARIES_PATH "")
    string(REGEX MATCHALL "-L[^ ]*" MYSQL_LIBDIR_LIST "${MY_TMP}")
    foreach(LIB ${MYSQL_LIBDIR_LIST})
      string(REGEX REPLACE "[ ]*-L([^ ]*)" "\\1" LIB "${LIB}")
      list(APPEND MYSQL_ADD_LIBRARIES_PATH "${LIB}")
      #message("[DEBUG] MYSQL ADD_LIBRARIES_PATH : ${MYSQL_ADD_LIBRARIES_PATH}")
    endforeach(LIB ${MYSQL_LIBS})

  else(MYSQL_CONFIG)
    set(MYSQL_ADD_LIBRARIES "")
    list(APPEND MYSQL_ADD_LIBRARIES "mysqlclient_r")
  endif(MYSQL_CONFIG)
endif(UNIX)

set(_MYSQL_ROOT_PATHS)

if(WIN32)
  # read environment variables and change \ to /
  file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" PROGRAM_FILES_32)
  file(TO_CMAKE_PATH "$ENV{ProgramW6432}" PROGRAM_FILES_64)

  cmake_host_system_information(
    RESULT
      _MYSQL_ROOT_HINTS_SUBKEYS
    QUERY
      WINDOWS_REGISTRY
      "HKEY_LOCAL_MACHINE\\SOFTWARE\\MySQL AB" SUBKEYS
    VIEW BOTH
  )
  list(SORT _MYSQL_ROOT_HINTS_SUBKEYS COMPARE NATURAL ORDER DESCENDING)

  set(_MYSQL_ROOT_HINTS_REGISTRY_LOCATIONS)
  foreach(subkey IN LISTS _MYSQL_ROOT_HINTS_SUBKEYS)
    cmake_host_system_information(
      RESULT
        _MYSQL_ROOT_HINTS_REGISTRY_LOCATION
      QUERY
        WINDOWS_REGISTRY
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\MySQL AB\\${subkey}" VALUE "Location"
      VIEW BOTH
    )
    list(APPEND _MYSQL_ROOT_HINTS_REGISTRY_LOCATIONS ${_MYSQL_ROOT_HINTS_REGISTRY_LOCATION})
  endforeach()

  set(_MYSQL_ROOT_HINTS
    ${_MYSQL_ROOT_HINTS}
	${_MYSQL_ROOT_HINTS_REGISTRY_LOCATIONS}
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MariaDB 10.4;INSTALLDIR]"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MariaDB 10.4 (x64);INSTALLDIR]"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MariaDB 10.5;INSTALLDIR]"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MariaDB 10.5 (x64);INSTALLDIR]"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MariaDB 10.6;INSTALLDIR]"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MariaDB 10.6 (x64);INSTALLDIR]"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MariaDB 10.11;INSTALLDIR]"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MariaDB 10.11 (x64);INSTALLDIR]"
  )

  file(GLOB _MYSQL_ROOT_PATHS_VERSION_SUBDIRECTORIES
    LIST_DIRECTORIES TRUE
    "${PROGRAM_FILES_64}/MySQL/MySQL Server *"
    "${PROGRAM_FILES_32}/MySQL/MySQL Server *"
    "$ENV{SystemDrive}/MySQL/MySQL Server *"
    "${PROGRAM_FILES_64}/MariaDB/MariaDB Server *"
    "${PROGRAM_FILES_32}/MariaDB/MariaDB Server *"
    "$ENV{SystemDrive}/MariaDB/MariaDB Server *"
  )

  list(SORT _MYSQL_ROOT_PATHS_VERSION_SUBDIRECTORIES COMPARE NATURAL ORDER DESCENDING)

  set(_MYSQL_ROOT_PATHS
    ${_MYSQL_ROOT_PATHS}
	${_MYSQL_ROOT_PATHS_VERSION_SUBDIRECTORIES}
    "${PROGRAM_FILES_64}/MySQL"
    "${PROGRAM_FILES_32}/MySQL"
    "$ENV{SystemDrive}/MySQL"
  )
endif(WIN32)

# If MYSQL_ROOT_DIR is set (e.g. by compile_deps/cmake/LegionCoreDeps.cmake or
# by the user on the command line), search there FIRST with NO_DEFAULT_PATH so
# the vendored Connector/C wins over any system-installed libmariadb.
if(MYSQL_ROOT_DIR OR DEFINED ENV{MYSQL_ROOT_DIR})
  find_path(MYSQL_INCLUDE_DIR
    NAMES mysql.h
    HINTS ${MYSQL_ROOT_DIR} ENV MYSQL_ROOT_DIR
    PATH_SUFFIXES include include/mysql include/mariadb
    NO_DEFAULT_PATH
  )
  if(UNIX)
    find_library(MYSQL_LIBRARY
      NAMES mariadb libmariadb mariadbclient libmariadbclient mysqlclient libmysqlclient
      HINTS ${MYSQL_ROOT_DIR} ENV MYSQL_ROOT_DIR
      PATH_SUFFIXES lib lib/mariadb lib64 lib64/mariadb
      NO_DEFAULT_PATH
    )
  endif()
endif()

find_path(MYSQL_INCLUDE_DIR
  NAMES
    mysql.h
  HINTS
    ${_MYSQL_ROOT_HINTS}
  PATHS
    ${MYSQL_ADD_INCLUDE_PATH}
    /usr/include
    /usr/include/mysql
    /usr/local/include
    /usr/local/include/mysql
    /usr/local/mysql/include
	${_MYSQL_ROOT_PATHS}
  PATH_SUFFIXES
    include
    include/mysql
  DOC
    "Specify the directory containing mysql.h."
)

if(UNIX)
  foreach(LIB ${MYSQL_ADD_LIBRARIES})
    find_library(MYSQL_LIBRARY
      NAMES
        mysql libmysql ${LIB}
      PATHS
        ${MYSQL_ADD_LIBRARIES_PATH}
        /usr/lib
        /usr/lib/mysql
        /usr/local/lib
        /usr/local/lib/mysql
        /usr/local/mysql/lib
      DOC "Specify the location of the mysql library here."
    )
  endforeach(LIB ${MYSQL_ADD_LIBRARY})
endif(UNIX)

if(WIN32)
  find_library(MYSQL_LIBRARY
    NAMES
      libmysql libmariadb
    HINTS
      ${_MYSQL_ROOT_HINTS}
    PATHS
      ${MYSQL_ADD_LIBRARIES_PATH}
      ${_MYSQL_ROOT_PATHS}
    PATH_SUFFIXES
      lib
      lib/opt
    DOC "Specify the location of the mysql library here."
  )
endif(WIN32)

# On Windows you typically don't need to include any extra libraries
# to build MYSQL stuff.

if(NOT WIN32)
  find_library(MYSQL_EXTRA_LIBRARIES
    NAMES
      z zlib
    PATHS
      /usr/lib
      /usr/local/lib
    DOC
      "if more libraries are necessary to link in a MySQL client (typically zlib), specify them here."
  )
else(NOT WIN32)
  set(MYSQL_EXTRA_LIBRARIES "")
endif(NOT WIN32)

if(UNIX)
    find_program(MYSQL_EXECUTABLE mysql
    PATHS
        ${MYSQL_CONFIG_PREFER_PATH}
        /usr/local/mysql/bin/
        /usr/local/bin/
        /usr/bin/
    DOC
        "path to your mysql binary."
    )
endif(UNIX)

if(WIN32)
  find_program(MYSQL_EXECUTABLE mysql
    HINTS
      ${_MYSQL_ROOT_HINTS}
    PATHS
      ${_MYSQL_ROOT_PATHS}
    PATH_SUFFIXES
      bin
      bin/opt
    DOC
      "path to your mysql binary."
  )
endif(WIN32)

unset(MySQL_lib_WANTED)
unset(MySQL_binary_WANTED)
set(MYSQL_REQUIRED_VARS "")
foreach(_comp IN LISTS MySQL_FIND_COMPONENTS)
  if(_comp STREQUAL "lib")
    set(MySQL_${_comp}_WANTED TRUE)
	if(MySQL_FIND_REQUIRED_${_comp})
	  list(APPEND MYSQL_REQUIRED_VARS "MYSQL_LIBRARY")
	  list(APPEND MYSQL_REQUIRED_VARS "MYSQL_INCLUDE_DIR")
	endif()
    if(EXISTS "${MYSQL_LIBRARY}" AND EXISTS "${MYSQL_INCLUDE_DIR}")
      set(MySQL_${_comp}_FOUND TRUE)
    else()
      set(MySQL_${_comp}_FOUND FALSE)
    endif()
  elseif(_comp STREQUAL "binary")
    set(MySQL_${_comp}_WANTED TRUE)
	if(MySQL_FIND_REQUIRED_${_comp})
	  list(APPEND MYSQL_REQUIRED_VARS "MYSQL_EXECUTABLE")
	endif()
    if(EXISTS "${MYSQL_EXECUTABLE}" )
      set(MySQL_${_comp}_FOUND TRUE)
    else()
      set(MySQL_${_comp}_FOUND FALSE)
    endif()
  else()
    message(WARNING "${_comp} is not a valid MySQL component")
    set(MySQL_${_comp}_FOUND FALSE)
  endif()
endforeach()
unset(_comp)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MySQL
  REQUIRED_VARS
    ${MYSQL_REQUIRED_VARS}
  HANDLE_COMPONENTS
  FAIL_MESSAGE
    "Could not find the MySQL libraries! Please install the development libraries and headers"
)
unset(MYSQL_REQUIRED_VARS)

if(MYSQL_FOUND)
  if(MySQL_lib_WANTED AND MySQL_lib_FOUND)
    message(STATUS "Found MySQL library: ${MYSQL_LIBRARY}")
    message(STATUS "Found MySQL headers: ${MYSQL_INCLUDE_DIR}")
  endif()

  # ---------------------------------------------------------------------------
  # LegionCore minimum-version enforcement.
  #
  # We officially target MariaDB 10.6.3+ (LTS series). MySQL 8.0+ also works,
  # but the SQL schema and update scripts assume MariaDB-flavoured grammar.
  # We try mysql_config --version first, then parse mysql.h as a fallback.
  # ---------------------------------------------------------------------------
  set(LEGIONCORE_MIN_MARIADB_VERSION "10.6.3")
  set(_MYSQL_DETECTED_VERSION "")

  if(MYSQL_CONFIG)
    execute_process(
      COMMAND "${MYSQL_CONFIG}" --version
      OUTPUT_VARIABLE _MYSQL_CONFIG_VERSION_RAW
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET)
    string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" _MYSQL_DETECTED_VERSION "${_MYSQL_CONFIG_VERSION_RAW}")
  endif()

  if(NOT _MYSQL_DETECTED_VERSION AND EXISTS "${MYSQL_INCLUDE_DIR}/mysql_version.h")
    file(STRINGS "${MYSQL_INCLUDE_DIR}/mysql_version.h" _MYSQL_VERSION_LINE
         REGEX "^[ \t]*#define[ \t]+MYSQL_SERVER_VERSION[ \t]+\"[^\"]+\"")
    string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" _MYSQL_DETECTED_VERSION "${_MYSQL_VERSION_LINE}")
  endif()

  if(_MYSQL_DETECTED_VERSION)
    message(STATUS "Detected MySQL/MariaDB client version: ${_MYSQL_DETECTED_VERSION}")
    # MariaDB is reported as e.g. "10.11.6-MariaDB"; MySQL as "8.0.36".
    # Only enforce the floor for the MariaDB (10.x / 11.x) family.
    if("${_MYSQL_CONFIG_VERSION_RAW}" MATCHES "[Mm]aria" OR _MYSQL_DETECTED_VERSION VERSION_LESS "8.0.0")
      if(_MYSQL_DETECTED_VERSION VERSION_LESS "${LEGIONCORE_MIN_MARIADB_VERSION}")
        message(WARNING
          "LegionCore requires MariaDB >= ${LEGIONCORE_MIN_MARIADB_VERSION} "
          "(detected ${_MYSQL_DETECTED_VERSION}). The build will continue, but "
          "the server may refuse to start or fail SQL updates at runtime.")
      endif()
    endif()
  else()
    message(STATUS "Could not determine MySQL/MariaDB version; skipping minimum-version check "
                   "(LegionCore targets MariaDB >= ${LEGIONCORE_MIN_MARIADB_VERSION}).")
  endif()
  unset(_MYSQL_DETECTED_VERSION)
  unset(_MYSQL_CONFIG_VERSION_RAW)
  unset(_MYSQL_VERSION_LINE)
  # ---------------------------------------------------------------------------
  if(MySQL_binary_WANTED AND MySQL_binary_FOUND)
    message(STATUS "Found MySQL executable: ${MYSQL_EXECUTABLE}")
  endif()
  mark_as_advanced(MYSQL_FOUND MYSQL_LIBRARY MYSQL_EXTRA_LIBRARIES MYSQL_INCLUDE_DIR MYSQL_EXECUTABLE)

  if(NOT TARGET MySQL::MySQL AND MySQL_lib_WANTED AND MySQL_lib_FOUND)
    add_library(MySQL::MySQL UNKNOWN IMPORTED)
    set_target_properties(MySQL::MySQL
      PROPERTIES
        IMPORTED_LOCATION
          "${MYSQL_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES
          "${MYSQL_INCLUDE_DIR}")
  endif()
else()
  message(FATAL_ERROR "Could not find the MySQL libraries! Please install the development libraries and headers")
endif()