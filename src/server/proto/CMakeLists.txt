# Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders
  # Exclude the BNet Client/ proto descriptors. The upstream Reforged fork
  # only committed 4 of the ~17 .pb.cc files this directory needs (it's
  # missing rpc_types.pb.h, attribute_types.pb.h, account_service.pb.h, and
  # 13 more service descriptors). We investigated backporting them from
  # upstream TrinityCore master but TC master's proto tree has diverged
  # significantly: it's restructured into api/{client,common}/v1/ subdirs
  # with hundreds of files that LegionCore 7.3.5 was never designed for.
  # The bnetserver and game/Services classes that need the full BNet
  # descriptor set need those regenerated specifically for Legion 7.3.5
  # before bnetserver can fully link. See README.md "Known limitation".
  # The remaining proto sources (Login.pb.cc, RealmList.pb.cc) ARE
  # complete and self-contained, so libproto.a still builds usefully.
  ${CMAKE_CURRENT_SOURCE_DIR}/Client)

if (USE_COREPCH)
  set(PRIVATE_PCH_HEADER PrecompiledHeaders/protoPCH.h)
endif()

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_library(proto
  ${PRIVATE_SOURCES}
)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders
  ${CMAKE_CURRENT_SOURCE_DIR}/Client/api
  ${CMAKE_CURRENT_SOURCE_DIR}/Client/global_extensions)

target_include_directories(proto
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(proto
  PRIVATE
    trinity-default-interface
    trinity-no-warning-interface
  PUBLIC
    common
    protobuf)

if (MSVC)
  target_compile_definitions(protobuf
    PRIVATE
      -D_SCL_SECURE_NO_WARNINGS)
endif()

set_target_properties(proto
  PROPERTIES
    DEFINE_SYMBOL TRINITY_API_EXPORT_PROTO
    FOLDER "server")

if( BUILD_SHARED_LIBS )
  if( UNIX )
    install(TARGETS proto
      LIBRARY
        DESTINATION lib)
  elseif( WIN32 )
    install(TARGETS proto
      RUNTIME
        DESTINATION "${CMAKE_INSTALL_PREFIX}")
  endif()
endif()

# Generate precompiled header
if (USE_COREPCH)
  add_cxx_pch(proto ${PRIVATE_PCH_HEADER})
endif()
