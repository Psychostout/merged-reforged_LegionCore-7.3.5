# Development API / IDs Reference

Maintainer note: this is a practical reference for future custom modules and scripts in this LegionCore 7.3.5 tree. It is not exhaustive API documentation, but it lists the important script APIs, ID types, DB tables, and file paths you are most likely to need.

Current verified target: `bnetserver` builds with Boost 1.83. Full `worldserver`/scripts build is not yet verified in this recovery pass.

---

## 1. Where custom scripts go

Primary custom script folder:

```text
src/server/scripts/Custom/
```

Current restored custom files:

```text
src/server/scripts/Custom/Solocraft.cpp
src/server/scripts/Custom/custom_script_loader.cpp
```

Typical custom layout:

```text
src/server/scripts/Custom/
├── custom_script_loader.cpp
├── Solocraft.cpp
├── npc_teleporter.cpp
├── npc_starter_gear.cpp
├── player_login_rewards.cpp
└── custom_events.cpp
```

`custom_script_loader.cpp` should call every custom script registration function:

```cpp
void AddSC_solocraft();
void AddSC_npc_teleporter();
void AddSC_player_login_rewards();

void AddCustomScripts()
{
    AddSC_solocraft();
    AddSC_npc_teleporter();
    AddSC_player_login_rewards();
}
```

Each script file usually ends with:

```cpp
void AddSC_script_name()
{
    new my_script_class();
}
```

---

## 2. Main script API header

The main script API is declared here:

```text
src/server/game/Scripting/ScriptMgr.h
```

Important script base classes found there:

```text
SpellScriptLoader
WorldScript
FormulaScript
BattlePayProductScript
MapScript
WorldMapScript
InstanceMapScript
BattlegroundMapScript
ItemScript
CreatureScript
GameObjectScript
AreaTriggerScript
OnlyOnceAreaTriggerScript
SceneTriggerScript
EventObjectScript
BattlegroundScript
OutdoorPvPScript
CommandScript
WeatherScript
AuctionHouseScript
ConditionScript
VehicleScript
DynamicObjectScript
TransportScript
AchievementCriteriaScript
AchievementRewardScript
PlayerScript
SessionScript
GuildScript
GroupScript
WorldStateScript
```

For most custom work you will use:

```text
CreatureScript      NPC AI, gossip menus, quest NPC behavior
GameObjectScript    clickable objects, gameobject gossip, quest objects
PlayerScript        login/logout, level, money, XP, chat, quest, kill hooks
CommandScript       custom GM/player commands
SpellScriptLoader   spell and aura behavior
WorldScript         server/world lifecycle hooks
InstanceMapScript   dungeon/raid instance logic
```

---

## 3. Common PlayerScript hooks

Useful hooks from `PlayerScript`:

```cpp
OnLogin(Player* player)
OnLogin(Player* player, bool firstLogin)
OnLogout(Player* player)
OnCreate(Player* player)
OnDelete(ObjectGuid const& guid)
OnLevelChanged(Player* player, uint8 oldLevel)
OnGiveXP(Player* player, uint32& amount, Unit* victim)
OnMoneyChanged(Player* player, int64& amount)
OnReputationChange(Player* player, uint32 factionId, int32& standing, bool incremental)
OnQuestComplete(Player* player, Quest const* quest)
OnQuestReward(Player* player, Quest const* quest)
OnCreatureKill(Player* killer, Creature* killed)
OnPVPKill(Player* killer, Player* killed)
OnPlayerKilledByCreature(Creature* killer, Player* killed)
OnSpellCast(Player* player, Spell* spell, bool skipCheck)
OnSpellLearned(Player* player, uint32 spellID)
OnUpdateZone(Player* player, uint32 newZone, uint32 newArea)
OnMapChanged(Player* player)
OnMovementInform(Player* player, uint32 moveType, uint32 ID)
OnChat(Player* player, uint32 type, uint32 lang, std::string& msg)
OnLootItem(Player* player, Item* item, uint32 count)
OnCreateItem(Player* player, Item* item, uint32 count)
OnQuestRewardItem(Player* player, Item* item, uint32 count)
```

Example use cases:

```text
OnLogin              welcome message, account rewards, first-login setup
OnLevelChanged       level-up rewards, scaling rules
OnGiveXP             XP multipliers or XP blocking
OnMoneyChanged       gold cap / tax / reward systems
OnQuestComplete      custom progression unlocks
OnCreatureKill       kill rewards, custom currencies
OnChat               chat filter or custom chat commands
OnUpdateZone         zone-based buffs/events
```

---

## 4. CreatureScript / NPC API

Use `CreatureScript` for NPC behavior.

Important hooks:

```cpp
OnGossipHello(Player* player, Creature* creature)
OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code)
OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
OnQuestSelect(Player* player, Creature* creature, Quest const* quest)
OnQuestComplete(Player* player, Creature* creature, Quest const* quest)
OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt)
GetDialogStatus(Player* player, Creature* creature)
GetAI(Creature* creature) const
```

Common NPC AI base classes:

```text
ScriptedAI
BossAI
npc_escortAI
PassiveAI
CombatAI
```

Common AI methods:

```cpp
Reset()
EnterCombat(Unit* who)
JustDied(Unit* killer)
JustSummoned(Creature* summon)
KilledUnit(Unit* victim)
DamageTaken(Unit* attacker, uint32& damage)
SpellHit(Unit* caster, SpellInfo const* spell)
MovementInform(uint32 type, uint32 id)
UpdateAI(uint32 diff)
```

NPC script DB link:

```sql
UPDATE creature_template SET ScriptName = 'your_script_name' WHERE entry = YOUR_CREATURE_ENTRY;
```

---

## 5. GameObjectScript API

Use `GameObjectScript` for clickable objects.

Important hooks:

```cpp
OnGossipHello(Player* player, GameObject* go)
OnGossipSelect(Player* player, GameObject* go, uint32 sender, uint32 action)
OnGossipSelectCode(Player* player, GameObject* go, uint32 sender, uint32 action, const char* code)
OnQuestAccept(Player* player, GameObject* go, Quest const* quest)
OnQuestReward(Player* player, GameObject* go, Quest const* quest, uint32 opt)
GetDialogStatus(Player* player, GameObject* go)
```

GameObject script DB link:

```sql
UPDATE gameobject_template SET ScriptName = 'your_go_script_name' WHERE entry = YOUR_GAMEOBJECT_ENTRY;
```

---

## 6. SpellScript / AuraScript API

Use `SpellScriptLoader` for custom spell behavior.

Common pattern:

```cpp
class spell_custom_example : public SpellScriptLoader
{
public:
    spell_custom_example() : SpellScriptLoader("spell_custom_example") { }

    class spell_custom_example_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_custom_example_SpellScript);

        void HandleHit(SpellEffIndex effIndex)
        {
            Unit* caster = GetCaster();
            Unit* target = GetHitUnit();
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_custom_example_SpellScript::HandleHit, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_custom_example_SpellScript();
    }
};

void AddSC_spell_custom()
{
    new spell_custom_example();
}
```

Common macros/functions:

```text
PrepareSpellScript(classname)
PrepareAuraScript(classname)
SpellEffectFn(...)
AuraEffectApplyFn(...)
AuraEffectRemoveFn(...)
OnEffectHit
OnEffectHitTarget
OnObjectAreaTargetSelect
AfterCast
BeforeCast
```

Spell script DB link:

```sql
INSERT INTO spell_script_names (spell_id, ScriptName)
VALUES (12345, 'spell_custom_example');
```

Important ID:

```text
spell_id = Spell ID from DBC/DB2/client data
ScriptName = C++ SpellScriptLoader name string
```

---

## 7. CommandScript API

Use `CommandScript` for GM/player commands.

Typical file examples:

```text
src/server/scripts/Commands/cs_account.cpp
src/server/scripts/Commands/cs_misc.cpp
src/server/scripts/Commands/cs_lookup.cpp
```

Common pattern:

```cpp
class custom_commandscript : public CommandScript
{
public:
    custom_commandscript() : CommandScript("custom_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> commands =
        {
            { "customtest", SEC_GAMEMASTER, false, &HandleCustomTestCommand, "" }
        };
        return commands;
    }

    static bool HandleCustomTestCommand(ChatHandler* handler, char const* args)
    {
        handler->PSendSysMessage("Custom command works.");
        return true;
    }
};
```

Security levels are account/GM levels. Check existing command scripts for exact constants used in this branch.

---

## 8. Movement, path IDs, waypoint IDs

There are two common meanings for "path IDs":

1. **Code movement point IDs** used in `MovePoint` / `MovementInform`.
2. **Database waypoint/path IDs** used in waypoint tables.

### Code movement point IDs

Common methods:

```cpp
me->GetMotionMaster()->MovePoint(pointId, x, y, z);
me->GetMotionMaster()->MoveChase(target);
me->GetMotionMaster()->MoveFollow(target, distance, angle);
me->GetMotionMaster()->MoveRandom(radius);
me->GetMotionMaster()->MoveTargetedHome();
me->GetMotionMaster()->MoveIdle();
me->GetMotionMaster()->Clear();
```

Handle arrival:

```cpp
void MovementInform(uint32 type, uint32 id) override
{
    if (type == POINT_MOTION_TYPE && id == POINT_INTRO)
    {
        // reached point
    }
}
```

Recommended custom point ID style:

```cpp
enum MovementPoints
{
    POINT_INTRO       = 1,
    POINT_PHASE_ONE   = 2,
    POINT_RETURN_HOME = 3
};
```

Avoid reusing the same point ID for different logic inside one AI.

### Escort/waypoint API

`npc_escortAI` supports:

```cpp
AddWaypoint(id, x, y, z, waitTime);
Start(...);
WaypointReached(uint32 waypointId);
WaypointStart(uint32 waypointId);
```

Relevant files:

```text
src/server/game/AI/ScriptedAI/ScriptedEscortAI.cpp
src/server/game/AI/ScriptedAI/ScriptedEscortAI.h
```

### Database waypoint/script waypoint tables

Common DB tables used by scripts/movement:

```text
script_waypoint(entry, pointid, location_x, location_y, location_z, waittime, point_comment)
waypoint_data / waypoint_scripts   # if present in your imported world DB
creature.MovementID
creature.currentwaypoint
creature_template_movement
creature_movement_override
```

Important ID concepts:

```text
entry       = creature_template entry ID
pointid     = waypoint number/order
MovementID  = DB movement/path reference used by creature spawns/templates
map         = Map ID where the creature/object exists
position_x/y/z/orientation = world coordinates
```

---

## 9. Important database ID types

Most custom scripts combine C++ logic with DB IDs.

Common IDs:

```text
Creature entry ID       creature_template.entry
Creature spawn GUID     creature.guid
GameObject entry ID     gameobject_template.entry
GameObject spawn GUID   gameobject.guid
Spell ID                Spell.dbc/db2 ID, spell_script_names.spell_id
Quest ID                quest_template.ID or quest_template.Id depending on DB schema
Item ID                 item_sparse/item_template/hotfix item ID depending on subsystem
Map ID                  Map.dbc/db2 ID
Zone ID                 AreaTable/zone ID
Area ID                 AreaTable area ID
Faction ID              Faction.dbc/db2 ID
Scene ID                scene template/DB2 related IDs
Conversation ID         conversation template IDs where supported
BroadcastText ID        broadcast text hotfix/world DB ID
Phase ID                phase-related DB/hotfix ID
Condition ID/value      conditions table source/value fields
SmartAI entryorguid     smart_scripts.entryorguid
```

Always verify the exact table/column names in your imported DB because Legion-era schemas can differ between dumps.

---

## 10. ScriptName database connections

Common `ScriptName` linkage points:

```sql
-- Creature script
UPDATE creature_template
SET ScriptName = 'npc_custom_name'
WHERE entry = 123456;

-- GameObject script
UPDATE gameobject_template
SET ScriptName = 'go_custom_name'
WHERE entry = 234567;

-- Spell script
INSERT INTO spell_script_names (spell_id, ScriptName)
VALUES (345678, 'spell_custom_name');
```

Other tables that may use scripts or script-like names depending on schema:

```text
areatrigger_scripts
conditions.ScriptName
game_event scripts/tables
smart_scripts
event_scripts
quest scripts
spell_area
```

---

## 11. Gossip / menu IDs

Gossip work usually involves one of two approaches:

1. Pure C++ menu built in `OnGossipHello`.
2. DB-driven gossip menu IDs and NPC text.

Common C++ calls vary by branch, but existing scripts show the expected style.
Search examples:

```bash
grep -R "ADD_GOSSIP_ITEM\|AddGossipItem\|SendGossipMenu" src/server/scripts -n
```

Common DB concepts:

```text
gossip_menu.menu_id
gossip_menu.text_id
npc_text.ID
gossip_menu_option.menu_id
gossip_menu_option.id
creature_template.gossip_menu_id
```

---

## 12. EventMap and timed events

Many creature/boss scripts use `EventMap`.

Common pattern:

```cpp
EventMap events;

enum Events
{
    EVENT_CAST_ONE = 1,
    EVENT_CAST_TWO = 2
};

void Reset() override
{
    events.Reset();
}

void EnterCombat(Unit* /*who*/) override
{
    events.ScheduleEvent(EVENT_CAST_ONE, 5s);
}

void UpdateAI(uint32 diff) override
{
    if (!UpdateVictim())
        return;

    events.Update(diff);

    while (uint32 eventId = events.ExecuteEvent())
    {
        switch (eventId)
        {
            case EVENT_CAST_ONE:
                DoCastVictim(SPELL_EXAMPLE);
                events.ScheduleEvent(EVENT_CAST_ONE, 10s);
                break;
        }
    }

    DoMeleeAttackIfReady();
}
```

Prefer script-local enum values for events and movement points.

---

## 13. Useful core object APIs

Common object accessors used in scripts:

```cpp
Player* player
Creature* creature
GameObject* go
Unit* unit
Map* map
InstanceScript* instance
WorldSession* session
```

Useful calls:

```cpp
player->GetGUID()
player->getLevel()
player->GetMapId()
player->GetZoneId()
player->GetAreaId()
player->TeleportTo(mapId, x, y, z, o)
player->AddItem(itemId, count)
player->CastSpell(target, spellId, true)
player->KilledMonsterCredit(entry)
player->ModifyMoney(amount)
player->GetSession()

creature->GetEntry()
creature->GetGUID()
creature->SummonCreature(entry, x, y, z, o, type, despawnTime)
creature->DespawnOrUnsummon(delay)
creature->AI()->Talk(id)

unit->CastSpell(target, spellId, triggered)
unit->HasAura(spellId)
unit->AddAura(spellId, target)
unit->RemoveAura(spellId)
unit->GetHealthPct()
```

Check the actual class headers before using a method, because signatures can differ by branch.

---

## 14. Important include paths for scripts

Common includes:

```cpp
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "Creature.h"
#include "GameObject.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Chat.h"
#include "ObjectMgr.h"
#include "World.h"
#include "Config.h"
#include "Map.h"
#include "InstanceScript.h"
#include "MotionMaster.h"
#include "TemporarySummon.h"
```

Do not include heavy headers unless needed. Prefer examples from nearby scripts.

---

## 15. Config API

Use `sConfigMgr` for config values.

Examples:

```cpp
bool enabled = sConfigMgr->GetBoolDefault("Custom.Enable", false);
uint32 value = sConfigMgr->GetIntDefault("Custom.Value", 1);
float rate = sConfigMgr->GetFloatDefault("Custom.Rate", 1.0f);
std::string text = sConfigMgr->GetStringDefault("Custom.Text", "default");
```

Custom config keys should be documented in `worldserver.conf.dist` and should use a clear prefix:

```text
Custom.Teleporter.Enable
Custom.StarterGear.Enable
Solocraft.Enable
```

---

## 16. Logging API

Common logging style:

```cpp
TC_LOG_INFO("server.loading", "Custom script loaded");
TC_LOG_ERROR("sql.sql", "Missing custom DB row for entry %u", entry);
TC_LOG_DEBUG("scripts", "Debug value: %u", value);
```

Use existing log categories where possible:

```text
server.loading
sql.sql
scripts
misc
entities.player
spells
maps
```

---

## 17. Safe custom development rules

Recommended rules:

```text
1. Put custom scripts under src/server/scripts/Custom/.
2. Keep each feature in its own .cpp file.
3. Add one AddSC_* function per feature.
4. Register it in custom_script_loader.cpp.
5. Put custom DB changes in sql/custom/.
6. Prefix custom config keys clearly.
7. Avoid editing core systems unless a script cannot solve the problem.
8. Back up character DB before testing SQL changes.
9. Test bnetserver separately from worldserver.
10. Keep build logs in Dev_referance/build_logs/.
```

---

## 18. Suggested custom SQL folder convention

Recommended:

```text
sql/custom/my_server/
├── 0001_custom_npcs.sql
├── 0002_custom_items.sql
├── 0003_custom_spells.sql
├── 0004_custom_teleporter.sql
└── README.md
```

Include comments in every SQL file:

```sql
-- Adds custom teleporter NPC
-- Requires C++ ScriptName: npc_custom_teleporter
-- Creature entry: 900000
```

Use high custom IDs that do not collide with existing data. Pick a range and document it, for example:

```text
Custom creature entries: 900000-909999
Custom gameobject entries: 910000-919999
Custom item IDs: check hotfix/item system before choosing
Custom spell scripts: use real spell IDs unless creating hotfix spell data
```

Do not assume these ranges are safe forever; search the DB first.

---

## 19. Files to inspect before adding new systems

```text
src/server/game/Scripting/ScriptMgr.h
src/server/game/AI/ScriptedAI/ScriptedCreature.h
src/server/game/AI/ScriptedAI/ScriptedEscortAI.h
src/server/game/Entities/Player/Player.h
src/server/game/Entities/Creature/Creature.h
src/server/game/Entities/Unit/Unit.h
src/server/game/Spells/SpellScript.h
src/server/scripts/Commands/
src/server/scripts/Custom/
src/server/scripts/Spells/
sql/custom/
sql/updates/world/
```

---

## 20. Known branch-specific notes

```text
- Boost 1.83 is currently the verified Boost version.
- bnetserver is verified with a targeted build.
- Full worldserver/scripts build is not yet verified in this recovery pass.
- BNet protobuf generated files are present under src/server/proto/Client/.
- SoloCraft is restored and disabled by default with Solocraft.Enable = 0.
- Dev logs should go under Dev_referance/build_logs/.
```
