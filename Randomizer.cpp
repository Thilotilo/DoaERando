#include "Randomizer.h"
#include <stdio.h>
#include "Generals.h"
#include "General.h"

using namespace std;

namespace DoaERando {

Randomizer::Randomizer(ROM* rom, int seed)
    : myRom(rom)
    , myGenerator(seed)
    , myBattleRandomizer()
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

    BYTE general1;
    do {
        general1 = distribution(myGenerator);
    } while (general1 == 0xB3 || general1 == 0xA6 || general1 == 0xA7 || general1 == 0xA8);
    printf("Setting ");
    PrintName(general1);
    printf(" as a starting general\n");
    myRom->WriteByte(0x35558, general1);
    startingGenerals.push_back(general1);

    BYTE general2;
    do {
        general2 = distribution(myGenerator);
    } while (general2 == 0xB3 || general2 == 0xA6 || general2 == 0xA7 || general2 == 0xA8 || general2 == general1);
    printf("Setting ");
    PrintName(general2);
    printf(" as a starting general\n");
    myRom->WriteByte(0x35559, general2);
    startingGenerals.push_back(general2);

    myGenerals.SetStartingGenerals(startingGenerals);

    // Set Starting generals to 0xCO
    // This uses the instructions from the ImproveInitialBattlesAndFlags method and places the
    // general IDs for the starting generals in the appropriate bytes.
    myRom->WriteByte(0x35434, general1);
    myRom->WriteByte(0x35437, general2);
}

static bool IsSingleExitCave(int offset)
{
    const int XU_ZHE_CAVE_OFFSET = 0x07;
    const int GOLD_KEY_CAVE_OFFSET = 0x46;
    const int NAN_YANG_CAVE_OFFSET = 0x63;
    const int SALTPETER_CAVE_OFFSET = 0x99;

    return (offset == XU_ZHE_CAVE_OFFSET ||
            offset == GOLD_KEY_CAVE_OFFSET ||
            offset == NAN_YANG_CAVE_OFFSET ||
            offset == SALTPETER_CAVE_OFFSET);
}

static bool AreExitsFromSameCave(int offset1, int offset2)
{
    const int MT_GANG_TAI_ENTRANCE_OFFSET = 0x0B;
    const int MT_GANG_TAI_EXIT_OFFSET = 0x1F;
    const int WU_CAVE_EXIT_OFFSET = 0x26;
    const int WU_CAVE_ENTRANCE_OFFSET = 0x39;
    const int CHANG_AN_ENTRANCE_OFFSET = 0x58;
    const int CHANG_AN_EXIT_OFFSET = 0x61;
    const int WEI_CAVE_2_ENTRANCE_OFFSET = 0x6C;
    const int WEI_CAVE_2_EXIT_OFFSET = 0x7B;
    const int WEI_CAVE_1_EXIT_OFFSET = 0x7E;
    const int WEI_CAVE_1_ENTRANCE_OFFSET = 0x93;

    return ((offset1 == MT_GANG_TAI_ENTRANCE_OFFSET && offset2 == MT_GANG_TAI_EXIT_OFFSET) ||
            (offset1 == MT_GANG_TAI_EXIT_OFFSET && offset2 == MT_GANG_TAI_ENTRANCE_OFFSET) ||
            (offset1 == WU_CAVE_ENTRANCE_OFFSET && offset2 == WU_CAVE_EXIT_OFFSET) ||
            (offset1 == WU_CAVE_EXIT_OFFSET && offset2 == WU_CAVE_ENTRANCE_OFFSET) ||
            (offset1 == CHANG_AN_ENTRANCE_OFFSET && offset2 == CHANG_AN_EXIT_OFFSET) ||
            (offset1 == CHANG_AN_EXIT_OFFSET && offset2 == CHANG_AN_ENTRANCE_OFFSET) ||
            (offset1 == WEI_CAVE_2_ENTRANCE_OFFSET && offset2 == WEI_CAVE_2_EXIT_OFFSET) ||
            (offset1 == WEI_CAVE_2_EXIT_OFFSET && offset2 == WEI_CAVE_2_ENTRANCE_OFFSET) ||
            (offset1 == WEI_CAVE_1_ENTRANCE_OFFSET && offset2 == WEI_CAVE_1_EXIT_OFFSET) ||
            (offset1 == WEI_CAVE_1_EXIT_OFFSET && offset2 == WEI_CAVE_1_ENTRANCE_OFFSET));
}

// TODO: Split this out into its own file, as we'll have more location shuffling logic in the future
void Randomizer::CaveShuffle()
{
    // These are world map offsets
    const int XU_ZHE_CAVE_OFFSET = 0x07;
    const int MT_GANG_TAI_ENTRANCE_OFFSET = 0x0B;
    const int MT_GANG_TAI_EXIT_OFFSET = 0x1F;
    const int WU_CAVE_EXIT_OFFSET = 0x26;
    const int WU_CAVE_ENTRANCE_OFFSET = 0x39;
    const int GOLD_KEY_CAVE_OFFSET = 0x46;
    const int CHANG_AN_ENTRANCE_OFFSET = 0x58;
    const int CHANG_AN_EXIT_OFFSET = 0x61;
    const int NAN_YANG_CAVE_OFFSET = 0x63;
    const int WEI_CAVE_2_ENTRANCE_OFFSET = 0x6C;
    const int WEI_CAVE_2_EXIT_OFFSET = 0x7B;
    const int WEI_CAVE_1_EXIT_OFFSET = 0x7E;
    const int WEI_CAVE_1_ENTRANCE_OFFSET = 0x93;
    const int SALTPETER_CAVE_OFFSET = 0x99;

    // Magic numbers are bad, but 8 million consts to start a method are obnoxious.
    vector<vector<BYTE>> CaveWarpOffsets = {
        {XU_ZHE_CAVE_OFFSET,          0x08, 0x09},
        {MT_GANG_TAI_ENTRANCE_OFFSET, 0x0A},
        {MT_GANG_TAI_EXIT_OFFSET,     0x1E},
        {WU_CAVE_EXIT_OFFSET,         0x27},
        {WU_CAVE_ENTRANCE_OFFSET,     0x38},
        {GOLD_KEY_CAVE_OFFSET,        0x47, 0x48},
        {CHANG_AN_ENTRANCE_OFFSET,    0x57, 0x59},
        {CHANG_AN_EXIT_OFFSET,        0x60, 0x62},
        {NAN_YANG_CAVE_OFFSET,        0x64, 0x65},
        {WEI_CAVE_2_ENTRANCE_OFFSET,  0x6D, 0x7D},
        {WEI_CAVE_2_EXIT_OFFSET,      0x7A, 0x7C},
        {WEI_CAVE_1_EXIT_OFFSET,      0x7F, 0x80},
        {WEI_CAVE_1_ENTRANCE_OFFSET,  0x94, 0x95},
        {SALTPETER_CAVE_OFFSET,       0x9A, 0x9B}
    };

    vector<vector<BYTE>> ShuffledOffsets = CaveWarpOffsets;

    for (int i = 0; i < 10000; ++i)
    {
        uniform_int_distribution<int> caveDistribution(0, ShuffledOffsets.size() - 1);
        int location1 = caveDistribution(myGenerator);
        int location2 = caveDistribution(myGenerator);
        swap(ShuffledOffsets[location1], ShuffledOffsets[location2]);
    }

    // Now do sanity checks
    // For now, let's prevent the following:
    // 1) Wu Cave entrance should not be a single-entrance cave to prevent softlocks from canal
    // 2) Mt Gang Tai Exit cannot be a single-entrance cave, and it's other side cannot come to
    //    one of the Wei caves if the other is a Single Exit cave
    // 3) Wei1 Exit & Wei2 Entrance can't be opposite ends of the same cave
    // 4) Wei1 Exit & Wei2 Entrance can't both be single-entrance caves.
    while (IsSingleExitCave(ShuffledOffsets[4][0]) ||
           IsSingleExitCave(ShuffledOffsets[2][0]) ||
           (AreExitsFromSameCave(ShuffledOffsets[2][0], ShuffledOffsets[9][0]) &&
            IsSingleExitCave(ShuffledOffsets[11][0])) ||
           (AreExitsFromSameCave(ShuffledOffsets[2][0], ShuffledOffsets[11][0]) &&
            IsSingleExitCave(ShuffledOffsets[9][0])) ||
           AreExitsFromSameCave(ShuffledOffsets[9][0], ShuffledOffsets[11][0]) ||
           (IsSingleExitCave(ShuffledOffsets[9][0]) && IsSingleExitCave(ShuffledOffsets[11][0])))
    {
        uniform_int_distribution<int> caveDistribution(0, ShuffledOffsets.size() - 1);
        int location1 = caveDistribution(myGenerator);
        int location2 = caveDistribution(myGenerator);
        swap(ShuffledOffsets[location1], ShuffledOffsets[location2]);
    }

    // Collect all the bytes we need to update
    std::vector<std::vector<BYTE>> bytesToWrite;
    for (int i = 0; i < ShuffledOffsets.size(); ++i)
    {
        printf("Putting cave id 0x%02X in the vanilla 0x%02X location\n",
               ShuffledOffsets[i][0], CaveWarpOffsets[i][0]);
        int lsb = myRom->ReadByte(0x1C210 + ShuffledOffsets[i][0]);
        int msb = myRom->ReadByte(0x1C2C0 + ShuffledOffsets[i][0]);
        int shuffledAddressInBank = (msb << 8) | lsb;

        lsb = myRom->ReadByte(0x1C210 + CaveWarpOffsets[i][1]);
        msb = myRom->ReadByte(0x1C2C0 + CaveWarpOffsets[i][1]);
        int caveExitAddressInBank = (msb << 8) | lsb;

        // This puts the destination of the shuffled world map address into the vanilla location
        std::vector<BYTE> updateToWrite;
        updateToWrite.push_back(CaveWarpOffsets[i][0]);
        for (int j = 4; j < 9; ++j)
        {
            updateToWrite.push_back(myRom->ReadByteAtBank(0x7, shuffledAddressInBank + j));
        }

        bytesToWrite.push_back(updateToWrite);

        // This puts the destination of the vanilla cave exits to the shuffled world map location
        for (int j = 1; j < ShuffledOffsets[i].size(); ++j)
        {
            std::vector<BYTE> updatedCaveEntrance;
            updatedCaveEntrance.push_back(ShuffledOffsets[i][j]);
            for (int k = 4; k < 9; ++k)
            {
                updatedCaveEntrance.push_back(myRom->ReadByteAtBank(0x7, caveExitAddressInBank + k));
            }
            bytesToWrite.push_back(updatedCaveEntrance);
        }
    }

    // Now write everything
    for (auto& entry : bytesToWrite)
    {
        int lsb = myRom->ReadByte(0x1C210 + entry[0]);
        int msb = myRom->ReadByte(0x1C2C0 + entry[0]);
        int addressInBank = (msb << 8) | lsb;

        for (int i = 1; i < entry.size(); ++i)
        {
            myRom->WriteByteAtBank(0x7, addressInBank + i + 3, entry[i]);
        }
    }
}

// TODO: Split this out into its own file, as we'll have more constraints as shuffling goes up
void Randomizer::ItemShuffle()
{
    std::vector<BYTE> chestContentsToRandomize;

    // Grab starting items
    chestContentsToRandomize.push_back(myRom->ReadByte(0x3556C));
    chestContentsToRandomize.push_back(myRom->ReadByte(0x3556D));
    chestContentsToRandomize.push_back(myRom->ReadByte(0x35574));
    chestContentsToRandomize.push_back(myRom->ReadByte(0x35575));
    chestContentsToRandomize.push_back(myRom->ReadByte(0x3557C));
    chestContentsToRandomize.push_back(myRom->ReadByte(0x3557D));

    // Grab hidden items
    const int HIDDEN_ITEM_OFFSET = 0x30B10;
    for (int i = HIDDEN_ITEM_OFFSET; i < HIDDEN_ITEM_OFFSET + 20; ++i)
    {
        BYTE item = myRom->ReadByte(i);
        if (item != 0xFF) // I found a keyhole, i = 2
        {
            chestContentsToRandomize.push_back(item);
        }
    }

    // Grab non-hidden items
    const int ITEMS_OFFSET = 0x30914; // (Every 5th byte is the item ID) - data starts at 30910
    const int ITEMS_OFFSET_END = 0x309E7;
    for (int i = ITEMS_OFFSET; i < ITEMS_OFFSET_END; i += 5)
    {
        chestContentsToRandomize.push_back(myRom->ReadByte(i));
    }

    uniform_int_distribution<int> itemDistribution(0, chestContentsToRandomize.size() - 1);
    for (int i = 0; i < 100000; ++i)
    {
        int index1 = itemDistribution(myGenerator);
        int index2 = itemDistribution(myGenerator);

        BYTE item1 = chestContentsToRandomize[index1];
        BYTE item2 = chestContentsToRandomize[index2];

        // Make sure that we're not swapping coins into starting inventory
        if (!((index1 < 6 || index2 < 6) && (item1 < 0x10 || item2 < 0x10)))
        {
            chestContentsToRandomize[index1] = item2;
            chestContentsToRandomize[index2] = item1;
        }
    }

    int index = 0;

    for (int i = 0; i < chestContentsToRandomize.size(); ++i)
    {
        if (i == 0)
        {
            printf ("Starting items:\n");
        }
        if (i == 6)
        {
            printf ("Hidden items:\n");
        }
        if (i == 25)
        {
            printf ("Normal chests:\n");
        }
        printf ("\tChest %d: 0x%02X\n", i, chestContentsToRandomize[i]);
    }

    // Write new starting items
    myRom->WriteByte(0x3556C, chestContentsToRandomize[index++]);
    myRom->WriteByte(0x3556D, chestContentsToRandomize[index++]);
    myRom->WriteByte(0x35574, chestContentsToRandomize[index++]);
    myRom->WriteByte(0x35575, chestContentsToRandomize[index++]);
    myRom->WriteByte(0x3557C, chestContentsToRandomize[index++]);
    myRom->WriteByte(0x3557D, chestContentsToRandomize[index++]);

    // Write new hidden items
    for (int i = HIDDEN_ITEM_OFFSET; i < HIDDEN_ITEM_OFFSET + 20; ++i)
    {
        BYTE item = myRom->ReadByte(i);
        if (item != 0xFF) // I found a keyhole, i = 2
        {
            myRom->WriteByte(i, chestContentsToRandomize[index++]);
        }
    }

    for (int i = ITEMS_OFFSET; i < ITEMS_OFFSET_END; i += 5)
    {
        myRom->WriteByte(i, chestContentsToRandomize[index++]);
    }
}

void Randomizer::FixSlot7Glitch()
{
    // Based on meteorstrike's old guide, patching to terminate inner tactic
    //  loop when Y >= 7 rather than Y == 7
    myRom->WriteByte(0x3B616, 0x90);
}

// The belief here is that the random crashes we get in the vanilla game
//  are based upon an NMI running at an inopportune time.  Specifically,
//  the subroutine we are modifying below would pull the top two stack
//  bytes and then read them as a stack offset rather than just reading
//  the pulled values.  This led to a potential interrupt between the
//  pulls and the reads, at which point the reads would point to garbage
//  data, and the routine would return to who knows where and execute.
// We fix this by just reading the pulled data rather than the stack offset.
// This has the added bonus of allowing us to ignore storing off X, the TSX
//  and restoring X, since X is only used for the stack read.  Ultimately,
//  it frees up 11 bytes starting at 0x3C51F that we can use for other
//  purposes in the future.
void Randomizer::FixNMIGlitch()
{
    // This should hopefully stop all of the crashes
    std::vector<BYTE> loopingCode = {
        0x68,               // PLA
        0x85, 0xF3,         // STA $F3
        0x68,               // PLA
        0x85, 0xF4,         // STA $F4
        // Now we're just shifting this section up to follow immediately
        0xA0, 0x02,         // LDY #$02
        0xB1, 0xF3,         // LDA ($F3),Y
        0x48,               // PHA
        0x88,               // DEY
        0xB1, 0xF3,         // LDA ($F3),Y
        0x48,               // PHA
        0xA5, 0x45,         // LDA $45
        0x4C, 0x80, 0xC4,   // JMP $C480
        // Zero out remaining bytes from this logic
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    for (int i = 0; i < loopingCode.size(); ++i)
    {
        myRom->WriteByte(0x3C50B + i, loopingCode[i]);
    }
}

void Randomizer::ResurrectAllAfterDeath()
{
    printf("Resurrecting all after death...\n");
    myRom->WriteByte(0x36145, 0x01); // Resurrect all generals with an ID greater than this...
    myRom->WriteByte(0x36149, 0xD9); // and an ID less than this
}

void Randomizer::OnlyLaunchShuLuBuOnce()
{
    // For whatever reason, the game checks Lu Bu's recruitment flags rather than
    // the "battle active" flag to determine if we should launch the Shu Lu Bu battle.
    // This causes problems in the rando when that Lu Bu's recruitment flags can be
    // turned active even after defeating the Shu Battle.  We fix this by skipping the
    // battle if the battle active flag is false rather than if Lu Bu's status is 00
    // Thus, we're changing these instructions:
    //   LDA $63B7
    //   BPL $B3DD
    // to
    //   LDA $654F
    //   BNE $B3DD
    myRom->WriteByte(0x3B3D9, 0x4F); // $654F
    myRom->WriteByte(0x3B3DA, 0x65);
    myRom->WriteByte(0x3B3DB, 0xD0); // BNE
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

void Randomizer::DisableNonGeneralBattles()
{
    // With these changes, the odds of a general battle is 100%, but if there are no
    // generals, we will still get a random battle of forces.
    myRom->WriteByte(0x3B310, 0x00); // Set zones 0-5 to 100% generals
    myRom->WriteByte(0x3B31B, 0x00); // Set zones 6-8 to 100% generals
}

void Randomizer::IncreaseTacticGains()
{
    // Since we aren't gaining TP on every level now, let's up our TP gain
    // Here, we're going to randomize each 1/16 chance value to be between
    // 5-10, with the last slot being 15-20.
    const int TACTIC_GAINS_OFFSET = 0x3B666;
    uniform_int_distribution<BYTE> standardDistribution(5,10);
    uniform_int_distribution<BYTE> majorGainDistribution(15,20);
    for (int i = 0; i < 15; ++i)
    {
        BYTE tacticGain = standardDistribution(myGenerator);
        myRom->WriteByte(TACTIC_GAINS_OFFSET + i, tacticGain);
        printf("Increasing tactic gain 0x%X to %d\n", i, tacticGain);
    }
    BYTE tacticGain = majorGainDistribution(myGenerator);
    myRom->WriteByte(TACTIC_GAINS_OFFSET + 15, tacticGain);
    printf("Increasing tactic gain 0x%X to %d\n", 15, tacticGain);
}

void Randomizer::DoubleXpGain()
{
    const int XP_MULTIPLIER_OFFSET = 0x39a28;
    for (int i = 0; i < 8; ++i)
    {
        BYTE currentXpBoost = myRom->ReadByte(XP_MULTIPLIER_OFFSET + i);
        BYTE increasedXpBoost = currentXpBoost * 2;
        myRom->WriteByte(XP_MULTIPLIER_OFFSET + i, increasedXpBoost);
    }
}

void Randomizer::MakeAllGeneralsRecruitableAndEncounterable()
{
    myGenerals.SetAllGeneralsEncounterable();
    myGenerals.SetAllGeneralsRecruitable();

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
    myGenerals.AdjustZone1Generals();
    for (int i = 0; i < 9; ++i)
    {
        printf("Zone %d Generals: \n\t", i);
        auto zoneGenerals = myGenerals.GetGeneralIdsFromZone(i);
        for (auto& general : zoneGenerals)
        {
            PrintName(general);
            printf("\n\t");
        }
        printf("\n");
    }

    myGenerals.SetZonesForZone0(myGenerator);
    uniform_int_distribution<int> generalDistribution(0, myGenerals.size()-1);
    for (int i = 0; i < 10000; ++i)
    {
        myGenerals.SwapZones(generalDistribution(myGenerator), generalDistribution(myGenerator));
    }
    myGenerals.ScaleForZone(myGenerator);
    myGenerals.RandomizeCaoCaoAndSunCe(myGenerator);
    myGenerals.ScaleSpecialGenerals(myGenerator);
    myGenerals.UpdateGenerals();

    printf("After Randomization:\n");
    for (int i = 0; i < 9; ++i)
    {
        printf("Zone %d Generals: \n\t", i);
        auto zoneGenerals = myGenerals.GetGeneralIdsFromZone(i);
        for (auto& general : zoneGenerals)
        {
            PrintName(general);
            printf("\n\t");
        }
        printf("\n");
    }
}

void Randomizer::RandomizeBattles()
{
    myBattleRandomizer.RandomizeZone1(myGenerals, myGenerator);
    myBattleRandomizer.RandomizeZone2(myGenerals, myGenerator);
    myBattleRandomizer.RandomizeZone3(myGenerals, myGenerator);
    myBattleRandomizer.RandomizeZone4(myGenerals, myGenerator);
    myBattleRandomizer.RandomizeZone5(myGenerals, myGenerator);
    myBattleRandomizer.RandomizeZone6(myGenerals, myGenerator);
    myBattleRandomizer.RandomizeZone7(myGenerals, myGenerator);
    myBattleRandomizer.RandomizeZone8(myGenerals, myGenerator);
    myBattleRandomizer.UpdateBattles(*myRom);
}

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
        // Now let's write #C0 to 3 different addresses, Guan Suo and 2 others that will be assigned when
        // we select starting generals (for now, we'll just write $6500 for the address).  This is already
        // written to Guan Suo a few bytes past here.  We're just moving that write up with the others to save
        // a couple of instruction bytes.
        0xA9, 0xC0,             // LDA #$C0
        0x8D, 0x00, 0x63,       // STA $6300 (LSB to be overwritten with starting general ID)
        0x8D, 0x00, 0x63,       // STA $6300 (LSB to be overwritten with starting general ID)
        0x8D, 0xA4, 0x63,       // STA $63A4 (Guan Suo)
        // Here, we need to finish with the initialization code that we overwrote/moved.
        // First, we need to write 1 to 0x6010, since we overwrote that portion
        0xA9, 0x01,             // LDA #$01
        0x8D, 0x10, 0x60,       // STA $6010 (Chapter)
        // Now we need to shift a couple of other writes around to handle the moved Guan Suo flag set.
        0x8D, 0x73, 0x60,       // STA $6073 (Level)
        0xA9, 0x0A,             // LDA #$0A
        0x8D, 0xF8, 0x61,       // STA $61F8 (Max TP)
        // Now we need to branch to the remainder of the init code
        0xD0, 0x06,             // BNE ($9441) - (LDA #$00)
        // Zero out now-unused bytes
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
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
    // Fix Luo Yang lookup.
    // Right now, the version of Luo Yang we get is based upon the status of the Chin bridge.  Since
    // we've permanently enabled it, that means we only get the restored Luo Yang castle, which means
    // that we can't fight the original Luo Yang, nor can we recruit the general in it, nor can we
    // use the Gold Key.  To fix this, we just want to change it to use the new Luo Yang gates to
    // determine which version we want to load.
    myRom->WriteByte(0x3BCF9, 0x77);
}

}
