#ifndef __RANDOMIZER_H__
#define __RANDOMIZER_H__

#include "Rom.h"
#include <random>
#include "Generals.h"

namespace DoaERando {

class Randomizer
{
public:
    Randomizer(ROM* rom, int seed);

    int GetGeneralLocation(BYTE id);
    void PrintName(BYTE id);

    void RandomizeStartingGenerals();

    void FixSlot7Glitch();
    void ResurrectAllAfterDeath();
    void GivePartyMaxFoodFromNPC();
    void ModifyEncounterRate();

    //void RandomizeBattles(std::vector<std::vector<BYTE>>& generalsForZone);

    void SetGeneralRecruitable(BYTE id);
    void SetGeneralEncounterable(BYTE id);

    void MakeAllGeneralsRecruitableAndEncounterable();

    void RandomizeTacticLevels();
    void RandomizeGenerals();
    //void RandomizeZones();

    // For Freeform Map
    void ImproveInitialBattlesAndFlags();
    void ImproveMap();


private:
    Randomizer() = delete;
    ROM* myRom;
    Generator myGenerator;
    Generals myGenerals;
    
};

}

#endif
