#ifndef __RANDOMIZER_H__
#define __RANDOMIZER_H__

#include "Rom.h"
#include "RNG.h"
#include "Generals.h"
#include "BattleRandomizer.h"

namespace DoaERando {

class Randomizer
{
public:
    Randomizer(ROM* rom, int seed);

    int GetGeneralLocation(BYTE id);
    void PrintName(BYTE id);

    // IMPORTANT: MUST BE CALLED AFTER ImproveInitialBattlesAndFlags
    void RandomizeStartingGenerals();

    void ItemShuffle();
    void CaveShuffle();

    void FixSlot7Glitch();
    void FixNMIGlitch();
    void ResurrectAllAfterDeath();
    void OnlyLaunchShuLuBuOnce();
    void GivePartyMaxFoodFromNPC();
    void ModifyEncounterRate();
    void DisableChapter1LockForSiShui();
    void RemoveZhugeLiangFetchQuest();

    void SetGeneralRecruitable(BYTE id);
    void SetGeneralEncounterable(BYTE id);

    void MakeAllGeneralsRecruitableAndEncounterable();

    void RandomizeTacticLevels();
    void RandomizeGenerals();
    void RandomizeBattles();

    // Other QoL Boosts
    void DisableNonGeneralBattles();
    void IncreaseTacticGains();
    void DoubleXpGain();

    // For Freeform Map
    void ImproveInitialBattlesAndFlags();
    void ImproveMap();

    // Newest General Shuffle
    void SetGeneralForZone0AndRemove(BYTE generalId, std::vector<BYTE>& ids);
    void NewGeneralAndBattleShuffle();

private:
    Randomizer() = delete;
    ROM* myRom;
    RNG myRNG;
    Generals myGenerals;
    BattleRandomizer myBattleRandomizer;
    
};

}

#endif
