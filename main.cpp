#include "Rom.h"
#include "Randomizer.h"

#include <fstream>

using namespace DoaERando;
using namespace std;

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
    randomizer.DoubleXpGain();
    randomizer.DisableNonGeneralBattles();
    randomizer.ResurrectAllAfterDeath();
    randomizer.GivePartyMaxFoodFromNPC();
    randomizer.ModifyEncounterRate();
    randomizer.FixSlot7Glitch();
    randomizer.OnlyLaunchShuLuBuOnce();
    randomizer.DisableChapter1LockForSiShui();
    randomizer.RemoveZhugeLiangFetchQuest();
    randomizer.MoveGuiYangBattle();

    printf("Saving file...\n");
    rom.WriteRom("./RandoRoms/", 14, seed);

    return 0;
}
