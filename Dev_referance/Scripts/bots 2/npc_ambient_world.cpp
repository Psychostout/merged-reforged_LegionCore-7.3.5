/*
 * npc_ambient_world.cpp
 * Registration-only entry point.
 *
 * The companion system is split across these files:
 *   npc_ambient_world.h        -- shared header (types, enums, declarations)
 *   npc_ambient_data.cpp       -- name pools, speech, zone data, mounts
 *   npc_ambient_companion.cpp  -- CompanionData, DB helpers, roster
 *   npc_ambient_ai.cpp         -- AI state machine + CreatureScript (gossip)
 *   npc_ambient_instance.cpp   -- dungeon/raid entry, auto-fill, replicas
 *   npc_ambient_spawn.cpp      -- PlayerScript, spawning, restore, chat commands
 *   npc_ambient_world.cpp      -- this file (registration only)
 */

#include "npc_ambient_world.h"

// Factory functions defined in their respective .cpp files
class npc_ambient_ai;
npc_ambient_ai* CreateAmbientAIScript();

class AmbientWorldPlayerScript;
AmbientWorldPlayerScript* CreateAmbientPlayerScript();

void AddSC_npc_ambient_world()
{
    CreateAmbientAIScript();
    CreateAmbientPlayerScript();
}
