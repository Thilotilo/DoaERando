#ifndef __RANDOMIZER_H__
#define __RANDOMIZER_H__

#include "Rom.h"
#include <random>

namespace DoaERando {

class Randomizer
{
public:
    Randomizer(ROM* rom, int seed);

    int GetGeneralLocation(BYTE id);
    void PrintName(BYTE id);

    void RandomizeStartingGenerals();
    void ResurrectAllAfterDeath();
    void GivePartyMaxFoodFromNPC();
    void ModifyEncounterRate();

    void RandomizeBattles(std::vector<std::vector<BYTE>>& generalsForZone);

    void SetGeneralRecruitable(BYTE id);
    void SetGeneralEncounterable(BYTE id);

    void MakeAllGeneralsRecruitableAndEncounterable();

    void RandomizeTacticLevels();
    void RandomizeZones();


private:
    Randomizer() = delete;
    ROM* myRom;
    std::mt19937_64 myGenerator;
    
};

}

#endif
