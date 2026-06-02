# Target Windows 10 (RS5 / 1809) as the API floor.
# 0x0601 = Win 7 (EOL, dropped by Boost 1.85 / OpenSSL 3 / modern MSVC runtimes)
# 0x0A00 = Win 10 / Server 2016+
add_definitions(-D_WIN32_WINNT=0x0A00)
add_definitions(-DWINVER=0x0A00)
add_definitions(-DNTDDI_VERSION=0x0A000007)  # NTDDI_WIN10_RS5 (1809)
add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-DNOMINMAX)
# Boost.Asio + Windows.h interaction: prevent winsock.h being pulled in by windows.h
# (winsock2.h must come first). Boost handles this only if WIN32_LEAN_AND_MEAN is set,
# which we already do above, but be explicit for any code that includes <windows.h> directly.
add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)

# set up output paths for executable binaries (.exe-files, and .dll-files on DLL-capable platforms)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  include(${CMAKE_SOURCE_DIR}/cmake/compiler/msvc/settings.cmake)
elseif (CMAKE_CXX_PLATFORM_ID MATCHES "MinGW")
  include(${CMAKE_SOURCE_DIR}/cmake/compiler/mingw/settings.cmake)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  include(${CMAKE_SOURCE_DIR}/cmake/compiler/clang/settings.cmake)
endif()
