#include "Randomizer.h"
#include <stdio.h>
#include <algorithm>
#include "Generals.h"
#include "General.h"
#include "GeneralIds.h"

using namespace std;

namespace DoaERando {

Randomizer::Randomizer(ROM* rom, int seed)
    : myRom(rom)
    , myRNG(seed)
    , myBattleRandomizer()
    , myGenerals(*rom)
    , myNPCManipulator(*rom)
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

    std::vector<BYTE> startingGenerals;
    startingGenerals.push_back(0xA8);

    BYTE general1;
    do {
        general1 = myRNG.GetRandomByte(0x05, 0xD7);
    } while (general1 == 0xB3 || general1 == 0xA6 || general1 == 0xA7 || general1 == 0xA8);
    printf("Setting ");
    PrintName(general1);
    printf(" as a starting general\n");
    myRom->WriteByte(0x35558, general1);
    startingGenerals.push_back(general1);

    BYTE general2;
    do {
        general2 = myRNG.GetRandomByte(0x05, 0xD7);
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
        int location1 = myRNG.GetRandomInt(0, ShuffledOffsets.size() - 1);
        int location2 = myRNG.GetRandomInt(0, ShuffledOffsets.size() - 1);
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
        int location1 = myRNG.GetRandomInt(0, ShuffledOffsets.size() - 1);
        int location2 = myRNG.GetRandomInt(0, ShuffledOffsets.size() - 1);
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

    // Add Zhao Yun's sister's item
    chestContentsToRandomize.push_back(myRom->ReadByte(0x368DC));

    // Add Intro Letr item (We'll put gemsword behind this item)
    chestContentsToRandomize.push_back(myRom->ReadByte(0x366DD));

    // Add Silver Key item
    chestContentsToRandomize.push_back(myRom->ReadByte(0x3680F));

    // Add Gunpowder Item
    chestContentsToRandomize.push_back(myRom->ReadByte(0x368A2));

    // Add Qing Long Item
    chestContentsToRandomize.push_back(myRom->ReadByte(0x37333));

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

    for (int i = 0; i < 100000; ++i)
    {
        int index1 = myRNG.GetRandomIndexFromByteVector(chestContentsToRandomize);
        int index2 = myRNG.GetRandomIndexFromByteVector(chestContentsToRandomize);

        BYTE item1 = chestContentsToRandomize[index1];
        BYTE item2 = chestContentsToRandomize[index2];

        // 3 Conditions we have to verify
        // 1) Make sure that we're not swapping coins into starting inventory or talk-and-speak inventory
        // 2) Make sure that the Silver Key is not behind Zhao Yun's sister
        // 3) Make sure that the Zhou Letr isn't in Guang Zong.
        if (!((index1 < 11 || index2 < 11) && (item1 < 0x10 || item2 < 0x10)) // 1
            || (index1 == 14 && item2 == 0x22) || (index2 == 14 && item1 == 0x22) // 2
            || (index1 == 8 && item2 == 0x18) || (index2 == 8 && item2 == 0x18)) // 3
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
            printf ("Talk-and-get items:\n");
        }
        if (i == 11)
        {
            printf ("Hidden items:\n");
        }
        if (i == 30)
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

    // Write new Zhou Letr item
    myRom->WriteByte(0x368DC, chestContentsToRandomize[index++]);

    // Write new Intro Letr item
    myRom->WriteByte(0x366DD, chestContentsToRandomize[index++]);

    // Write new Silver Key item
    myRom->WriteByte(0x3680F, chestContentsToRandomize[index++]);

    // Write new Gunpowder Item
    myRom->WriteByte(0x368A2, chestContentsToRandomize[index++]);

    // Write new Qing Long Item
    myRom->WriteByte(0x37333, chestContentsToRandomize[index++]);

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

void Randomizer::DisableChapter1LockForSiShui()
{
    printf("Opening Si Shui fortress to Liu Bei...\n");
    // Make only chapters below 1 (i.e., none) block entrance to Si Shui
    myRom->WriteByte(0x3642B, 0x01);
    // Don't disable Si Shui battle unless we enter in chapter 0 (i.e., never)
    myRom->WriteByte(0x3643B, 0x00);
    // Don't decrement the Y counter unless we are in a chapter below 1 (i.e., never)
    myRom->WriteByte(0x3BE8B, 0x01);
}

void Randomizer::RemoveZhugeLiangFetchQuest()
{
    // Put Zhuge Jin in the house from this chapter and on:
    myRom->WriteByte(0x36A0C, 0x01);
    // Instead of shifting to get the bit indicating if ZGL should show up initially, just set it.
    myRom->WriteByte(0x36A26, 0x38); // SEC - the next instruction in BCS
}

void Randomizer::MakeZhangBaoAndGuanXingAvailable()
{
    // In the special Chang Sha castle execution, it clears out
    // Guan Yu & Guan Xing's sprites if $6552 b5 hasn't been set.
    // There's no real value in having Guan Yu, so to prevent
    // modifying the code as much as possible, we simply clear out
    // Guan Yu twice
    // Replace the Guan Xing address to clear with Guan Yu
    myRom->WriteByte(0x36DAE, 0x03);
    // Same concept with Zhang Fei, but even more important because
    // We MUST keep Zhang Fei out, lest he conflict with Liu Bei
    // Replace the Zhang Bao address to clear with Zhang Fei
    myRom->WriteByte(0x36BBB, 0x0B);
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
    for (int i = 0; i < 15; ++i)
    {
        BYTE tacticGain = myRNG.GetRandomByte(5, 10);
        myRom->WriteByte(TACTIC_GAINS_OFFSET + i, tacticGain);
        printf("Increasing tactic gain 0x%X to %d\n", i, tacticGain);
    }
    BYTE tacticGain = myRNG.GetRandomByte(15, 20);
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
    const int TACTIC_LEVEL_OFFSET = 0x3b880;
    printf("Old Tactic Levels:\n");
    for(int i = 1; i < 50; ++i)
    {
        printf("  Level %d: Tactic id 0x%X\n", i + 1, myRom->ReadByte(TACTIC_LEVEL_OFFSET + i));
    }
    for (int i = 0; i < 10000; ++i)
    {
        int levelSlot1 = myRNG.GetRandomByte(1, 49);
        int levelSlot2 = myRNG.GetRandomByte(1, 49);

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

    myGenerals.SetZonesForZone0(myRNG);
    for (int i = 0; i < 10000; ++i)
    {
        myGenerals.SwapZones(
            myRNG.GetRandomInt(0, myGenerals.size() - 1),
            myRNG.GetRandomInt(0, myGenerals.size() - 1));
    }
    myGenerals.ScaleForZone(myRNG);
    myGenerals.RandomizeCaoCaoAndSunCe(myRNG);
    myGenerals.ScaleSpecialGenerals(myRNG);
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
    myBattleRandomizer.RandomizeZone1(myGenerals, myRNG);
    myBattleRandomizer.RandomizeZone2(myGenerals, myRNG);
    myBattleRandomizer.RandomizeZone3(myGenerals, myRNG);
    myBattleRandomizer.RandomizeZone4(myGenerals, myRNG);
    myBattleRandomizer.RandomizeZone5(myGenerals, myRNG);
    myBattleRandomizer.RandomizeZone6(myGenerals, myRNG);
    myBattleRandomizer.RandomizeZone7(myGenerals, myRNG);
    myBattleRandomizer.RandomizeZone8(myGenerals, myRNG);
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

void Randomizer::MoveGuiYangBattle()
{
    // First, set the flags to clear the throne room battle
    // We only want to set two bits, so let's read and then set those 2 bits only
    auto flags = myRom->ReadByte(0x34761);
    // bit 1 = Gui Yang battle started
    // bit 2 = Gui Yang battle won
    flags |= 0x06;
    myRom->WriteByte(0x34761, flags);

    // Next, we're going to move the Gui Yang battle to the castle entrance
    myRom->WriteByte(0x30C2B, 0x09); // Vertical offset in block
    myRom->WriteByte(0x30C83, 0x12); // Vertical block
    myRom->WriteByte(0x30CDB, 0x03); // Horizontal offset in block
    myRom->WriteByte(0x30D33, 0x0E); // Horizontal block
}

// New General Shuffle Logic:

static void PullIdFromVector(BYTE id, vector<BYTE>& ids)
{
    ids.erase(std::remove(ids.begin(), ids.end(), id), ids.end());
}

static void PullLuBuIds(vector<BYTE>& ids)
{
    // Remove all 4 Lu Bu bytes.
    PullIdFromVector(LU_BU_GEMSWORD_ID, ids);
    PullIdFromVector(LU_BU_NAN_YANG_ID, ids);
    PullIdFromVector(LU_BU_JI_ZHOU_ID, ids);
    PullIdFromVector(LU_BU_SHU_ID, ids);
}

void Randomizer::SetGeneralForZone0AndRemove(BYTE generalId, std::vector<BYTE>& ids)
{
    myGenerals.SetAndScaleGeneralForZone(generalId, 0, myRNG);
    myGenerals.SetGeneralEncounterable(generalId, false);
    myGenerals.SetGeneralRecruitable(generalId, false);
    PullIdFromVector(generalId, ids);
}

void Randomizer::ReconfigureQingZhouCave()
{
    // This closes the Qing Zhou Exit
    myRom->WriteByte(0x19BD1, 0x62);
    myRom->WriteByte(0x19BD9, 0x80);

    // This prevents the Qing Zhou cave from disappearing after defeating
    // the Yellow Scarves.
    // Easiest way for now is just to replace the "inactive" write
    // to the flag with the "active" write (00 -> 01)
    myRom->WriteByte(0x3639F, 0x01);

    // We want to force bridge creation again.  Since we care about this trigger,
    // overwrite any previous updates that make the bridge permanently available.
    myRom->WriteByte(0x34781, 0x00);

    // Update the cave NPC data to lose the yellow scarves and instead have a single
    // Talk-and-join general.  With this update and the new use of the cave, we
    // now no longer need special code to execute when we load this map, so we
    // save 2 additional bytes on top of the 14 bytes we save by decreasing the
    // character count by 2
    std::vector<BYTE> caveNPCData {
        // NPC in cave -> actual NPC to be set by the manipulator, so we just
        // set it to 0 for now.
        0x1B, 0x04, 0x83, 0x03, 0x19, 0x19, 0x00,
        0x27, 0x03, 0x88, 0x04, 0x69, 0x69, 0x02, // Brigand guarding bridge
        0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Cleared NPC Data
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Cleared NPC Data
        // Since there's no longer any code to execute, we need to clear
        // those extra 2 bytes too (plus the original FF)
        0x00, 0x00, 0x00
    };

    for (int i = 0; i < caveNPCData.size(); ++i)
    {
        myRom->WriteByte(0x31188 + i, caveNPCData[i]);
    }

    // Remove now-unused code for loading the Qing Zhou cave so we can reclaim
    // it for other nefarious purposes.
    const size_t NUM_EVENT_BYTES_TO_CLEAR = 22;
    for (size_t i = 0; i < NUM_EVENT_BYTES_TO_CLEAR; ++i)
    {
        myRom->WriteByte(0x363BF + i, 0x00);
    }
}

void Randomizer::ReconfigureLiuKuiTentGeneral()
{
    std::vector<BYTE> tentNPCData {
        // Configure NPC info ->actual NPC ID to be set by the manipulator,
        // So we just set it to 0 for now
        0x13, 0x04, 0x97, 0x1C, 0x19, 0x19, 0x00,
        // Since there's no longer any code to execute, we need to clear
        // those extra 2 bytes too (plus move the original FF)
        0xFF, 0x00, 0x00
    };

    for (int i = 0; i < tentNPCData.size(); ++i)
    {
        myRom->WriteByte(0x31420 + i, tentNPCData[i]);
    }

    // Remove now-unused code for loading the NPC so we can reclaim
    // it for other nefarious purposes.
    const size_t NUM_EVENT_BYTES_TO_CLEAR = 11;
    for (size_t i = 0; i < NUM_EVENT_BYTES_TO_CLEAR; ++i)
    {
        myRom->WriteByte(0x36EE0 + i, 0x00);
    }
}

void Randomizer::MakeZone0GeneralsUnfirable()
{
    std::vector<BYTE> newCode {
        // $60AF contains the general id
        0xAC, 0xAF, 0x60,   // LDY $60AF
        0xA9, 0x02,         // LDA #$02
        // E3C8 returns byte A of the general info of the general id in Y
        0x20, 0xC8, 0xE3,   // JSR $E3C8
        0xF0, 0x05,         // BEQ ($94E5) - the "can't fire" response
        0xD0, 0x09,         // BNE ($94EB) - the confirmation dialog
        // Now finish padding the rest of the overwritten code
        0x00, 0x00, 0x00
    };

    for (int i = 0; i < newCode.size(); ++i)
    {
        myRom->WriteByte(0x394E6 + i, newCode[i]);
    }

    // Remove the old unfirable generals list so we can reclaim
    // it for other nefarious purposes.
    const size_t NUM_EVENT_BYTES_TO_CLEAR = 17;
    for (size_t i = 0; i < NUM_EVENT_BYTES_TO_CLEAR; ++i)
    {
        myRom->WriteByte(0x39587 + i, 0x00);
    }
}

// This is here simply to remove code executed on
// entrance triggers, since we don't actually need
// many of these, and it frees up space to do so.
void Randomizer::RemoveUneededEntranceTriggerCode()
{
    // Tie Men Xia - we don't want to remove NPCs, so the
    // code is unneeded
    // Remove the "code executed" bit
    BYTE execByte = myRom->ReadByte(0x31A2D);
    execByte &= 0xDF; // clear bit 5
    myRom->WriteByte(0x31A2D, execByte);
    // Blank out both the code-to-execute pointer (moving the
    // FF terminator as well) and the actual code
    myRom->WriteByte(0x31A31, 0xFF);
    myRom->WriteByte(0x31A32, 0x00);
    myRom->WriteByte(0x31A33, 0x00);

    const size_t NUM_EVENT_BYTES_TO_CLEAR = 15;
    for (size_t i = 0; i < NUM_EVENT_BYTES_TO_CLEAR; ++i)
    {
        myRom->WriteByte(0x3536E, 0x00);
    }

}

void Randomizer::UpdateLiuBeiTurnInTrigger()
{
    // 2 things we're fixing at the moment.
    // 1) Increment chapter rather than overwrite it when we turn in Liu Bei
    // 2) Don't modify the Si Shui battle flag.

    // Increment chapter.  This actually leaves 2 meaningless bytes before it
    // (LDA #$02), but fixing that is a pretty heavy cleanup, so we'll just
    // let it do the uneeded work.
    myRom->WriteByte(0x362CE, 0xEE);

    // Si Shui fix.  This also wastes 3 bytes, but it's easier to just write 0
    // to the same location twice rather than shift all the code.
    myRom->WriteByte(0x36301, 0x9E);
    myRom->WriteByte(0x36302, 0x60);
}

void Randomizer::MakeIronOreUseful()
{
    // Blacksmith now returns a random top tier weapon (Qing Long, Qing Guang, Nu Long, Halberd)
    std::vector<BYTE> possibleGear = {0x4F, 0x50, 0x51, 0x52};
    BYTE gear = myRNG.GetRandomValueFromByteVector(possibleGear);

    printf("Giving the blacksmith weapon 0x%02X...\n", gear);

    std::vector<BYTE> newCode {
        0xA9, 0x1D,         // LDA #$1D
        0x85, 0xF2,         // STA $F2
        // This subroutine checks the party inventory for the item in $F2,
        // removes it if it finds it, and sets C to indicate if it was found.
        0x20, 0x39, 0x9E,   // JSR $3E39
        0x90, 0x1A,         // BCC ($B150)
        // If we found the item, set bit 3 of 6731 (indicates we have turned in
        // the ore), and set the bit indicating that the weapons were stolen in
        // vanilla
        0xAD, 0x31, 0x67,   // LDA $6731
        0x09, 0x08,         // ORA #$08
        0x8D, 0x31, 0x67,   // STA $6731
        // This subroutine gets B from bits 2-0 and X from bits 4-3.  It then sets
        // the bit B at $655X.
        // Here, 0x1A would be B = 2, X = 3, so it sets bit 5 of $6553
        0xA9, 0x1A,         // LDA #$1A
        0x20, 0xC6, 0x9C,   // JSR $9CC6
        // This is the dialog subroutine.  The dialogue is determined by the 2
        // bytes following the call, the first referring to the id, and the
        // second referring to the bank
        0x20, 0x0D, 0xD1,   // JSR $D10D
        0x14, 0x02,         // Text set below
        // This subroutine places the item in A into the inventory.  Since we
        // just lost the iron ore, we don't have to worry about space issues
        0xA9, gear,         // LDA #(gearId)
        0x20, 0x6F, 0xE9,   // JSR $E96F
        // Get us out of here
        0x4C, 0xA6, 0xC4,   // JMP $C4A6 (switches back to bank E, then returns)

        // This is the path if we don't have the ore in our inventory
        0xAD, 0x31, 0x67,   // LDA $6731
        0x29, 0x08,         // ORA #$08
        0xF0, 0x08,         // BEQ ($B15F)
        // We're here if we turned in the ore before
        0x20, 0x0D, 0xD1,   // JSR $D10D
        0x15, 0x02,         // Text set below
        0x4C, 0xA6, 0xC4,   // JMP $C4A6
        // We're here if we have never turned the ore in
        0x20, 0x0D, 0xD1,   // JSR $D10D
        0x13, 0x02,         // Text set below
        0x4C, 0xA6, 0xC4,   // JMP $C4A6

        // Zero out the remaining unused bytes (0x11 of them)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00
    };

    for (int i = 0; i < newCode.size(); ++i)
    {
        myRom->WriteByte(0x3713D + i, newCode[i]);
    }

    // Now let's update some text strings:
    std::vector<BYTE> blacksmithText {
        // Bank 2 Text 0x13 - Text if you haven't turned in the ore and don't have the ore
        0x11,0x2D,0x38,0x3D,0x36,0x0A,0x3C,0x34,0x0A,0x2E,0x3E,0x3C,0x34,0x0A,0x18,0x2D,0x3E,0x3D,0xFB, // Bring me some Iron (19 bytes)
        0x1E,0x2D,0x34,0x0A,0x30,0x3D,0x33,0x0A,0x35,0x3E,0x2D,0x2F,0x64,0x3D,0x34,0xFB, // Ore and fortune (16 bytes)
        0x66,0x38,0x3B,0x3B,0x0A,0x2E,0x3C,0x38,0x3B,0x34,0x0A,0x64,0x3F,0x3E,0x3D,0xFB, // Will smile upon (16 bytes)
        0x2F,0x37,0x34,0x34,0xEC, // thee. (5 bytes)
        // Bank 2 Text 0x14 - Text if you have iron ore.
        0x23,0x37,0x30,0x3D,0x3A,0x0A,0x68,0x3E,0x64,0x0A,0x35,0x3E,0x2D,0x0A,0x2F,0x37,0x34,0xFB, // Thank you for the (18 bytes)
        0x3E,0x2D,0x34,0x63,0x0A,0x18,0x0A,0x37,0x30,0x65,0x34,0x0A,0x3C,0x30,0x33,0x34,0xFB, // ore. I have made (17 bytes)
        0xF1,gear,0xEC, // [WEAPON]. (3 bytes)
        // Bank 3 Text 0x15 - Text if you have turned in the ore and don't have ore in your inventory
        0x18,0x0A,0x30,0x3C,0x0A,0x2D,0x34,0x2F,0x38,0x2D,0x34,0x33,0x63,0xFB, // I am retired. (14 bytes)
        0x1F,0x3B,0x34,0x30,0x2E,0x34,0x0A,0x3B,0x34,0x30,0x65,0x34,0xEC, // Please leave. (13 bytes)
        // Bank 3 Text 0x16 - Now unused (1 byte)
        0xFF,
    };

    for (unsigned int i = 0; i < blacksmithText.size(); ++i)
    {
        myRom->WriteByte(0x2C613 + i, blacksmithText[i]);
    }

    // Free the currently unused bytes
    unsigned int numFreedBytes = 0x2C777 - 0x2C613 - blacksmithText.size();
    for (unsigned int i = 0; i < numFreedBytes; ++i)
    {
        myRom->WriteByte(0x2C613 + blacksmithText.size() + i, 0x00);
    }

    // Finally update the pointers to the new text
    myRom->WriteByte(0x2C024, 0x3B);
    myRom->WriteByte(0x2C025, 0x61);
    myRom->WriteByte(0x2C026, 0x7D);
}

void Randomizer::MoveGemswordToSimaHuisHut()
{
    // Move the actual gemsword
    myRom->WriteByte(0x30A12, 0x01);
    myRom->WriteByte(0x30A52, 0x01);
    myRom->WriteByte(0x30A92, 0x0A);
    myRom->WriteByte(0x30AD2, 0x1E);

    // Remove the gemsword room in Luo Yang
    // (Easier and nicer than removing the discolored tile
    myRom->WriteByte(0x1F6F2, 0x15);
}

void Randomizer::MakeIntroLetterUseful()
{
    // Text starting at AAB3 (2AAC3):
    std::vector<BYTE> newText {
        // Text 4C (offset 0)
        0xF9,0x2D,0x27, // Event for checking for the intro letr (3 bytes)
        // Text 4D (offset 3)
        0x1F,0x2D,0x34,0x2E,0x34,0x3D,0x2F,0x0A,0x30,0x0A,0x3B,0x34,0x2F,0x2F,0x34,0x2D,0xFB, // Present a letter (17 bytes)
        0x3E,0x35,0x0A,0x38,0x3D,0x2F,0x2D,0x3E,0x33,0x64,0x32,0x2F,0x38,0x3E,0x3D,0xFB, // of introduction (16 bytes)
        0x3E,0x2D,0x0A,0x3B,0x34,0x30,0x65,0x34,0x0A,0x30,0x2F,0x0A,0x3E,0x3D,0x32,0x34,0xEC, // or leave at once. (17 bytes)
        // Text 4E (offset 53)
        0x12,0x3E,0xF0,0xF0,0x90,0xEB,0xF0,0x1E,0xEC, // Come in please. (9 bytes)
        // Text 4F (offset 62)
        0xF9,0xC0,0x17, // Event for "Come in please" + moving the lady (3 bytes)
        // Text 50 (offset 65)
        0x18,0x0A,0x35,0x3E,0x64,0x3D,0x33,0x0A,0x30,0x0A,0x3B,0x3E,0x32,0x3A,0x34,0x33,0xFB, // I found a locked (17 bytes)
        0x2E,0x30,0x35,0x34,0x0A,0x38,0x3D,0x0A,0x1B,0x64,0x3E,0x0A,0x28,0x30,0x3D,0x36,0xFB, // safe in Luo Yang (17 bytes)
        0x30,0x3D,0x33,0x0A,0x3F,0x64,0x2F,0x0A,0x38,0x2F,0x0A,0x31,0x34,0x37,0x38,0x3D,0x33,0xFD, // and put it behind> (18 bytes)
        0x2F,0x37,0x30,0x2F,0x0A,0x39,0x30,0x2D,0x63,0x0A,0x18,0x35,0x0A,0x68,0x3E,0x64,0xFB, // that jar. If you (17 bytes)
        0x32,0x30,0x3D,0x0A,0x3E,0x3F,0x34,0x3D,0x0A,0x38,0x2F,0x62,0x0A,0x68,0x3E,0x64,0xFB, // can open it, you (17 bytes)
        0x3C,0x30,0x68,0x0A,0x3A,0x34,0x34,0x3F,0x0A,0x38,0x2F,0x2E,0xFD, // may keep its> (13 bytes)
        0x32,0x3E,0x3D,0x2F,0x34,0x3D,0x2F,0x2E,0xEC, // contents. (9 bytes)
    };

    for (unsigned int i = 0; i < newText.size(); ++i)
    {
        myRom->WriteByte(0x2AAC3 + i, newText[i]);
    }

    //0x2AB7F is the start for text id 51
    const unsigned int NUM_TEXT_BYTES_TO_RECLAIM = 0x2AB7F - 0x2AAC3 - newText.size();
    for (unsigned int i = 0; i < NUM_TEXT_BYTES_TO_RECLAIM; ++i)
    {
        myRom->WriteByte(0x2AAC3 + newText.size() + i, 0x00);
    }

    // AAB3 is the start:
    // 4C (offset 0) = AAB3
    // 4D (offset 3) = AAB6
    // 4E (offset 53 (0x35)) = AAE8
    // 4F (offset 62 (0x3E)) = AAF1
    // 50 (offset 65 (0x41)) = AAF4

    // no need to overwrite the 4C pointer, since it didn't change
    myRom->WriteByte(0x2815D, 0xB6);
    myRom->WriteByte(0x2815E, 0xE8);
    myRom->WriteByte(0x2815F, 0xF1);
    myRom->WriteByte(0x28160, 0xF4);
    // 4C - 4F already have AA as their pointer MSB
    myRom->WriteByte(0x28360, 0xAA);


    // Starts at 0x3673D
    std::vector<BYTE> newCode1 {
        0xA9, 0x1A,       // LDA #$1A (1A is the Intro Letr)
        0x85, 0xF2,       // STA $F2
        // This subroutine checks the party inventory for the item in $F2,
        // removes it if it finds it, and sets C to indicate if it was found.
        0x20, 0x39, 0x9E, // JSR $9E39
        0x90, 0x0B,       // BCC ($A741) (the second jump instruction)
        // We're here if the intro letr was found.
        // Setting 670Eb0 to 1 will make the lady use the second text on
        // subsequent entries to the hut.
        0xAD, 0x0E, 0x67, // LDA $670E
        0x09, 0x01,       // ORA #$01
        0x8D, 0x0E, 0x67, // LDA $670E
        // Head to other new code to trigger the text + lady move
        0x4C, 0xC0, 0x97, // JMP $97C0
        // Head to other new code for the failed-to-find intro letr path
        0x4C, 0xC8, 0x97, // JMP $97C8
    };

    for (unsigned int i = 0; i < newCode1.size(); ++i)
    {
        myRom->WriteByte(0x3673D + i, newCode1[i]);
    }

    // Starts at 0x357D0
    std::vector<BYTE> newCode2 {
        // success path
        0x20, 0x0D, 0xD1, // JSR $D10D
        0x4E, 0x01,       // Come in please
        // Jump to lady moving event (it will take care of returning)
        0x4C, 0x18, 0xA7, // JMP $A718
        // Into letr not found path
        // Check if we've turned in the intro letr previously by checking $670Eb0
        0xAD, 0x0E, 0x67, // LDA $67OE
        0x29, 0x01,       // AND #$01
        // If we turned it in before, go ahead with the success path
        0xD0, 0xF1,       // BNE $(97C0)
        // We don't have it and have never turned it in
        0x20, 0x0D, 0xD1, // JSR $D10D
        0x4D, 0x01,       // "Present a letter..."
        // Get us out of here
        0x4C, 0xA6, 0xC4, // JMP $C4A6 (switches back to bank E, then returns)
    };

    for (unsigned int i = 0; i < newCode2.size(); ++i)
    {
        myRom->WriteByte(0x357D0 + i, newCode2[i]);
    }

    // NPC Data starts at 0x3123A
    myRom->WriteByte(0x3123B, 0x01); // Remove the "use text 3 flag" for Shui Jing
    myRom->WriteByte(0x3123E, 0x50); // Change Shui Jing text to the new text
    myRom->WriteByte(0x3123F, 0x50); // Replace text 2 with the same
    myRom->WriteByte(0x31242, 0x01); // Remove the "use text 3 flag" for the lady
    // Lady's first text is already 4C, so no need to change that
    myRom->WriteByte(0x31246, 0x4F); // Lady text 2 = "Come in please" + move

    // Finally, there's a character in Chen Cang that sets 670E to #03 when you talk
    // to him.  Remove that section from the code (and free a few bytes in the process)
    myRom->WriteByte(0x366E4, 0x05); // Move the branch that was taking us to the 670E set to now just return

    // no longer executed bytes
    for (unsigned int i = 0; i < 8; ++i)
    {
        myRom->WriteByte(0x366ED + i, 0x00);
    }
}

void Randomizer::LetWangGuiTravelToYangZhou()
{
    // Because Yang Zhou doesn't have event code, we can't just do 1 NPC swap, we actually
    // have to do 2 in order to have room for event code
    myRom->WriteByte(0x31922, 0x14); // Text Bank 1 YTile 04
    myRom->WriteByte(0x31923, 0x07); // YZone 07
    myRom->WriteByte(0x31924, 0x19); // Move around, XTile 09
    myRom->WriteByte(0x31925, 0x3E); // Trigger code + XZone 1E
    myRom->WriteByte(0x31926, 0x18); // Set text 1
    myRom->WriteByte(0x31927, 0x18); // Set text 2
    BYTE WangGuiId = myRom->ReadByte(0x31976);
    myRom->WriteByte(0x31928, WangGuiId);
    // Trigger code at 0x97E0
    myRom->WriteByte(0x31929, 0xE0);
    myRom->WriteByte(0x3192A, 0x97);
    // Terminator
    myRom->WriteByte(0x3192B, 0xFF);
    // Remaining zeros from the other NPCs
    for (unsigned int i = 0; i < 0x5; ++i)
    {
        myRom->WriteByte(0x3192C + i, 0x00);
    }

    // Trigger code written at 0x97E0 = 0x357F0
    std::vector<BYTE> eventCode {
        // Check if we've obtained Yang Jin
        0xA9, 0x02,       // LDA #$02
        0x20, 0xA0, 0x9C, // JSR $9CA0
        0xB0, 0x05,       // BCC (RTS loc)
        0xA9, 0x00,       // LDA #$00
        0x8D, 0x06, 0x05, // STA $0506
        0x60,             // RTS
    };

    for (unsigned int i = 0; i < eventCode.size(); ++i)
    {
        myRom->WriteByte(0x357F0 + i, eventCode[i]);
    }
}

void Randomizer::NewGeneralAndBattleShuffle()
{
    vector<BYTE> ids = myGenerals.GetAllGeneralIds();
    printf("%ld generals to start\n", ids.size());
    PullLuBuIds(ids);

    // Randomize Pang Tong among generals that can have 240+ INT
    vector<BYTE> strategistIds = {ZHUGE_LIANG_ID, CAO_CAO_ID, SUN_CE_ID, PANG_TONG_ID, ZHANG_ZHAO_ID, LU_XUN_ID};
    BYTE strategist = myRNG.GetRandomValueFromByteVector(strategistIds);
    myNPCManipulator.ReplacePangTong(strategist);
    SetGeneralForZone0AndRemove(strategist, ids);
    myGenerals.ScaleGeneralAllyHpForZone(strategist, 0, myRNG);
    myBattleRandomizer.PlaceGeneralInBattle(strategist, 0x4B);

    // Next thing is to pull out all of the Generals we don't want to randomize
    SetGeneralForZone0AndRemove(LIU_BEI_ID, ids); // Since he always starts in standard mode
    PullIdFromVector(SIMA_YI_ID, ids); // We don't randomize these
    PullIdFromVector(CAO_PI_ID, ids);
    PullIdFromVector(SUN_QUAN_ID, ids);
    // However, we want to go ahead and scale those generals tactics;
    myGenerals.ScaleSpecialGenerals(myRNG);

    // We can pull these one by one as the required code is modified, but for
    // now, we can hardcode all of the already hardcoded talk-and-join generals
    // to be their vanilla characters.  We're going to put them before the
    // removal of the random starting generals, but they will be moved to the
    // random pool as they get implemented.

    BYTE SongYongId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(SongYongId, ids);
    BYTE SongRenId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(SongRenId, ids);
    myNPCManipulator.ReplaceSongYongAndSongRen(SongYongId, SongRenId);

    BYTE MiZhuId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(MiZhuId, ids);
    myNPCManipulator.ReplaceMiZhu(MiZhuId);

    BYTE ChenDengId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(ChenDengId, ids);
    myNPCManipulator.ReplaceChenDeng(ChenDengId);

    BYTE LiuFengId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(LiuFengId, ids);
    myNPCManipulator.ReplaceLiuFeng(LiuFengId);

    BYTE HuoHuId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(HuoHuId, ids);
    myNPCManipulator.ReplaceHuoHu(HuoHuId);

    BYTE GuanPingId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(GuanPingId, ids);
    myNPCManipulator.ReplaceGuanPing(GuanPingId);

    BYTE YangJinId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(YangJinId, ids);
    myNPCManipulator.ReplaceYangJin(YangJinId);

    BYTE WangGuiId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(WangGuiId, ids);
    myNPCManipulator.ReplaceWangGui(WangGuiId);

    BYTE ZhouChaoId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(ZhouChaoId, ids);
    myNPCManipulator.ReplaceZhouChao(ZhouChaoId);

    BYTE XuZheId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(XuZheId, ids);
    myNPCManipulator.ReplaceXuZhe(XuZheId);

    BYTE ZhouCangId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(ZhouCangId, ids);
    myNPCManipulator.ReplaceZhouCang(ZhouCangId);

    BYTE MaSuId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(MaSuId, ids);
    myNPCManipulator.ReplaceMaSu(MaSuId);

    BYTE MaLiangId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(MaLiangId, ids);
    myNPCManipulator.ReplaceMaLiang(MaLiangId);

    BYTE ZhaoYunId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(ZhaoYunId, ids);
    myNPCManipulator.ReplaceZhaoYun(ZhaoYunId);

    BYTE GuanXingId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(GuanXingId, ids);
    myNPCManipulator.ReplaceGuanXing(GuanXingId);

    BYTE ZhangBaoId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(ZhangBaoId, ids);
    myNPCManipulator.ReplaceZhangBao(ZhangBaoId);

    BYTE JiangWeiId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(JiangWeiId, ids);
    myNPCManipulator.ReplaceJiangWei(JiangWeiId);

    BYTE ZhugeLiangId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(ZhugeLiangId, ids);
    myNPCManipulator.ReplaceZhugeLiang(ZhugeLiangId);

    // Huang Zhong & Wei Yan have zone 5 enemy stats but zone 0 ally HP
    // Both are hardcoded to only appear at Chang Sha
    BYTE HuangZhongId = myRNG.GetRandomValueFromByteVector(ids);
    myGenerals.SetAndScaleGeneralForZone(HuangZhongId, 5, myRNG);
    myGenerals.ScaleGeneralAllyHpForZone(HuangZhongId, 0, myRNG);
    PullIdFromVector(HuangZhongId, ids);
    myBattleRandomizer.PlaceGeneralInBattle(HuangZhongId, 0x1F);
    BYTE WeiYanId = myRNG.GetRandomValueFromByteVector(ids);
    myGenerals.SetAndScaleGeneralForZone(WeiYanId, 5, myRNG);
    myGenerals.ScaleGeneralAllyHpForZone(WeiYanId, 0, myRNG);
    PullIdFromVector(WeiYanId, ids);
    myBattleRandomizer.PlaceGeneralInBattle(WeiYanId, 0x1F);
    myNPCManipulator.ReplaceHuangZhongAndWeiYan(HuangZhongId, WeiYanId);

    // Same logic with Ma Chao & Ma Dai, but both are at Luo
    BYTE MaChaoId = myRNG.GetRandomValueFromByteVector(ids);
    myGenerals.SetAndScaleGeneralForZone(MaChaoId, 6, myRNG);
    myGenerals.ScaleGeneralAllyHpForZone(MaChaoId, 0, myRNG);
    PullIdFromVector(MaChaoId, ids);
    myBattleRandomizer.PlaceGeneralInBattle(MaChaoId, 0x24);
    BYTE MaDaiId = myRNG.GetRandomValueFromByteVector(ids);
    myGenerals.SetAndScaleGeneralForZone(MaDaiId, 6, myRNG);
    myGenerals.ScaleGeneralAllyHpForZone(MaDaiId, 0, myRNG);
    PullIdFromVector(MaDaiId, ids);
    myBattleRandomizer.PlaceGeneralInBattle(MaDaiId, 0x24);
    myNPCManipulator.ReplaceMaChaoAndMaDai(MaChaoId, MaDaiId);

    // This is the newly available general in the Qing
    // Zhou cave that doesn't exist in vanilla
    BYTE QingZhouCaveGeneralId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(QingZhouCaveGeneralId, ids);
    myNPCManipulator.PlaceGeneralInQingZhouCave(QingZhouCaveGeneralId);

    // This is the newly available general in the Liu Kui
    // Tent that doesn't exist in vanilla
    BYTE LiuKuiTentGeneralId = myRNG.GetRandomValueFromByteVector(ids);
    SetGeneralForZone0AndRemove(LiuKuiTentGeneralId, ids);
    myNPCManipulator.PlaceGeneralInLiuKuiTent(LiuKuiTentGeneralId);

    // Assign Warlords to appropriate castles (vanilla for standard)
    myBattleRandomizer.PlaceGeneralInBattle(SUN_QUAN_ID, 0x36);
    myBattleRandomizer.PlaceGeneralInBattle(CAO_PI_ID, 0x39);
    myBattleRandomizer.PlaceGeneralInBattle(CAO_PI_ID, 0x3D);
    myBattleRandomizer.PlaceGeneralInBattle(SIMA_YI_ID, 0x4E);
    myBattleRandomizer.PlaceGeneralInBattle(SIMA_YI_ID, 0x3E);
    myBattleRandomizer.PlaceGeneralInBattle(SIMA_YI_ID, 0x47);

    // Grab starting generals from the rest
    // Scoping just for readability
    {
        BYTE startingGeneral1 = myRNG.GetRandomValueFromByteVector(ids);
        SetGeneralForZone0AndRemove(startingGeneral1, ids);
        BYTE startingGeneral2 = myRNG.GetRandomValueFromByteVector(ids);
        SetGeneralForZone0AndRemove(startingGeneral2, ids);

        std::vector<BYTE> startingGenerals;
        startingGenerals.push_back(0xA8);

        printf("Setting ");
        PrintName(startingGeneral1);
        printf(" as a starting general\n");
        myRom->WriteByte(0x35558, startingGeneral1);
        startingGenerals.push_back(startingGeneral1);

        printf("Setting ");
        PrintName(startingGeneral2);
        printf(" as a starting general\n");
        myRom->WriteByte(0x35559, startingGeneral2);
        startingGenerals.push_back(startingGeneral2);

        myGenerals.SetStartingGenerals(startingGenerals);

        // Set Starting generals to 0xCO
        // This uses the instructions from the ImproveInitialBattlesAndFlags method and places the
        // general IDs for the starting generals in the appropriate bytes.
        myRom->WriteByte(0x35434, startingGeneral1);
        myRom->WriteByte(0x35437, startingGeneral2);
        // This is a special case to prevent Guan Suo from getting
        // set to $C0 - just replace the Guan Suo write with a
        // starting general a second time
        myRom->WriteByte(0x3543A, startingGeneral2);
    }

    // Add The non-gemsword Lu Bus back to the mix, prevent them from being recruitable:
    ids.push_back(LU_BU_NAN_YANG_ID);
    myGenerals.SetGeneralRecruitable(LU_BU_NAN_YANG_ID, false);
    ids.push_back(LU_BU_JI_ZHOU_ID);
    myGenerals.SetGeneralRecruitable(LU_BU_JI_ZHOU_ID, false);
    ids.push_back(LU_BU_SHU_ID);
    myGenerals.SetGeneralRecruitable(LU_BU_SHU_ID, false);

    // Randomize all of the ids
    for (int i = 0; i < 100000; ++i)
    {
        int index1 = myRNG.GetRandomIndexFromByteVector(ids);
        int index2 = myRNG.GetRandomIndexFromByteVector(ids);
        swap(ids[index1], ids[index2]);
    }

    ids.push_back(LU_BU_GEMSWORD_ID);
    myGenerals.SetGeneralRecruitable(LU_BU_GEMSWORD_ID, false);

    printf("%ld generals remaining\n", ids.size());
    // Assign zones as desired (6, 12, 12, 12, 12, 24, 36, remainder)
    vector<BYTE> zone1Generals(ids.cbegin(), ids.cbegin() + 6);
    vector<BYTE> zone2Generals(ids.cbegin() + 6, ids.cbegin() + 18);
    vector<BYTE> zone3Generals(ids.cbegin() + 18, ids.cbegin() + 30);
    vector<BYTE> zone4Generals(ids.cbegin() + 30, ids.cbegin() + 42);
    vector<BYTE> zone5Generals(ids.cbegin() + 42, ids.cbegin() + 54);
    vector<BYTE> zone6Generals(ids.cbegin() + 54, ids.cbegin() + 78);
    vector<BYTE> zone7Generals(ids.cbegin() + 78, ids.cbegin() + 114);
    vector<BYTE> zone8Generals(ids.cbegin() + 114, ids.cend());

    vector<vector<BYTE>> generalsForZone {zone1Generals, zone2Generals,
                                          zone3Generals, zone4Generals,
                                          zone5Generals, zone6Generals,
                                          zone7Generals, zone8Generals};

    for (int i = 0; i < generalsForZone.size(); ++i)
    {
        for (auto& generalId : generalsForZone[i])
        {
            myGenerals.SetAndScaleGeneralForZone(generalId, i + 1, myRNG);
        }
    }
    // Now lets do some special logic for Sun Ce & Cao Cao:
    myGenerals.RandomizeCaoCaoAndSunCe(myRNG);
    myGenerals.ScaleGeneralAllyHpForZone(CAO_CAO_ID, 0, myRNG);
    myGenerals.ScaleGeneralAllyHpForZone(SUN_CE_ID, 0, myRNG);

    // Give the gemsword Lu Bu the max gear possible to increase
    // gemsword value
    myGenerals.SetGeneralWeapon(LU_BU_GEMSWORD_ID, 0x13);
    myGenerals.SetGeneralDefense(LU_BU_GEMSWORD_ID, 0x0F);

    //
    // Do Zone logic for each zone, passing in a list of generals for each zone
    printf("1.\n");
    myBattleRandomizer.RandomizeZone1(zone1Generals, myRNG);
    printf("2.\n");
    myBattleRandomizer.RandomizeZone2(zone2Generals, myRNG);
    printf("3.\n");
    myBattleRandomizer.RandomizeZone3(zone3Generals, myRNG);
    printf("4.\n");
    myBattleRandomizer.RandomizeZone4(zone4Generals, myRNG);
    printf("5.\n");
    myBattleRandomizer.RandomizeZone5(zone5Generals, myRNG);
    printf("6.\n");
    myBattleRandomizer.RandomizeZone6(zone6Generals, myRNG);
    printf("7.\n");
    myBattleRandomizer.RandomizeZone7(zone7Generals, myRNG);
    printf("8.\n");
    myBattleRandomizer.RandomizeZone8(zone8Generals, myRNG);
    printf("9.\n");

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

    myBattleRandomizer.UpdateBattles(*myRom);

    // Here's where we do some other events with deadlock checking.
    // At the moment, there is only one - the Han Zhong Bridge
    // Trigger general.
    vector<BYTE> BridgeTriggerIds = myGenerals.GetAllGeneralIds();
    PullLuBuIds(BridgeTriggerIds);
    // Remove any Lu Bus that can't ever be in our party
    PullIdFromVector(LU_BU_NAN_YANG_ID, BridgeTriggerIds);
    PullIdFromVector(LU_BU_JI_ZHOU_ID, BridgeTriggerIds);
    PullIdFromVector(LU_BU_SHU_ID, BridgeTriggerIds);
    // Remove Liu Bei, as he can leave the party
    PullIdFromVector(LIU_BEI_ID, BridgeTriggerIds);
    // Remove the Bridge-locked general
    PullIdFromVector(QingZhouCaveGeneralId, BridgeTriggerIds);

    // Set the Bridge trigger general
    BYTE HanZhongBridgeTriggerId = myRNG.GetRandomValueFromByteVector(BridgeTriggerIds);
    myNPCManipulator.ReplaceHanZhongBridgeTrigger(HanZhongBridgeTriggerId);

    // Fix the text boxes that cause long names to break stuff
    myNPCManipulator.FixLongNameTexts();

/* PREVIOUS PSEUDOISH CODE:
    vector<BYTE> ids; // Fill this with all of the generals
    pullLuBus(ids);
    // Get list of generals

    // GetRandomValue (ZhangZhao, PangTong, LiuXun*)
    // RandomizePangTong(value)
    // PangTongBattle.AddGeneral(value)
    // Remove PangTong General
    BYTE pangTongSlotId = PickPangTongSlot(!warlordMode); // Parameter is warlord mode to determine if Liu Xun is on the list
    RandomizePangTong(pangTongSlotId);
    Remove(ids, pangTongSlotId);
    General& pangTongGeneral = myGenerals.getGeneralById(pangTongSlotId);
    pangTongGeneral.randomizeForZone(0);
    pangTongGeneral.encounterable = false;
    pangTongGeneral.recruitable = false;
    pangTongGeneral.allyHp = GetRandomByte(0x01, 0x7F);

    //
    // Pull out generals we don't want to put in the pool
    if (warlordMode)
    {
        // (All warlord-related generals here in warlord mode)
        Remove(ids, ...);
    }
    else
    {
        // Remove (SimaYi, CaoPi, SunQuan)
        // Remove (LiuBei)
        // Remove (ZGL)
        remove(ids, SIMA_YI_ID);
        remove(ids, CAO_PI_ID);
        remove(ids, SUN_QUAN_ID);
        remove(ids, ZHUGE_LIANG_ID);
        remove(ids, LIU_BEI_ID);
    }

    // Pull out 2 starting generals and place them, Scale them for zone 0
    BYTE startingGeneral1 = PullRandomByteFromVector(ids);
    BYTE startingGeneral2 = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(startingGeneral1, 0);
    myGenerals.randomizeGeneralForZone(startingGeneral2, 0);
    // Pull out 22 generals and place them in a talk+place situation, scale them for zone 0

    BYTE generalId1 = PullRandomByteFromVector(ids);
    BYTE generalId2 = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId1, 0);
    myGenerals.randomizeGeneralForZone(generalId2, 0);
    myNpcRandomizer.randomizeSongYongAndSongRen(generalId1, generalId2);

    BYTE generalId = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId);
    myNpcRandomizer.randomizeMiZhe(generalId);

    generalId = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId, 0);
    myNpcRandomizer.randomizeChenDing(generalId);

    generalId = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId, 0);
    myNpcRandomizer.randomizeYangJin(generalId);

    generalId = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId, 0);
    myNpcRandomizer.randomizeWangGui(generalId);

    generalId = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId, 0);
    myNpcRandomizer.randomizeZhouChao(generalId);

    generalId = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId, 0);
    myNpcRandomizer.randomizeZhaoYun(generalId);

    generalId = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId, 0);
    myNpcRandomizer.randomizeXuZhe(generalId);

    generalId = getRandomGeneralId();
    myNpcRandomizer.RandomizerXuZheBridgeTrigger(getRandomGeneral())

    generalId = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId, 0);
    myNpcRandomizer.randomizeGuanPing(generalId);

    generalId = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId, 0);
    myNpcRandomizer.randomizeLiuFeng(generalId);

    generalId = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId, 0);
    myNpcRandomizer.randomizeZhouCang(generalId);

    generalId = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId, 0);
    myNpcRandomizer.randomizeHuoHu(generalId);

    generalId = getRandomGeneralId();
    myNpcRandomizer.RandomizeHanZhongBridgeTrigger(getRandomGeneral())

    generalId = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId, 0);
    myNpcRandomizer.randomizeZhangBao(generalId);

    generalId = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId, 0);
    myNpcRandomizer.randomizeGuanXing(generalId);

    generalId = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId, 0);
    myNpcRandomizer.randomizeJiangWei(generalId);

    generalId = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId, 0);
    myNpcRandomizer.randomizeQingZhouGeneral(generalId);

    generalId = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId, 0);
    myNpcRandomizer.randomizeLiuKuiFortGeneral(generalId);

    generalId = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId, 0);
    myNpcRandomizer.randomizePirateGeneral(generalId);

    // Pull out Huang Zhong/Wei Yan generals, place them in Chang Sha, and in Chang Sha Battle, scale them for zone 5,
    //  but scale ally hp for zone 0
    generalId1 = PullRandomByteFromVector(ids);
    generalId2 = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId1, 5);
    myGenerals.randomizeGeneralForZone(generalId2, 5);
    myNpcRandomizer.randomizeHuangZhongAndWeiYan(generalId1, generalId2);
    General& huangZhongGeneral = myGenerals.getGeneralById(generalId1);
    huangZhongGeneral.allyHp = GetRandomByte(0x01, 0x7F);
    myBattleRandomizer.PlaceGeneralInBattle(generalId1, CHANG_SHA_BATTLE);
    General& weiYanGeneral = myGenerals.getGeneralById(generalId2);
    weiYanGeneral.allyHp = GetRandomByte(0x01, 0x7F);
    myBattleRandomizer.PlaceGeneralInBattle(generalId2, CHANG_SHA_BATTLE);

    // Pull out Ma Chao/Ma Dai generals, place them in Luo Battle, and Luo Trigger.  Scale them for zone 6,
    //  but scale ally hp for zone 0
    generalId1 = PullRandomByteFromVector(ids);
    generalId2 = PullRandomByteFromVector(ids);
    myGenerals.randomizeGeneralForZone(generalId1, 6);
    myGenerals.randomizeGeneralForZone(generalId2, 6);
    myNpcRandomizer.RandomizeMaChaoAndMaDai(generalId1, generalId2);
    General& maChaoGeneral = myGenerals.getGeneralById(generalId1);
    maChaoGeneral.allyHp = GetRandomByte(0x01, 0x7F);
    myBattleRandomizer.PlaceGeneralInBattle(generalId1, LUO_BATTLE);
    General& maDaiGeneral = myGenerals.getGeneralById(generalId2);
    maDaiGeneral.allyHp = GetRandomByte(0x01, 0x7F);
    myBattleRandomizer.PlaceGeneralInBattle(generalId2, LUO_BATTLE);


    //
    // Randomize starting Warlord as appropriate (tiger leveling for Warlord, zone 0 for standard)
    if (warlordMode)
    {
        ...
    }
    else
    {
        // We still want to randomize Liu Bei
        General& liuBeiGeneral = myGenerals.getGeneralById(LIU_BEI_ID);
        liuBeiGeneral.randomizeForZone(0);
    }
    // Assign Warlords to appropriate castles (vanilla for standard)
    PlaceGeneralInBattle(CAO_PI_ID, 0x39);
    PlaceGeneralInBattle(CAO_PI_ID, 0x3D);
    PlaceGeneralInBattle(SIMA_YI_ID, 0x4E);
    PlaceGeneralInBattle(SIMA_YI_ID, 0x3E);
    PlaceGeneralInBattle(SIMA_YI_ID, 0x47);

    //
    // Put non-gemsword Lu Bus back into battle
    addNonGemswordLuBus();
    // Randomize the remaining generals order
    randomizeIds(ids);
    // Place gemsword Lu Bu in back to force him into zone 8
    ids.push_back(GEMSWORD_LU_BU_ID);

    // Assign zones as desired (6, 12, 12, 12, 12, 24, 36, remainder)
    SetZonesForGenerals(ids, 6, 12, 12, 12, 12, 24, 36);
    //
    // Do Zone logic for each zone, passing in a list of generals for each zone
    myBattleRandomizer.RandomizeZone1(vector<BYTE>(ids.cbegin(), ids.cbegin() + 6), myGenerator);
    myBattleRandomizer.RandomizeZone2(vector<BYTE>(ids.cbegin() + 6, ids.cbegin() + 18), myGenerator);
    myBattleRandomizer.RandomizeZone3(vector<BYTE>(ids.cbegin() + 18, ids.cbegin() + 30), myGenerator);
    myBattleRandomizer.RandomizeZone4(vector<BYTE>(ids.cbegin() + 30, ids.cbegin() + 42), myGenerator);
    myBattleRandomizer.RandomizeZone5(vector<BYTE>(ids.cbegin() + 42, ids.cbegin() + 54), myGenerator);
    myBattleRandomizer.RandomizeZone6(vector<BYTE>(ids.cbegin() + 54, ids.cbegin() + 78), myGenerator);
    myBattleRandomizer.RandomizeZone7(vector<BYTE>(ids.cbegin() + 78, ids.cbegin() + 114), myGenerator);
    myBattleRandomizer.RandomizeZone8(vector<BYTE>(ids.cbegin() + 114, ids.cend()), myGenerator);
    //
    // Move the Gui Yang battle location to Gui Yang front
    MoveGuiYangBattleToFront();
    // Remove Zhao Fan from Gui Yang
    RemoveGuiYangThroneBattle();
    //
    // Read first general from Ma Yuan Yi
    generalId = myBattleRandomizer.getStartingGeneral(MA_YUAN_YI_BATTLE)
    // Set Ma Yuan Yi NPC data to be first general
    myNpcRandomizer.setMaYuanYiGeneral(generalId);
    //
    // Read first general from Lu Meng
    generalId = myBattleRandomizer.getStartingGeneral(LU_MENG_BATTLE)
    // Set Lu Meng NPC data to be first general
    myNpcRandomizer.setLuMengGeneral(generalId);
    //
    // Read first general from Yuan Shang
    generalId = myBattleRandomizer.getStartingGeneral(YUAN_SHANG_BATTLE)
    // Set Yuan Shang NPC data to be first general.
    myNpcRandomizer.setYuanShangGeneral(generalId);
*/
}

static int numDigits(int num)
{
    int digits = 0;
    if (num == 0)
    {
        return 1;
    }
    while (num != 0)
    {
        num = num/10;
        digits++;
    }

    return digits;
}

static void appendNumber(std::vector<BYTE>& v, int num)
{
    int digits = numDigits(num);

    int divisor = 1;
    for (int i = 1; i < digits; ++i)
    {
        divisor *= 10;
    }

    while (divisor > 0)
    {
        v.push_back((num / divisor) % 10);
        divisor /= 10;
    }
}

void Randomizer::RewriteTitleScreen(int major, int minor, int patch, int seed)
{
    // I assume there are line pointers here, but I simply don't know where
    // they are, so I'm simply replacing the extra characters I don't want
    // display with 0xFF, but I'm not actually moving any line starts

    // Line format:
    // first 2 bytes - location to write (lower nibble of byte 2
    //                 defines horizontal start position)
    // third byte - number of bytes to write
    const int CREDIT_START = 0x349A1;
    int index = CREDIT_START;

    // adding scoping braces for readability
    // RANDOMIZER
    {
        std::vector<BYTE> randomizerLine {
            0x21,0xAB,0x0A,0x21,0x10,0x1D,0x13,0x1E,0x1C,0x18,0x29,0x14,0x21 // RANDOMIZER
        };
        // Fill remaining bytes for this line with 0xFF
        randomizerLine.resize(0x0F, 0xFF);
        // Write the data
        for (int i = 0; i < randomizerLine.size(); ++i, ++index)
        {
            myRom->WriteByte(index, randomizerLine[i]);
        }
    }


    // VERSION X.Y.Z
    {
        int combinedVersionDigits = numDigits(major) + numDigits(minor) + numDigits(patch);
        BYTE versionLineLen = 8 + combinedVersionDigits + 2; // 8 for VERSION_, 2 for dots between versions
        if (versionLineLen % 2 == 1) // Make sure we always have an even number of characters for centering
        {
            versionLineLen++;
        }
        // Set the lower nibble to a centering offset based upon the version length
        BYTE versionOffset = (0x10 - static_cast<BYTE>(versionLineLen/2)) | 0xE0;
        // Write VERSION_
        std::vector<BYTE> versionLine {0x21, versionOffset, 0x17, 0x25,0x14,0x21,0x22,0x18,0x1E,0x1D,0x0A};
        if (combinedVersionDigits % 2 == 1) // Add an extra space to get us to an even number of characters
        {
            versionLine.push_back(0x0A);
        }
        // Add patch numbers
        appendNumber(versionLine, major);
        versionLine.push_back(0x63);
        appendNumber(versionLine, minor);
        versionLine.push_back(0x63);
        appendNumber(versionLine, patch);
        // Fill remaining bytes for this line with 0xFF
        versionLine.resize(0x1A, 0xFF);
        // Write the data
        for (int i = 0; i < versionLine.size(); ++i, ++index)
        {
            myRom->WriteByte(index, versionLine[i]);
        }
    }

    // SEED X
    {
        int seedDigits = numDigits(seed);
        BYTE seedLineLen = 5 + seedDigits;
        if (seedLineLen % 2 == 1) // Make sure we always have an even number of characters for centering
        {
            seedLineLen++;
        }
        // Set the lower nibble to a centering offset based upon the seed length
        BYTE seedOffset = (0x10 - static_cast<BYTE>(seedLineLen/2)) | 0x20;
        // Write SEED_
        std::vector<BYTE> seedLine {0x22,seedOffset,0x12,0x22,0x14,0x14,0x13,0x0A};
        if (seedDigits % 2 == 0) // Write an extra space if needed to get to even character count
        {
            seedLine.push_back(0x0A);
        }
        // Add seed number
        appendNumber(seedLine, seed);
        // Fill remaining bytes for this line with 0xFF
        seedLine.resize(0x15, 0xFF);
        // Write the data
        for (int i = 0; i < seedLine.size(); ++i, ++index)
        {
            myRom->WriteByte(index, seedLine[i]);
        }
    }

    // 4 BLANK LINES
    {
        std::vector<BYTE> blankLine0 {0x22, 0x65, 0x0F};
        blankLine0.resize(0x12, 0xFF);
        for (int i = 0; i < blankLine0.size(); ++i, ++index)
        {
            myRom->WriteByte(index, blankLine0[i]);
        }

        std::vector<BYTE> blankLine1 {0x22, 0xA2, 0x1D};
        blankLine1.resize(0x20, 0xFF);
        for (int i = 0; i < blankLine1.size(); ++i, ++index)
        {
            myRom->WriteByte(index, blankLine1[i]);
        }

        std::vector<BYTE> blankLine2 {0x22, 0xE6, 0x13};
        blankLine2.resize(0x16, 0xFF);
        for (int i = 0; i < blankLine2.size(); ++i, ++index)
        {
            myRom->WriteByte(index, blankLine2[i]);
        }

        std::vector<BYTE> blankLine3 {0x23, 0x29, 0x0B};
        blankLine3.resize(0x0E, 0xFF);
        for (int i = 0; i < blankLine3.size(); ++i, ++index)
        {
            myRom->WriteByte(index, blankLine3[i]);
        }
    }

    // CREATED BY THILOTILO
    {
        // Text data
        std::vector<BYTE> createdByLine {
            0x23,0x66,0x18,0x12,0x21,0x14,0x10,0x23,0x14,0x13,0x0A,0x11,0x28,0x0A,0x23,0x17,0x18,0x1B,0x1E,0x23,0x18,0x1B,0x1E};
        // Fill remaining bytes for this line with 0xFF
        createdByLine.resize(0x1B, 0xFF);
        // Write the data
        for (int i = 0; i < createdByLine.size(); ++i, ++index)
        {
            myRom->WriteByte(index, createdByLine[i]);
        }
    }
}

}
