#include "Rom.h"
#include "Randomizer.h"

#include <fstream>

using namespace DoaERando;
using namespace std;

#define MAJOR_RELEASE 1
#define MINOR_RELEASE 0
#define PATCH 0

int main(void)
{
    ROM rom;
    rom.ReadRom("./SourceRom/Destiny_of_an_Emperor_USA.nes");
    ifstream fin("./config/seed.cfg");
    uint64_t seed;
    fin >> seed;
    ifstream modeFile("./config/newShuffle");
    bool useNewShuffle = modeFile.good();
    Randomizer randomizer(&rom, seed);

    for (uint8_t i=0; i<0xD9; ++i)
    {
        printf("General 0x%0x is ", i);
        randomizer.PrintName(i);
        printf("\n");
    }

    // This is a non-negotiable - it changes no behavior, but enables other features.
    randomizer.ImproveInitialBattlesAndFlags();
    randomizer.FixNMIGlitch();

    // Optional items
    randomizer.ImproveMap();

    // Need to do this before randomizing the starting generals
    randomizer.MakeAllGeneralsRecruitableAndEncounterable();

    // Need to do this before altering location-exit behavior
    randomizer.RemoveFinalIntroScreen();

    // Non-random fix, but needs to come before
    // ReturnToEntryDirectionOnFlagClear
    randomizer.DisableChapter1LockForSiShui();
    // Randomized features
    if (useNewShuffle)
    {
        printf("Using new shuffle algorithm...\n");
        randomizer.RemoveUneededEntranceTriggerCode();
        randomizer.ReconfigureQingZhouCave();
        randomizer.ReconfigureLiuKuiTentGeneral();
        randomizer.NewGeneralAndBattleShuffle();
        randomizer.MakeZhangBaoAndGuanXingAvailable();
        randomizer.MakeZone0GeneralsUnfirable();
        randomizer.UpdateLiuBeiTurnInTrigger();
        randomizer.ReturnToEntryDirectionOnFlagClear();
        randomizer.MakeAllGeneralsBilletable();
    }
    else
    {
        printf("Using old shuffle algorithm...\n");
        randomizer.RandomizeStartingGenerals();
        randomizer.RandomizeGenerals();
        randomizer.RandomizeBattles();
    }

    randomizer.RandomizeTacticLevels();

    randomizer.ItemShuffle();
    randomizer.CaveShuffle();

    // QoL and other non-random features
    randomizer.IncreaseTacticGains();
    randomizer.MakeIronOreUseful();
    randomizer.MoveGemswordToSimaHuisHut();
    randomizer.MakeIntroLetterUseful();
    randomizer.LetWangGuiTravelToYangZhou();
    randomizer.DoubleXpGain();
    randomizer.DisableNonGeneralBattles();
    randomizer.ResurrectAllAfterDeath();
    randomizer.GivePartyMaxFoodFromNPC();
    randomizer.ModifyEncounterRate();
    randomizer.FixSlot7Glitch();
    randomizer.OnlyLaunchShuLuBuOnce();
    randomizer.RemoveZhugeLiangFetchQuest();
    randomizer.MoveGuiYangBattle();
    randomizer.ProtectChapter1Generals();

    randomizer.RewriteTitleScreen(MAJOR_RELEASE, MINOR_RELEASE, PATCH, seed);
    printf("Saving file...\n");
    rom.WriteRom("./RandoRoms/", MAJOR_RELEASE, MINOR_RELEASE, PATCH, seed);

    return 0;
}
