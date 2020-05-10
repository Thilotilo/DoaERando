#include "Randomizer.h"
#include <stdio.h>

using namespace std;

namespace DoaERando {

Randomizer::Randomizer(ROM* rom, int seed)
    : myRom(rom)
    , myGenerator(seed)
{
}

int Randomizer::GetGeneralLocation(BYTE id)
{
    const int BANK_OFFSET = 0x30010 - 0x8000;
    const int GENERAL_ADDRESSES_LSB_START = 0x32610;
    const int GENERAL_ADDRESSES_MSB_START = 0x32710;
    int msb = myRom->ReadByte(GENERAL_ADDRESSES_MSB_START + id);
    int lsb = myRom->ReadByte(GENERAL_ADDRESSES_LSB_START + id);
    int address = (msb << 8) + lsb + BANK_OFFSET;
    return address;
}

void Randomizer::PrintName(BYTE id)
{
    int nameOffset = GetGeneralLocation(id) + 20;
    while (myRom->ReadByte(nameOffset) != 0xFF)
    {
        char character = myRom->ReadByte(nameOffset) + 'A' - 0x10;
        switch(myRom->ReadByte(nameOffset))
        {
            case 0x2D:
                character = 'r';
                break;
            case 0x2F:
                character = 't';
                break;
            case 0x64:
                character = 'u';
                break;
            case 0x0A:
                character = ' ';
                break;
            default:
                break;
        }

        printf("%c", character);
        ++nameOffset;
    }
}

void Randomizer::RandomizeStartingGenerals()
{
    printf("Randomizing Starting Generals...\n");
    uniform_int_distribution<int> distribution(0x05, 0xD8);

    BYTE general = distribution(myGenerator);
    if (general != 0xB3 && general != 0xA6 && general != 0xA7 && general != 0xA8)
    {
        printf("Setting ");
        PrintName(general);
        printf(" as a starting general\n");
        myRom->WriteByte(0x35558, general);
        
    }
    general = distribution(myGenerator);
    if (general != 0xB3 && general != 0xA6 && general != 0xA7 && general != 0xA8)
    {
        printf("Setting ");
        PrintName(general);
        printf(" as a starting general\n");
        myRom->WriteByte(0x35559, general);
    }
}

void Randomizer::ResurrectAllAfterDeath()
{
    printf("Resurrecting all after death...\n");
    myRom->WriteByte(0x36145, 0x01);
    myRom->WriteByte(0x36149, 0xD9);
}

void Randomizer::RandomizeBattles(vector<vector<BYTE>>& generalsForZone)
{
    vector<int> zoneForBattle = {1, 1, 1, 1, 1, 1,
                           2, 2, 2, 2, 2, 2, 2, 2,
                           3, 3, 3, 3, 3, 3,
                           4, 4, 4, 4, 4, 4, 4,
                           5, 5, 5, 5, 5,
                           6, 6, 6, 6, 6, 6, 6, 6,
                           7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                           8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                           4, 4,
                           5, 5,
                           7, 7, 8, 6};
    printf("Sizes of generalsPerZone");
    int c = 0;
    for (auto& s : generalsForZone)
    {
        printf("%d: %ld\n", c++, s.size());
    }

    for (int i = 0; i < zoneForBattle.size(); ++i)
    {
        const int BATTLE0_OFFSET = 0x30D70;
        geometric_distribution<int> distribution(0.3);
        int value;
        while ((value = distribution(myGenerator)) > 4);
        int numGenerals = value + 1;
        int numRebelForces = 0;
        if (numGenerals < 5)
        {
            uniform_int_distribution<int> rebelForceDistribution(1, 5 - numGenerals);
            numRebelForces = rebelForceDistribution(myGenerator);
        }

        int curPos = BATTLE0_OFFSET + i*5;

        for (int g = 0; g < 5; ++g)
        {
            if (myRom->ReadByte(curPos + g) == 0x88 || myRom->ReadByte(curPos + g) == 0x71)
                continue;
            if (g < numGenerals)
            {
                uniform_int_distribution<int> generalDistribution(0, generalsForZone[zoneForBattle[i]].size() - 1);
                int zone = zoneForBattle[i];
                int value = generalDistribution(myGenerator);
                //printf("trying to find general %d in zone %d\n", value, zone);
                BYTE general = generalsForZone[zone][value] & 0xFF;
                //printf("Found general 0x%02X\n", general & 0xFF);
                //BYTE general = generalsForZone[zoneForBattle[i]][generalDistribution(myGenerator)] & 0xFF;
                myRom->WriteByte(curPos + g, general);
            }
            else if (g < (numGenerals + numRebelForces))
            {
                myRom->WriteByte(curPos + g, 0x04);
            }
            else
            {
                myRom->WriteByte(curPos + g, 0x00);
            }
        }

        printf("Battle %d is now: ", i);
        for (int g = 0; g < 5; ++g)
        {
            if (myRom->ReadByte(curPos + g) != 0)
            {
                PrintName(myRom->ReadByte(curPos + g));
                printf("   ");
            }
        }
        printf("\n");

    }
}

void Randomizer::SetGeneralRecruitable(BYTE id)
{
    const int BASE_ADDRESS = 0x3b153;
    const BYTE bitmask = 1<<(7-(id%8));
    BYTE newByte = myRom->ReadByte(BASE_ADDRESS + (id/8)) | bitmask;
    myRom->WriteByte(BASE_ADDRESS + (id/8), newByte);
}

void Randomizer::SetGeneralEncounterable(BYTE id)
{
    const int BASE_ADDRESS = 0x35722;
    const BYTE bitmask = 1<<(7-(id%8));
    BYTE newByte = myRom->ReadByte(BASE_ADDRESS + (id/8)) | bitmask;
    myRom->WriteByte(BASE_ADDRESS + (id/8), newByte);
}

void Randomizer::MakeAllGeneralsRecruitableAndEncounterable()
{
    for (BYTE id = 0x05; id < 0xD9; ++id)
    {
        SetGeneralRecruitable(id);
        SetGeneralEncounterable(id);
    }

    //Allow any General that appears in a town and isn't already on your party to show up.
    myRom->WriteByte(0x3E8F0, 0xC0); // Compare general config with 0xC0 rather than 0x80
    myRom->WriteByte(0x3E8F1, 0xD0); // Branch if != 0xC0 rather than branching if == 0x80 
}

void Randomizer::RandomizeTacticLevels()
{
    std::uniform_int_distribution<int> levelSlot(1, 30);

    const int TACTIC_LEVEL_OFFSET = 0x3b880;
    printf("Old Tactic Levels:\n");
    for(int i = 1; i < 31; ++i)
    {
        printf("  Level %d: Tactic id 0x%X\n", i + 1, myRom->ReadByte(TACTIC_LEVEL_OFFSET + i));
    }
    for (int i = 0; i < 10000; ++i)
    {
        int levelSlot1 = levelSlot(myGenerator);
        int levelSlot2 = levelSlot(myGenerator);

        myRom->SwapBytes(TACTIC_LEVEL_OFFSET + levelSlot1, TACTIC_LEVEL_OFFSET + levelSlot2);
    }

    printf("New Tactic Levels:\n");
    for(int i = 1; i < 31; ++i)
    {
        printf("  Level %d: Tactic id 0x%X\n", i + 1, myRom->ReadByte(TACTIC_LEVEL_OFFSET + i));
    }
}

void Randomizer::RandomizeZones()
{
    std::uniform_int_distribution<int> zoneDistribution(1,8);
    vector<vector<BYTE>> generalsForZone;
    generalsForZone.resize(9);
    bool randomizeGeneral[0xD9];
    for (BYTE i = 0; i < 0xD9; ++i)
    {
        switch (i)
        {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
            case 0x39:
            case 0x3A:
            case 0x3C:
            case 0x45:
            case 0x55:
            case 0x98:
            case 0x9A:
            case 0x9B:
            case 0x9C:
            case 0xA1:
            case 0xA2:
            case 0xA3:
            case 0xA6:
            case 0xA7:
            case 0xB3:
            case 0xB5:
            case 0xB6:
            case 0x71: // Cao Pi - don't randomize for now
            case 0x88: // Sima Yi - don't randomize for now
                randomizeGeneral[i] = false;
                break;
            default:
                randomizeGeneral[i] = true;
                break;
        }

        if (randomizeGeneral[i])
        {
            const BYTE MIN_SOLDIERS[9] = {1,   10,  24, 33, 48, 53, 64, 82,  99};
            const BYTE MAX_SOLDIERS[9] = {101, 27,  32, 47, 61, 60, 79, 111, 121};
            const BYTE MIN_WEAPON[9] = {0x00, 0x00, 0x04, 0x05, 0x06, 0x06, 0x09, 0x0B, 0x0E};
            const BYTE MAX_WEAPON[9] = {0x13, 0x05, 0x05, 0x06, 0x06, 0x07, 0x09, 0x0D, 0x13};
            const BYTE MAX_TACTIC[9] = {0x31, 0x06, 0x09, 0x09, 0x14, 0x15, 0x24, 0x26, 0x31};
            const BYTE MIN_ARMOR[9] = {0x0, 0x0, 0x4, 0x5, 0x4, 0x7, 0xA, 0xC, 0xE};
            const BYTE MAX_ARMOR[9] = {0xF, 0x3, 0x5, 0x6, 0x5, 0x7, 0xB, 0xF, 0xF};
            BYTE randomizedZone = zoneDistribution(myGenerator);

            int generalLocation = GetGeneralLocation(i);

            int zoneOffset = generalLocation + 2;
            {
                printf("Moving ");
                PrintName(i);
                printf(" from zone %d to zone %d\n", myRom->ReadByte(zoneOffset), randomizedZone);
                myRom->WriteByte(zoneOffset, randomizedZone);
                generalsForZone[randomizedZone].push_back(i);
            }
            int soldiersOffset = generalLocation + 6;
            std::uniform_int_distribution<int> soldierDistribution(MIN_SOLDIERS[randomizedZone], MAX_SOLDIERS[randomizedZone]);
            BYTE randomizedSoldiers = soldierDistribution(myGenerator);
            {
                if (myRom->ReadByte(soldiersOffset) & 0x80)
                {
                    printf("Tiger detected, not modifying the first value.\n");
                    printf("Changing ");
                    PrintName(i);
                    printf("'s enemy soldier exponent from %d to %d\n", myRom->ReadByte(soldiersOffset), randomizedSoldiers);
                    myRom->WriteByte(soldiersOffset, randomizedSoldiers);
                }
                else
                {
                    printf("Changing ");
                    PrintName(i);
                    printf("'s soldier exponent from %d (and %d) to %d\n", myRom->ReadByte(soldiersOffset), myRom->ReadByte(soldiersOffset+1), randomizedSoldiers);
                    myRom->WriteByte(soldiersOffset, randomizedSoldiers);
                    myRom->WriteByte(soldiersOffset + 1, randomizedSoldiers);
                }
            }
            std::uniform_int_distribution<int> tacticDistribution(0, MAX_TACTIC[randomizedZone]);
            std::uniform_int_distribution<int> weaponDistribution(MIN_WEAPON[randomizedZone], MAX_WEAPON[randomizedZone]);
            std::uniform_int_distribution<int> armorDistribution(MIN_ARMOR[randomizedZone], MAX_ARMOR[randomizedZone]);

            int tacticOffset = generalLocation + 8;
            int weaponOffset = generalLocation + 4;
            int armorOffset = generalLocation + 5;

            myRom->WriteByte(tacticOffset, tacticDistribution(myGenerator));
            myRom->WriteByte(armorOffset, armorDistribution(myGenerator));
            BYTE weaponVal = myRom->ReadByte(weaponOffset) & 0xE0;
            myRom->WriteByte(weaponOffset, weaponVal + weaponDistribution(myGenerator));
        }

    }
    RandomizeBattles(generalsForZone);
}


}
