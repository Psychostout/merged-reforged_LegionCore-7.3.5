# Development Recommendations

This document is for the maintainer. It lists recommended next steps before larger builds and ideas for future development/custom work.

---

## Immediate recommendations before the next build

### 1. Add bnetserver-only build presets/scripts

Priority: high.

Reason: `bnetserver` is the verified target. Dedicated commands reduce the risk of accidentally starting a full worldserver/scripts build.

Suggested files:

```text
scripts/build_bnetserver.sh
scripts/build_bnetserver.ps1
```

Suggested behavior:

```text
- optionally run compile_deps setup
- configure with TOOLS=0 and SCRIPTS=none for a small target test
- build only bnetserver
- write logs to Dev_referance/build_logs/
- use -j1 on low-memory Linux environments
```

### 2. Add docs/BUILD_TARGETS.md

Priority: high.

Purpose: make target status obvious.

Suggested contents:

```text
Verified:
  bnetserver with Boost 1.83

Not yet verified:
  worldserver
  scripts
  full build
  Windows build
```

### 3. Clean/synchronize worldserver config files

Priority: high before worldserver testing.

Check these:

```text
src/server/worldserver/worldserver.conf.dist
Merged-LegionCore-Reforged/src/server/worldserver/worldserver.conf.dist
Dev_referance/_translation_workspace/worldserver.conf.dist.english
```

Goals:

```text
- make sure Solocraft.Enable exists where expected
- ensure database strings are documented
- remove remaining mechanical/French comment fragments if desired
- keep all actual setting lines valid
```

### 4. Decide how to handle the duplicate Merged-LegionCore-Reforged tree

Priority: medium/high.

The repo contains both the main source tree and a duplicate nested tree:

```text
Merged-LegionCore-Reforged/
```

This can confuse users and maintainers. Options:

```text
A. Keep it, but keep it synchronized.
B. Clearly document it as archival/reference only.
C. Remove it later if it is not needed.
```

Do not remove it without checking why it exists in your GitHub repo.

### 5. Add GitHub Actions for bnetserver only

Priority: medium.

Once local bnetserver scripts are stable, add CI:

```text
.github/workflows/bnetserver-linux.yml
```

Start with Linux + Boost 1.83 + bnetserver target only. Add Windows later.

---

## Build/dependency recommendations

### Keep Boost 1.83 for now

The verified bnetserver build uses Boost 1.83. Keep it until the Boost.Asio code is modernized.

Known future modernization point:

```cpp
#include <boost/asio/io_service.hpp>
```

Boost 1.88 does not ship that header.

### Keep CMake targets small while testing

Recommended test order:

```text
1. proto
2. common
3. database
4. shared
5. bnetserver
6. game/worldserver dependencies
7. worldserver without scripts if possible
8. scripts modules in batches
9. full build last
```

### Always log builds

Future logs should go under:

```text
Dev_referance/build_logs/
```

Suggested log names:

```text
step##_configure_<target>.log
step##_build_<target>.log
```

---

## Server/runtime recommendations

### Use a dedicated DB user

Do not run the server as MariaDB root. Use a dedicated user such as:

```text
legion
```

Grant it only the needed databases:

```text
auth
characters
world
hotfixes
```

### Keep portable server separate from source tree

Recommended runtime folder:

```text
LegionPortable/
```

Do not run a live server directly from the build tree if you can avoid it.

### Backup characters often

The most important live database is:

```text
characters
```

Back it up before SQL experiments.

---

## Custom script ideas

Good low-risk custom scripts:

```text
- welcome message / first login helper
- starter gear NPC
- teleporter NPC
- mall/vendor hub
- account-wide quality-of-life commands
- SoloCraft tuning presets
- event announcement NPC
```

Medium-risk custom scripts:

```text
- custom BattlePay shop content
- custom scaling rules
- dungeon/raid solo balancing
- custom world events
- custom progression unlocks
```

High-risk custom work:

```text
- packet/protocol changes
- authentication changes
- cross-realm or networking changes
- database schema rewrites
- modifying core object/session lifetime code
```

Recommendation: start custom work in `src/server/scripts/Custom/` and keep each feature isolated in its own file where possible.

---

## Documentation recommendations

Add these when convenient:

```text
docs/BUILD_TARGETS.md       # verified/unverified target status
docs/SQL_IMPORT_GUIDE.md    # exact SQL import order once confirmed
docs/CONFIG_GUIDE.md        # plain-English ports/database/config settings
docs/CUSTOM_SCRIPTS.md      # how to add scripts safely
Dev_referance/PATCH_EXPORT.md # how to export commits/patches from this workspace
```

---

## Suggested next Step

Recommended Step 16:

```text
Add bnetserver-only build presets and helper scripts, but do not run them yet.
```

This gives a safe, repeatable command for the target that is already known to build.
