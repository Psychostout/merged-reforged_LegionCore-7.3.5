LegionCore 7.3.5 Reforged — Complete Work Summary
==================================================

Last updated: 2026-06-04
Repository: https://github.com/Psychostout/merged-reforged_LegionCore-7.3.5
Local workspace path used in this session:
  /home/user/merged-reforged_LegionCore-7.3.5

Purpose of this file
--------------------
This file replaces the very long working transcript with a readable summary of
everything done from the beginning of the project through the current state.
The original long transcript was preserved as:

  uploads/Legion.readme.original-transcript.txt
  merged-reforged_LegionCore-7.3.5/LEGION_README.original-transcript.txt

Current high-level status
-------------------------
The project is a merged and modernized LegionCore 7.3.5 source tree with:

  * Legion client build: 26972
  * Reforged changes merged into LegionCore-7.3.5
  * CMake support: 3.18...4.3.2
  * Boost floor: 1.83
  * MariaDB floor: 10.6.3+
  * OpenSSL target: 3.5 LTS
  * Windows toolchain: Visual Studio 2022 / MSVC 19.30 / Win10 API
  * compile_deps/ helper system
  * SoloCraft restored
  * BNet protobuf files present and wired into CMake
  * bnetserver successfully built in a targeted Linux test with Boost 1.83

Most important confirmed milestone:

  [OK] bnetserver builds successfully with Boost 1.83.

Most important unverified item:

  [NOT YET VERIFIED] full worldserver/scripts build.


1. Original source/merge work
-----------------------------
The work began with two codebases:

  Base repo:
    The-Legion-Preservation-Project/LegionCore-7.3.5

  Reforged repo:
    Psychostout/LegionCore-Reforged

The previous session found that Reforged was not a divergent fork. It was ahead
of the then-current LegionCore-7.3.5 main branch by 64 commits, with no
conflicting divergence. A merge was performed cleanly.

Merge facts recorded in the earlier transcript:

  * Reforged was 64 commits ahead.
  * Merge had no textual conflicts.
  * Original main branch was preserved as a rollback point in that earlier work.
  * The merged tree matched Reforged at the merge point, before later
    modernization changes were added.

Reforged brought in work around BattlePay, WoW Token packets, Class Trial,
character boost, tools, extractors, deleted-character restoration, damage
variance, bag/bank sorting, Demon Invasion scripts, Skyreach/Spires work,
Brawler's Guild fixes, SQL cleanups, and other gameplay/script updates.


2. CMake modernization
----------------------
The top-level CMake requirement was changed to the range form:

  cmake_minimum_required(VERSION 3.18...4.3.2)

Meaning:

  * CMake 3.18 remains the minimum floor.
  * CMake policy behavior is declared compatible through 4.3.2.
  * This avoids CMake 4.x old-policy/deprecation problems.

Earlier configure tests showed CMake could parse/generate the project. In the
current session, CMake was also used for targeted bnetserver builds.


3. Dependency modernization history
-----------------------------------
The earlier transcript originally raised dependencies to:

  * Boost 1.85
  * MariaDB 10.6.3
  * OpenSSL 3.5 LTS

MariaDB:

  * FindMySQL.cmake was improved.
  * MariaDB 10.6 and 10.11 Windows registry hints were added.
  * A MariaDB version probe was added.
  * The project records a minimum MariaDB target of 10.6.3.

OpenSSL:

  * compile_deps was updated toward OpenSSL 3.5 LTS.

Boost:

  * The earlier target was Boost 1.85.
  * In the current recovery work, real testing proved bnetserver builds with
    Boost 1.83 and fails with Boost 1.88 because Boost 1.88 no longer provides
    boost/asio/io_service.hpp.
  * Therefore the current project was changed to Boost 1.83.

Current dependency position:

  * Boost: 1.83
  * MariaDB: 10.6.3+
  * OpenSSL: 3.5 LTS target, system 3.x acceptable in the current scripts/docs


4. Windows support modernization
--------------------------------
The Windows build support was modernized for:

  * Visual Studio 2022
  * MSVC v143 toolset
  * MSVC compiler version 19.30+
  * Windows 10 API target

Key Windows CMake settings include:

  * _WIN32_WINNT=0x0A00
  * WINVER=0x0A00
  * NTDDI_VERSION=0x0A000007

MSVC flags added/kept include:

  * /Zc:__cplusplus
  * /Zc:preprocessor
  * /utf-8
  * /EHsc
  * /permissive-

Files involved:

  * cmake/compiler/msvc/settings.cmake
  * cmake/platform/win/settings.cmake
  * CMakePresets.json
  * BUILD_WINDOWS.md

After Step 7, the Windows docs and presets point to Boost 1.83 paths, not
Boost 1.85 paths.


5. compile_deps/ dependency helper system
-----------------------------------------
A compile_deps/ folder was introduced so users can fetch/build dependencies in
a controlled way.

Important files:

  * compile_deps/README.md
  * compile_deps/DEPENDENCIES.md
  * compile_deps/setup_deps.ps1
  * compile_deps/setup_deps.sh
  * compile_deps/cmake/LegionCoreDeps.cmake
  * compile_deps/mariadb/source/

MariaDB Connector/C 3.4.8 source is vendored under compile_deps.

After Step 7, compile_deps now targets Boost 1.83:

  Windows prebuilt:
    boost_1_83_0-msvc-14.3-64.exe
    SHA-256:
    67975ce4a8799f17728ddba8e64e9b450a6bda7762643e829a96ccbbd1ca17d2

  Linux/source archive:
    boost-1.83.0.tar.xz
    SHA-256:
    c5a0688e1f0c05f354bbd0b32244d36085d9ffc9f932e8a18983a9908096f614

  Legacy source archive:
    boost_1_83_0.tar.bz2
    SHA-256:
    79e6d3f986444e5a80afbeccdaf2d1c1cf964baa8d766d20859d653a16c39848

compile_deps/cmake/LegionCoreDeps.cmake now checks for:

  compile_deps/boost/boost_1_83_0
  compile_deps/boost/install
  compile_deps/boost


6. CMake presets and build docs
-------------------------------
CMakePresets.json exists and includes presets for:

  * windows-msvc-release
  * windows-msvc-debug
  * linux-gcc-release

After Step 7:

  * Windows BOOST_ROOT default is C:/local/boost_1_83_0.
  * The Windows preset description references Boost 1.83.

BUILD_WINDOWS.md was updated so Windows users install:

  boost_1_83_0-msvc-14.3-64.exe

and set:

  BOOST_ROOT=C:\local\boost_1_83_0


7. SoloCraft restoration
------------------------
SoloCraft was restored from upstream source.

Files restored/present:

  src/server/scripts/Custom/Solocraft.cpp
  src/server/scripts/Custom/custom_script_loader.cpp

SoloCraft defaults to disabled:

  Solocraft.Enable = 0

SoloCraft is intended to be opt-in.


8. Config translation work
--------------------------
The earlier transcript included substantial work translating:

  src/server/worldserver/worldserver.conf.dist

from French to English while trying to preserve all actual setting lines.

A Google/manual translation was later placed under:

  _translation_workspace/

Known current note from Step 2 static inspection:

  * The live worldserver config has the key values expected.
  * The duplicate config copies may not be synchronized.
  * The duplicate Merged-LegionCore-Reforged copy lacked Solocraft.Enable in
    Step 2 inspection.
  * Some residual mechanical/French fragments may remain in comments.

Important live config values verified in Step 2:

  Game.Build.Version = 26972
  RealmID = 1
  WorldServerPort = 8085
  BnetServer.Port = 1118
  Solocraft.Enable = 0
  Warden.Enabled = 0
  Anticheat.Enable = 0

This area has not yet been cleaned/finalized in the current recovery steps.


9. BNet protobuf problem and resolution
---------------------------------------
Earlier, the project was blocked because important BNet protobuf generated
files were missing.

Examples of previously missing files:

  account_service.pb.h/.cc
  authentication_service.pb.h/.cc
  challenge_service.pb.h/.cc
  channel_service.pb.h/.cc
  connection_service.pb.h/.cc
  friends_service.pb.h/.cc
  game_utilities_service.pb.h/.cc
  notification_types.pb.h/.cc
  presence_service.pb.h/.cc
  profanity_filter_config.pb.h/.cc
  report_service.pb.h/.cc
  resource_service.pb.h/.cc
  user_manager_service.pb.h/.cc
  rpc_types.pb.h/.cc
  attribute_types.pb.h/.cc

An earlier attempt to use modern TrinityCore master protos was rejected because
those files were Dragonflight-era and structurally incompatible with Legion 7.3.5.

Later, the correct flat/period-style BNet protobuf files were added by the user.

In the current Step 2 inspection, those files were confirmed present under:

  src/server/proto/Client/

Step 3 fixed the stale build wiring:

  * src/server/proto/CMakeLists.txt no longer excludes Client/.
  * src/server/proto/PrecompiledHeaders/protoPCH.h was updated to reference the
    committed BNet descriptors.

This fixed the old condition where files existed but CMake still ignored them.


10. Current chat timeline
-------------------------
The current chat began because the previous chat/workspace was stuck and could
not be downloaded.

Step 1:
  Cloned:
    https://github.com/Psychostout/merged-reforged_LegionCore-7.3.5
  into:
    /home/user/merged-reforged_LegionCore-7.3.5

  Initial cloned HEAD:
    9dee6a7 Create LEGION_README.txt

Step 2:
  Performed static verification only. No build was run.

  Verified modernization files, compile_deps, SoloCraft, BNet proto files, and
  config basics.

  Found stale proto wiring:
    src/server/proto/CMakeLists.txt still excluded Client/.

  Added CHANGELOG Step 2 report.

  Commit:
    94ff37e Docs: add Step 2 static verification report

Step 3:
  Fixed proto wiring without touching README.md, per instruction.

  Changed:
    src/server/proto/CMakeLists.txt
    src/server/proto/PrecompiledHeaders/protoPCH.h
    CHANGELOG.txt

  Commit:
    3453074 Proto: include committed BNet descriptors in build wiring

Step 4:
  Ran a targeted bnetserver-only build test with Boost 1.88.

  Configure succeeded.

  Build failed at:
    src/server/bnetserver/REST/LoginRESTService.h

  Error:
    fatal error: boost/asio/io_service.hpp: No such file or directory

  Conclusion:
    Boost 1.88 does not provide the legacy Boost.Asio io_service.hpp header.

Step 5:
  Analyzed the Boost failure.

  Determined the previous successful bnetserver build likely used Boost 1.83.

  Suggested either:
    A) standardize on Boost 1.83, or
    B) modernize Boost.Asio includes for Boost 1.85+.

Step 6:
  Tested bnetserver with Boost 1.83.

  Temporarily lowered local Boost requirement to 1.83 for the test.

  Installed Boost 1.83.

  First build with -j2 failed due to OOM:
    c++: fatal error: Killed signal terminated program cc1plus

  Retried with -j1.

  Result:
    bnetserver built successfully.

  Output binary:
    build/bnetserver-boost183/src/server/bnetserver/bnetserver

  Binary size:
    106M

  Linked Boost libraries:
    libboost_filesystem.so.1.83.0
    libboost_program_options.so.1.83.0
    libboost_thread.so.1.83.0

Step 7:
  Permanently changed project support from Boost 1.85 to Boost 1.83.

  Files updated included:
    dep/boost/CMakeLists.txt
    CMakePresets.json
    BUILD_WINDOWS.md
    README.md
    compile_deps/DEPENDENCIES.md
    compile_deps/README.md
    compile_deps/setup_deps.ps1
    compile_deps/setup_deps.sh
    compile_deps/cmake/LegionCoreDeps.cmake
    cmake/compiler/msvc/settings.cmake
    cmake/platform/win/settings.cmake
    CHANGELOG.txt

  Commit:
    42dcfd2 Deps: set supported Boost floor to 1.83

Step 8:
  This file was rewritten into a complete current summary.


11. Current git state
---------------------
Current local commits added in this recovery chat:

  42dcfd2 Deps: set supported Boost floor to 1.83
  3453074 Proto: include committed BNet descriptors in build wiring
  94ff37e Docs: add Step 2 static verification report

At the time this summary was written, Step 8 updates this LEGION_README file but
has not yet been followed by a user-provided Step 9.


12. Current confirmed build status
----------------------------------
Confirmed successful:

  Target:
    bnetserver

  Build style:
    targeted only, not full build

  Successful command style:

    cmake -S . -B build/bnetserver-boost183 -G Ninja \
          -DCMAKE_BUILD_TYPE=RelWithDebInfo \
          -DTOOLS=0 \
          -DSCRIPTS=none

    cmake --build build/bnetserver-boost183 --target bnetserver -- -j1

  Required Boost version for this successful test:
    Boost 1.83

Confirmed not successful:

  Boost 1.88 bnetserver build failed due to missing:
    boost/asio/io_service.hpp

Not yet tested/verified:

  * full worldserver build
  * all scripts build
  * Windows build
  * packaging/install flow


13. Current known issues / next likely tasks
--------------------------------------------
1. Config synchronization/cleanup:

   The worldserver.conf.dist copies should be compared and synchronized:

     src/server/worldserver/worldserver.conf.dist
     _translation_workspace/worldserver.conf.dist.english
     Merged-LegionCore-Reforged/src/server/worldserver/worldserver.conf.dist

   Need to ensure Solocraft.Enable is present where expected and that comments
   are properly English.

2. Full build not yet verified:

   bnetserver is confirmed. worldserver and scripts are not.

3. Boost 1.85+ optional future path:

   If the project later wants to return to Boost 1.85+, the first known code
   compatibility issue is:

     src/server/bnetserver/REST/LoginRESTService.h
       #include <boost/asio/io_service.hpp>

   This likely needs modernization to io_context.hpp and follow-up testing.

4. README/CHANGELOG should remain aligned:

   README now states Boost 1.83 and bnetserver build success. CHANGELOG includes
   Step 2, Step 3, and Step 7 entries.


14. Current recommended next step
---------------------------------
Wait for Step 9.

Recommended Step 9 options:

  A) Commit this summary update to LEGION_README.txt.
  B) Fix/synchronize the config files.
  C) Run a targeted worldserver-related build only if explicitly permitted.
  D) Produce a patch file for the local commits so the work can be applied to
     the GitHub repo.

No further build should be run unless explicitly requested.


15. Step 9 documentation verification
-------------------------------------
Step 9 checked BUILD_WINDOWS.md, README.md, compile_deps documentation/scripts,
CMakePresets.json, and this LEGION_README summary.

Corrections made:

  * BUILD_WINDOWS.md now consistently uses Boost 1.83 paths.
  * The sample override path was corrected from D:\boost_1_88_0 to
    D:\boost_1_83_0.
  * The clone instructions now use git checkout main.
  * Misleading VS2019 Boost advice was removed; this branch requires VS2022 /
    MSVC v143.
  * README.md now records the current 479-setting worldserver.conf.dist count.
  * README.md OpenSSL tested version was updated to 3.5.6.
  * compile_deps wording was corrected for the Boost 1.83 CMake-friendly GitHub
    tarball.
  * Static checks passed: CMakePresets.json parses and setup_deps.sh passes
    bash -n.

Auto-build options identified:

  1. CMake build presets for bnetserver only.
  2. Linux/Windows helper scripts that configure and build only bnetserver.
  3. GitHub Actions CI for bnetserver, then later Windows/full builds.
  4. A larger bootstrap script for dependencies, configure, build, and package.

Recommended next implementation:

  Add bnetserver-only CMake build presets plus small Linux/Windows helper
  scripts first. Add GitHub Actions after the local scripted path is stable.


16. Step 10 compile_deps and portable server documentation
----------------------------------------------------------
Step 10 corrected dependency routing and added a standalone server deployment
guide.

Changes made:

  * CMakePresets.json no longer hard-codes BOOST_ROOT, MYSQL_ROOT_DIR, or
    OPENSSL_ROOT_DIR in the Windows preset environment. This lets
    compile_deps/cmake/LegionCoreDeps.cmake point CMake to populated
    compile_deps folders as originally intended.

  * BUILD_WINDOWS.md was updated to explain that compile_deps is used by default
    after setup_deps.ps1, while manual -D..._ROOT overrides still work.

  * compile_deps/README.md was corrected so MariaDB windows-x64/linux-x86_64 are
    described as optional prebuilt connector drop-in folders; the default path is
    the vendored Connector/C source.

  * Added PORTABLE_SERVER_SETUP.md, which explains how to assemble a portable
    server folder after build, including bin/, configs/, data/ClientData/,
    database/mariadb/, logs/, SQL imports, DB user/password setup, config edits,
    ports, startup order, account creation, and backups.

No build was run in Step 10.


17. Step 11 document folder and short setup guide
-------------------------------------------------
Step 11 organized documentation and added a short setup guide.

Changes made:

  * Moved project documents into docs/ while keeping only the GitHub-facing
    README.md at the repository root.

  * Moved files:
      BUILD_WINDOWS.md          -> docs/BUILD_WINDOWS.md
      CHANGELOG.txt             -> docs/CHANGELOG.txt
      LEGION_README.txt         -> docs/LEGION_README.txt
      PORTABLE_SERVER_SETUP.md  -> docs/PORTABLE_SERVER_SETUP.md

  * Added docs/setup_guide.md, a short one-line-per-step setup index pointing to
    the detailed documents.

  * Added docs/README.md as a documentation index.

  * Updated README.md and compile_deps/README.md links to point to the new docs/
    paths.

Recommendations before another build:

  1. Add bnetserver-only CMake build presets and helper scripts.
  2. Add a build-target status document for verified vs experimental targets.
  3. Synchronize and clean worldserver.conf.dist copies before worldserver work.
  4. Add GitHub Actions for the verified bnetserver target.
  5. Add a patch/export guide so the work can be moved without downloading the
     full workspace.

No build was run in Step 11.


18. Step 12 Dev_referance folder
---------------------------------
Step 12 moved maintainer-only reference material out of public docs and into a
new Dev_referance folder.

Changes made:

  * Created Dev_referance/.
  * Moved LEGION_README.txt and CHANGELOG.txt into Dev_referance/.
  * Moved _translation_workspace/ into Dev_referance/.
  * Added Dev_referance/README.md.
  * Copied existing bnetserver configure/build logs into Dev_referance/build_logs/.
  * Updated public docs so normal setup docs remain in docs/, while maintainer
    history/changelog/logs live in Dev_referance/.

From this point forward, build/configure logs should be stored under:

  Dev_referance/build_logs/

No build was run in Step 12.


19. Step 13 README, CMake, and dev-doc verification
---------------------------------------------------
Step 13 verified the public GitHub README, CMake-critical files, and maintainer
reference docs. No build was run.

Changes/verification:

  * README.md now has a clear user-documentation list:
      docs/setup_guide.md
      docs/BUILD_WINDOWS.md
      docs/PORTABLE_SERVER_SETUP.md
      compile_deps/README.md
      compile_deps/DEPENDENCIES.md
      docs/README.md

  * Dev_referance is mentioned only as maintainer-only material, not as a normal
    user setup document.

  * CMakePresets.json parses and `cmake --list-presets=all` works.

  * compile_deps/setup_deps.sh passes syntax check.

  * Root CMake-critical settings were verified:
      CMake 3.18...4.3.2
      Boost 1.83
      MariaDB 10.6.3 minimum probe
      MSVC 19.30 / VS2022
      Win10 API target
      proto Client descriptors included

  * The duplicate Merged-LegionCore-Reforged source tree had stale CMake files;
    those CMake-critical files were synchronized with the root tree.

  * Step 13 static verification log was saved under:
      Dev_referance/build_logs/step13_cmake_static_verification.log

No build or configure/generate was run in Step 13.


20. Step 15 verification, dependency docs, and recommendations
--------------------------------------------------------------
Step 15 performed static verification, added dependency documentation, and added
a maintainer recommendations document. No build was run.

Verification confirmed:

  * required docs exist
  * CMakePresets.json parses
  * cmake --list-presets=all works
  * compile_deps/setup_deps.sh syntax is valid
  * CMake 3.18...4.3.2 is still set
  * Boost floor is 1.83
  * MariaDB minimum is 10.6.3
  * MSVC floor is VS2022 / 19.30
  * Windows API target is Win10 / _WIN32_WINNT=0x0A00
  * root and nested BNet Client protobuf files are present
  * no stale critical references were found

Added:

  * docs/DEPENDENCY_SETUP.md — user-facing dependency/toolchain guide explaining
    what to install and where compile_deps places Boost/OpenSSL/MariaDB.
  * Dev_referance/RECOMMENDATIONS.md — maintainer-facing next-step and custom
    development recommendations.
  * Dev_referance/build_logs/step15_static_verification.log — Step 15 static
    verification log.

Updated links in README.md, docs/README.md, docs/setup_guide.md, and
compile_deps/README.md.


21. Step 16 push explanation and development API reference
----------------------------------------------------------
Step 16 documented whether this assistant can push to GitHub and added a new
maintainer development API/ID reference. No build was run.

GitHub push status:

  * The assistant cannot push to the operator's GitHub unless credentials/token
    or an authenticated remote are deliberately provided.
  * No push was attempted.
  * Safer options are to export patches or have the operator push locally.

Added:

  * Dev_referance/DEVELOPMENT_API_REFERENCE.md

This new document summarizes useful APIs/IDs for future custom development:

  * ScriptMgr script classes
  * PlayerScript hooks
  * CreatureScript / GameObjectScript hooks
  * SpellScript and AuraScript patterns
  * CommandScript pattern
  * movement/path/waypoint IDs
  * common DB ID types
  * ScriptName linkage tables
  * gossip/menu IDs
  * EventMap timed events
  * common Player/Creature/Unit APIs
  * config/logging APIs
  * safe custom script rules
  * suggested custom SQL layout

No build was run in Step 16.


22. Step 22 duplicate tree removal
----------------------------------
Step 22 removed the nested Merged-LegionCore-Reforged/ folder.

Reason:

  * The active server root is merged-reforged_LegionCore-7.3.5/.
  * The nested folder had no .git and was not the active build root.
  * The active root contains all Reforged changed/new merge files.
  * The nested SQL folder had no unique SQL files; root sql/ already contained
    all nested SQL files byte-for-byte and also had the extra TDB base archive.

No build was run in Step 22.
