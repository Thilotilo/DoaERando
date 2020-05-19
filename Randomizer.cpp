#include "Randomizer.h"
#include <stdio.h>
#include "Generals.h"
#include "General.h"

using namespace std;

namespace DoaERando {

Randomizer::Randomizer(ROM* rom, int seed)
    : myRom(rom)
    , myGenerator(seed)
    , myGenerals(*rom)
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
    uniform_int_distribution<int> distribution(0x05, 0xD7);

    std::vector<BYTE> startingGenerals;
    startingGenerals.push_back(0xA8);

    BYTE general = distribution(myGenerator);
    if (general != 0xB3 && general != 0xA6 && general != 0xA7 && general != 0xA8)
    {
        printf("Setting ");
        PrintName(general);
        printf(" as a starting general\n");
        myRom->WriteByte(0x35558, general);
        startingGenerals.push_back(general);
        
    }
    general = distribution(myGenerator);
    if (general != 0xB3 && general != 0xA6 && general != 0xA7 && general != 0xA8)
    {
        printf("Setting ");
        PrintName(general);
        printf(" as a starting general\n");
        myRom->WriteByte(0x35559, general);
        startingGenerals.push_back(general);
    }

    myGenerals.SetStartingGenerals(startingGenerals);
}

void Randomizer::FixSlot7Glitch()
{
    // Based on meteorstrike's old guide, patching to terminate inner tactic
    //  loop when Y >= 7 rather than Y == 7
    myRom->WriteByte(0x3B616, 0x90);
}

void Randomizer::ResurrectAllAfterDeath()
{
    printf("Resurrecting all after death...\n");
    myRom->WriteByte(0x36145, 0x01); // Resurrect all generals with an ID greater than this...
    myRom->WriteByte(0x36149, 0xD9); // and an ID less than this
}

void Randomizer::GivePartyMaxFoodFromNPC()
{
    printf("Feeding the armies well...\n");
    // 0xF423F is 999999 in decimal.  These bytes determine the amount of food the NPC gives you.
    // Note that the text still says 1000.  I'll need to change that elsewhere.
    myRom->WriteByte(0x361D7, 0x0F);
    myRom->WriteByte(0x361DB, 0x42);
    myRom->WriteByte(0x361DF, 0x3F);
}

void Randomizer::ModifyEncounterRate()
{
    printf("Keeping the enemies away(ish)...\n");
    // Don't encounter enemies on anything that's not mountains (and whatever id 0x05 is)
    myRom->WriteByte(0x3F4D6, 0x04);
    // Make Mountain encounter rate really high (50%)
    myRom->WriteByte(0x3F51C, 0x01);
    myRom->WriteByte(0x3F522, 0x00);
}

/*void Randomizer::RandomizeBattles(vector<vector<BYTE>>& generalsForZone)
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
}*/

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
    std::uniform_int_distribution<int> levelSlot(1, 49);

    const int TACTIC_LEVEL_OFFSET = 0x3b880;
    printf("Old Tactic Levels:\n");
    for(int i = 1; i < 50; ++i)
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
    for(int i = 1; i < 50; ++i)
    {
        printf("  Level %d: Tactic id 0x%X\n", i + 1, myRom->ReadByte(TACTIC_LEVEL_OFFSET + i));
    }
}

void Randomizer::RandomizeGenerals()
{
    myGenerals.SetZonesForZone0(myGenerator);
    uniform_int_distribution<int> generalDistribution(0, myGenerals.size()-1);
    for (int i = 0; i < 10000; ++i)
    {
        myGenerals.SwapIDs(generalDistribution(myGenerator), generalDistribution(myGenerator));
    }
    for (int i = 0; i < 10000; ++i)
    {
        myGenerals.SwapZones(generalDistribution(myGenerator), generalDistribution(myGenerator));
    }
    myGenerals.ScaleForZone(myGenerator);
    myGenerals.ScaleSpecialGenerals(myGenerator);
    myGenerals.UpdateGenerals();
}

/*void Randomizer::RandomizeZones()
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
                if (myRom->ReadByte(soldiersOffset + 1) & 0x80)
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
}*/

void Randomizer::ImproveInitialBattlesAndFlags()
{
    // File start code (from scratch) initializes several battles to non-active, but it does it sequentially
    // and hardcoded, rather than through an index.  There appears to be unused bytes in the same bank that
    // runs this initialization code.  Thus, to save a few bytes and enable us to set more flags in the $6500
    // range, we modify this code to use a loop and initialize based upon the contents of $8700-877F
    std::vector<BYTE> loopingCode = {
        0xA2, 0x00,             // LDX #$00
        0xBD, 0x00, 0x87,       // LDA $8700,X
        0x9D, 0x00, 0x65,       // STA $6500,X
        0xE8,                   // INX
        0xE0, 0x80,             // CPX #$80
        0xD0, 0xF5,             // BNE ($9416) - (The LDA $8700,X instruction)
        // This is designed to get us back on track. Write 1 to 0x6010, since we've overwritten that portion, then branch to existing code
        0xA9, 0x01,             // LDA #$01
        0x8D, 0x10, 0x60,       // STA $6010
        0xD0, 0x0C,             // BEQ ($9434) - (The STA $6073 instruction)
        // Zero out now-unused bytes
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    for (int i = 0; i < loopingCode.size(); ++i)
    {
        myRom->WriteByte(0x35424 + i, loopingCode[i]);
    }

    // Now update the data at $8700 to match the overwritten code
    myRom->WriteByte(0x34715, 0x01);    // Qing Zhou (Front) for Second Yellow Scarves Battle
    myRom->WriteByte(0x34738, 0x01);    // Ling Ling Fight for Wu
    myRom->WriteByte(0x34739, 0x01);    // Wu Ling Fight for Wu
    myRom->WriteByte(0x34753, 0x01);    // Luo Yang Gate 1 (Sima Yi)
    myRom->WriteByte(0x34754, 0x01);    // Luo Yang Gate 2 (Sima Yi)
    myRom->WriteByte(0x34755, 0x01);    // Luo Yang Gate 3 (Sima Yi)
    myRom->WriteByte(0x34756, 0x01);    // Luo Yang Sima kids
    myRom->WriteByte(0x34757, 0x01);    // Luo Yang Sima Yi 3
    myRom->WriteByte(0x3475E, 0x01);    // Ru Nan Sima Yi
}

void Randomizer::ImproveMap()
{
    // Add Bridge under ZGL's house
    myRom->WriteByte(0x160C3, 0x2A);
    myRom->WriteByte(0x160F2, 0xF1);
    // Add Bridge from Chang Sha to Po Yang
    myRom->WriteByte(0x1802C, 0xC8);
    myRom->WriteByte(0x18031, 0x22);
    myRom->WriteByte(0x18032, 0x22);
    myRom->WriteByte(0x18033, 0x22);
    myRom->WriteByte(0x18034, 0x28);
    myRom->WriteByte(0x18035, 0x4A);
    myRom->WriteByte(0x18036, 0xDC);
    myRom->WriteByte(0x18037, 0xDC);
    myRom->WriteByte(0x18053, 0xF7);
    myRom->WriteByte(0x18054, 0x07);
    myRom->WriteByte(0x18618, 0x27);
    // Add Bridge from Xu Zhou to Ji Zhou
    myRom->WriteByte(0x18838, 0xFE);
    myRom->WriteByte(0x18839, 0x4A);
    myRom->WriteByte(0x1883A, 0xDC);
    myRom->WriteByte(0x1883B, 0xDC);
    myRom->WriteByte(0x1883C, 0xDC);
    myRom->WriteByte(0x1883D, 0xDC);
    myRom->WriteByte(0x1883E, 0xCF);
    myRom->WriteByte(0x1883F, 0x60);
    myRom->WriteByte(0x18878, 0x7C);
    myRom->WriteByte(0x188BF, 0xA7);
    myRom->WriteByte(0x188C7, 0x41);
    myRom->WriteByte(0x189EE, 0xCD);
    myRom->WriteByte(0x189EF, 0xFE);
    myRom->WriteByte(0x189DE, 0xCD);
    // Add Hills in Ji Zhou
    myRom->WriteByte(0x1889A, 0x87);
    // Add Hills in Zone 2
    myRom->WriteByte(0x1842C, 0x87);
    // Enable all story-trigger bridges
    myRom->WriteByte(0x34781, 0x01);    // Automatically enable the bridge in the Qing Zhou cave
    myRom->WriteByte(0x34782, 0x01);    // Automatically enable the bridge from Luo Yang to Chin
    myRom->WriteByte(0x34784, 0x01);    // Automatically enable the bridge from Nan Yang to Ji Zhou
    myRom->WriteByte(0x34788, 0x01);    // Automatically enable the bridge from Gui Yang to Wu
}

}
