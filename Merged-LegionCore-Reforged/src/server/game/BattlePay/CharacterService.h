
#ifndef _sCharService
#define _sCharService

#include "ObjectGuid.h"

class WorldSession;
class Player;

class TC_GAME_API CharacterService
{
	CharacterService() = default;
	~CharacterService() = default;
	
public:
    void SetRename(Player* player);
    void ChangeFaction(Player* player);
    void ChangeRace(Player* player);
    void Customize(Player* player);
    void Boost(Player* player);
    void BoostCharacter(WorldSession* session, ObjectGuid targetCharGuid, uint8 targetLevel,
        uint16 overrideMapId = 0, uint16 overrideZoneId = 0,
        float overrideX = 0.f, float overrideY = 0.f, float overrideZ = 0.f, float overrideO = 0.f,
        bool isClassTrial = false, uint16 specId = 0);
    void RestoreDeletedCharacter(WorldSession* session);

	static CharacterService* instance();
};

#define sCharacterService CharacterService::instance()

#endif