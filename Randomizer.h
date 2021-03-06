#ifndef __RANDOMIZER_H__
#define __RANDOMIZER_H__

#include "Rom.h"
#include "RNG.h"
#include "Generals.h"
#include "BattleRandomizer.h"
#include "NPCManipulator.h"

namespace DoaERando {

class Randomizer
{
public:
    Randomizer(ROM* rom, int seed);

    int GetGeneralLocation(BYTE id);
    void PrintName(BYTE id);

    // IMPORTANT: MUST BE CALLED AFTER ImproveInitialBattlesAndFlags
    void RandomizeStartingGenerals();

    void RewriteTitleScreen(int major, int minor, int patch, int seed);
    void RemoveFinalIntroScreen();

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
    void MakeZhangBaoAndGuanXingAvailable();

    void SetGeneralRecruitable(BYTE id);
    void SetGeneralEncounterable(BYTE id);

    void MakeAllGeneralsRecruitableAndEncounterable();

    void RandomizeTacticLevels();
    void RandomizeGenerals();
    void RandomizeBattles();

    // Other QoL Boosts
    void UpdateGeneralNames();
    void DisableNonGeneralBattles();
    void IncreaseTacticGains();
    void DoubleXpGain();
    void MakeIronOreUseful();
    void MoveGemswordToSimaHuisHut();
    void MakeIntroLetterUseful();
    void LetWangGuiTravelToYangZhou();
    void ProtectChapter1Generals();

    // For Freeform Map
    void ImproveInitialBattlesAndFlags();
    void ImproveMap();
    void MoveGuiYangBattle();
    // IMPORTANT:
    // 1) MUST BE CALLED AFTER RemoveFinalIntroScreen
    // 2) DisableChapter1LockForSiShui may not be called after this is called.
    void ReturnToEntryDirectionOnFlagClear();

    // Newest General Shuffle
    void SetGeneralForZone0AndRemove(BYTE generalId, std::vector<BYTE>& ids);
    void NewGeneralAndBattleShuffle();
    void ReconfigureQingZhouCave();
    void ReconfigureLiuKuiTentGeneral();
    void MakeZone0GeneralsUnfirable();
    void RemoveUneededEntranceTriggerCode();
    void UpdateLiuBeiTurnInTrigger();
    void MakeAllGeneralsBilletable();
    void MakeWarlordsImmuneToAnSha();

private:
    Randomizer() = delete;
    ROM* myRom;
    RNG myRNG;
    Generals myGenerals;
    BattleRandomizer myBattleRandomizer;
    NPCManipulator myNPCManipulator;
    
};

}

#endif
