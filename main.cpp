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
    Randomizer randomizer(&rom, seed);

    for (uint8_t i=0; i<0xD9; ++i)
    {
        printf("General 0x%0x is ", i);
        randomizer.PrintName(i);
        printf("\n");
    }

    randomizer.RandomizeZones();
    randomizer.RandomizeTacticLevels();
    randomizer.RandomizeStartingGenerals();
    randomizer.ResurrectAllAfterDeath();
    randomizer.MakeAllGeneralsRecruitableAndEncounterable();
    randomizer.GivePartyMaxFoodFromNPC();
    randomizer.ModifyEncounterRate();

    printf("Saving file...\n");
    rom.WriteRom("./RandoRoms/", 0, seed);

    return 0;
}
