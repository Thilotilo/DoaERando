#include "BattleRandomizer.h"
#include "GeneralIds.h"

using namespace std;

namespace DoaERando {

const int BATTLES_OFFSET = 0x30D70;

Battle::Battle(int id)
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

    zone = zoneForBattle[id];
    battleId = id;
    for (int i = 0; i < 5; ++i)
    {
        generals[i] = 0x00;
    }
    slotsRemaining = 5;
    battleModified = false;
}

void Battle::OrganizeBattle()
{
    // Reorder battles so that Generals come first, then rebel forces, then empty slots
    std::vector<BYTE> generalList;
    int numRebels = 0;

    // First, let's just collect the generals and the number of rebel forces
    for (int i = 0; i < 5; ++i)
    {
        if (generals[i] > 0x04)
        {
            generalList.push_back(generals[i]);
        }
        if (generals[i] == 0x04)
        {
            numRebels++;
        }
    }

    int index = 0;

    // assign the generals
    for (auto& general : generalList)
    {
        generals[index++] = general;
    }
    // assign the rebel forces
    for (int i = 0; i < numRebels; ++i)
    {
        generals[index++] = 0x04;
    }
    // assign empty slots
    while (index < 0x05)
    {
        generals[index++] = 0x00;
    }

    battleModified = true;
}

BattleRandomizer::BattleRandomizer()
{
    for (int i = 0; i < 0x50; ++i)
    {
        myBattles.push_back(Battle(i));
    }
}

bool BattleRandomizer::PlaceGeneralInBattle(BYTE id, int battleId)
{
    if (myBattles[battleId].slotsRemaining == 0)
    {
        return false;
    }
    for (int i = 0; i < 5; ++i)
    {
        if (myBattles[battleId].generals[i] == id)
        {
            return false;
        }
        if (myBattles[battleId].generals[i] == 0x00)
        {
            myBattles[battleId].generals[i] = id;
            myBattles[battleId].slotsRemaining--;
            return true;
        }
    }
    return false;
}

void BattleRandomizer::FillBattlesRandomly(vector<BYTE> generalIds, vector<int> battleIds, RNG& rng)
{
    for (auto& battleId : battleIds)
    {
        int startingIndex = 5 - myBattles[battleId].slotsRemaining;
        for (int i = startingIndex; i < 5; ++i)
        {
            int value = rng.GetRandomInt(0, 9);
            // 60% chance of a general unless this is slot 0, in which case it's 100%
            if (value < 6 || i == 0)
            {
                // Get General
                int tries = 20;
                bool success = false;
                while (!success && tries-- > 0)
                {
                    BYTE generalToInsert = rng.GetRandomValueFromByteVector(generalIds);
                    success = PlaceGeneralInBattle(generalToInsert, battleId);
                }
                if (!success)
                {
                    // There are no more generals in this zone.
                    // Set Rebel Force instead
                    myBattles[battleId].generals[i] = 0x04;
                    myBattles[battleId].slotsRemaining--;
                }
            }
            // 40% chance of a rebel force
            else if (value < 9)
            {
                // Set Rebel Force
                myBattles[battleId].generals[i] = 0x04;
                myBattles[battleId].slotsRemaining--;
            }
        }
        myBattles[battleId].OrganizeBattle();
    }
}

void BattleRandomizer::FillBattleWithOnlyGenerals(vector<BYTE> generalIds, int battleId, RNG& rng)
{
    int startingIndex = 5 - myBattles[battleId].slotsRemaining;
    for (int i = startingIndex; i < 5; ++i)
    {
        bool success = false;
        while (!success)
        {
            BYTE generalToInsert = rng.GetRandomValueFromByteVector(generalIds);
            success = PlaceGeneralInBattle(generalToInsert, battleId);
        }
    }
    myBattles[battleId].OrganizeBattle();
}

void BattleRandomizer::RandomizeZone1(Generals& generals, RNG& rng)
{
    // Zone 1 special condition is that we want the following:
    // 3 generals, 1 at each of the 3 locations
    // Those 3 generals at both the front and back locations of Qing Zhou
    // All other generals at one of the 3 initial locations to ensure all of them are encountered.

    std::vector<BYTE> generalIds = generals.GetGeneralIdsFromZone(1);

    BYTE SpecialGeneral1 = rng.GetRandomValueFromByteVector(generalIds);
    BYTE SpecialGeneral2;
    BYTE SpecialGeneral3;
    do {
        SpecialGeneral2 = rng.GetRandomValueFromByteVector(generalIds);
    } while (SpecialGeneral2 == SpecialGeneral1);
    do {
        SpecialGeneral3 = rng.GetRandomValueFromByteVector(generalIds);
    } while (SpecialGeneral3 == SpecialGeneral2 || SpecialGeneral3 == SpecialGeneral1);

    // Assign the special generals to each battle
    PlaceGeneralInBattle(SpecialGeneral1, 0x00);
    PlaceGeneralInBattle(SpecialGeneral2, 0x01);
    PlaceGeneralInBattle(SpecialGeneral3, 0x02);

    PlaceGeneralInBattle(SpecialGeneral1, 0x03);
    PlaceGeneralInBattle(SpecialGeneral2, 0x03);
    PlaceGeneralInBattle(SpecialGeneral3, 0x03);

    PlaceGeneralInBattle(SpecialGeneral1, 0x05);
    PlaceGeneralInBattle(SpecialGeneral2, 0x05);
    PlaceGeneralInBattle(SpecialGeneral3, 0x05);

    std::vector<BYTE> nonSpecialGeneralIds;
    for (auto& id : generalIds)
    {
        if (id != SpecialGeneral1 && id != SpecialGeneral2 && id != SpecialGeneral3)
        {
            nonSpecialGeneralIds.push_back(id);
        }
    }

    // Make sure every general goes to a relevant battle
    for (auto& id : nonSpecialGeneralIds)
    {
        int battleId = rng.GetRandomInt(0, 2);
        while (!PlaceGeneralInBattle(id, battleId))
        {
            battleId = ((battleId + 1) % 3);
        }
    }

    std::vector<int> battlesToFill = {0x00, 0x01, 0x02, 0x04};
    // Now fill first 4 battles with generals/rebel forces
    FillBattlesRandomly(nonSpecialGeneralIds, battlesToFill, rng);

    // We want the front door to have filled generals.
    FillBattleWithOnlyGenerals(nonSpecialGeneralIds, 0x05, rng);

    // Battle 3, which is the back door, will not have any other generals
    myBattles[0x03].OrganizeBattle();
}

void BattleRandomizer::RandomizeZone2(Generals& generals, RNG& rng)
{
    // We may want to add Lu Bu logic into this one, but for now, the only special
    // logic for this zone is that we won't let Luo Yang be the sole location for
    // a general.

    std::vector<BYTE> generalIds = generals.GetGeneralIdsFromZone(2);

    // Luo Yang (0A) is bugged right now, and we don't fight the battle, so don't make it the
    // only location for a general.
    vector<int> battlesToFill = {0x06, 0x07, 0x08, 0x09, /*0x0A, */0x0B, 0x0C, 0x0D};
    // Make sure every general goes to a relevant battle
    for (auto& id : generalIds)
    {
        int battleIndex = rng.GetRandomInt(0, battlesToFill.size() - 1);
        int battleId = battlesToFill[battleIndex];
        while (!PlaceGeneralInBattle(id, battleId))
        {
            battleIndex = ((battleIndex + 1) % battlesToFill.size());
            battleId = battlesToFill[battleIndex];
        }
    }

    // Even thought we didn't make it the only location for a general, we do still
    // want to fill the Luo Yang battle in case we can fight it.
    battlesToFill.push_back(0x0A);
    // Now fill all battles with generals/rebel forces
    FillBattlesRandomly(generalIds, battlesToFill, rng);
}

void BattleRandomizer::RandomizeZone3(Generals& generals, RNG& rng)
{
    // We may want to add special logic to handle Yuan Shu's assassination/joining of
    // Yuan Shao in Ji Zhou, but for now, we're just ignoring that.  There are no
    // special considerations in this zone.

    std::vector<BYTE> generalIds = generals.GetGeneralIdsFromZone(3);

    vector<int> battlesToFill = {0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13};
    // Make sure every general goes to a relevant battle
    for (auto& id : generalIds)
    {
        int battleIndex = rng.GetRandomInt(0, battlesToFill.size() - 1);
        int battleId = battlesToFill[battleIndex];
        while (!PlaceGeneralInBattle(id, battleId))
        {
            battleIndex = ((battleIndex + 1) % battlesToFill.size());
            battleId = battlesToFill[battleIndex];
        }
    }

    // Now fill all battles with generals/rebel forces
    FillBattlesRandomly(generalIds, battlesToFill, rng);
}

void BattleRandomizer::RandomizeZone4(Generals& generals, RNG& rng)
{
    // We may want to add Yuan Shu joining Yuan Shao logic here in the future, but
    // for now, the only special consideration we're giving here is that we want the
    // first general in the Yuan Shao ambush (48) to be the first general in the Yuan Shao
    // battle at Ji Zhou (1A)
    // Update: We want to relegate the Yuan Shang battle (17) to duplicates, as it isn't always
    // there.
    
    std::vector<BYTE> generalIds = generals.GetGeneralIdsFromZone(4);

    BYTE SpecialGeneral = rng.GetRandomValueFromByteVector(generalIds);
    
    // Assign the special generals to each battle
    PlaceGeneralInBattle(SpecialGeneral, 0x48);
    PlaceGeneralInBattle(SpecialGeneral, 0x1A);

    // All the other battles can be filled like normal.
    vector<int> battlesToFill = {0x14, 0x15, 0x16, /*0x17, */0x18, 0x19, 0x1A};
    // Make sure every general goes to a relevant battle
    for (auto& id : generalIds)
    {
        int battleIndex = rng.GetRandomInt(0, battlesToFill.size() - 1);
        int battleId = battlesToFill[battleIndex];
        while (!PlaceGeneralInBattle(id, battleId))
        {
            battleIndex = ((battleIndex + 1) % battlesToFill.size());
            battleId = battlesToFill[battleIndex];
        }
    }

    // There is a non-existent battle with Zhou Cang (49) that would fit in this zone. We
    // can't trigger it yet, but let's fill it anyway (only with repeat generals, of course)
    battlesToFill.push_back(0x49);
    battlesToFill.push_back(0x17); // Yuan Shang Battle
    battlesToFill.push_back(0x48); // Ambush
    // Now fill all battles with generals/rebel forces
    FillBattlesRandomly(generalIds, battlesToFill, rng);
}

void BattleRandomizer::RandomizeZone5(Generals& generals, RNG& rng)
{
    // Ideally, we want to add a special consideration for 2 battles here.  We would like
    // the Chang Sha battles to have the Huang Zhong/Wei Yan equivalents at the town, and
    // we would like to have the Pang Tong battle (4B) yield Pang Tong, but coupling that logic
    // is beyond our scope at the moment, so we're going to fully randomize everything.
    // Note that there is no reason that Pang Tong can't trigger the recruitable flag, so no need to
    // relegate it to duplicates
    
    std::vector<BYTE> generalIds = generals.GetGeneralIdsFromZone(5);

    // All the other battles can be filled like normal.
    vector<int> battlesToFill = {0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x4B};
    // Make sure every general goes to a relevant battle
    for (auto& id : generalIds)
    {
        int battleIndex = rng.GetRandomInt(0, battlesToFill.size() - 1);
        int battleId = battlesToFill[battleIndex];
        while (!PlaceGeneralInBattle(id, battleId))
        {
            battleIndex = ((battleIndex + 1) % battlesToFill.size());
            battleId = battlesToFill[battleIndex];
        }
    }

    // There is a non-existent battle with Huang Zhong/Wei Yan (4A) that would fit in this zone. We
    // can't trigger it yet, but let's fill it anyway (only with repeat generals, of course)
    battlesToFill.push_back(0x4A);
    // Now fill all battles with generals/rebel forces
    FillBattlesRandomly(generalIds, battlesToFill, rng);
}

void BattleRandomizer::RandomizeZone6(Generals& generals, RNG& rng)
{
    // Ideally, we want to add a special consideration for the Luo battle here.  We would like
    // it to yield the Ma Chao/Ma Dai equivalents when accepting their offer at the town,  but
    // coupling that logic is beyond our scope at the moment, so we're going to fully randomize everything.
    // Note that there is no reason that Lu Bu (4F) can't trigger the recruitable flag, so no need to
    // relegate it to duplicates

    std::vector<BYTE> generalIds = generals.GetGeneralIdsFromZone(6);

    // All the other battles can be filled like normal.
    vector<int> battlesToFill = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x4F};
    // Make sure every general goes to a relevant battle
    for (auto& id : generalIds)
    {
        int battleIndex = rng.GetRandomInt(0, battlesToFill.size() - 1);
        int battleId = battlesToFill[battleIndex];
        while (!PlaceGeneralInBattle(id, battleId))
        {
            battleIndex = ((battleIndex + 1) % battlesToFill.size());
            battleId = battlesToFill[battleIndex];
        }
    }

    // Now fill all battles with generals/rebel forces
    FillBattlesRandomly(generalIds, battlesToFill, rng);
}

void BattleRandomizer::RandomizeZone7(Generals& generals, RNG& rng)
{
    // There are a few special considerations here.
    // 1) The Wu Ling/Ling Ling refights (28 & 29) are difficult to trigger with the current
    //    potential sequence breaks in the open world rando.  Thus, we'll fill them,
    //    but we will relegate them to duplicates
    // 2) There is a Yan Xun battle (2C) that is not triggerable, but we'll fill it anyway
    //    (with duplicates of course)
    // 3) Sun Quan must be in the 2nd Jian Ye fight (36), and only in that fight.

    std::vector<BYTE> generalIds = generals.GetGeneralIdsFromZone(7);

    // Assign Sun Quan to his battle
    PlaceGeneralInBattle(SUN_QUAN_ID, 0x36);

    // Now get another general vector that doesn't have Sun Quan
    std::vector<BYTE> otherGeneralIds;
    for (auto& id : generalIds)
    {
        if (id != SUN_QUAN_ID)
        {
            otherGeneralIds.push_back(id);
        }
    }

    // Other than the battles mentioned above, we can now fill the battles like normal.
    vector<int> battlesToFill = {0x2A, 0x2B, 0x2D, 0x2E, 0x2F,
                                 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x4C, 0x4D};

    // Make sure every general goes to a relevant battle
    for (auto& id : otherGeneralIds)
    {
        int battleIndex = rng.GetRandomInt(0, battlesToFill.size() - 1);
        int battleId = battlesToFill[battleIndex];
        while (!PlaceGeneralInBattle(id, battleId))
        {
            battleIndex = ((battleIndex + 1) % battlesToFill.size());
            battleId = battlesToFill[battleIndex];
        }
    }

    // Now add the battles we only want duplicates in
    battlesToFill.push_back(0x28);
    battlesToFill.push_back(0x29);
    battlesToFill.push_back(0x2C);

    // Now fill all battles with generals/rebel forces
    FillBattlesRandomly(otherGeneralIds, battlesToFill, rng);
}

void BattleRandomizer::RandomizeZone8(Generals& generals, RNG& rng)
{
    // There are a few special considerations here.
    // 1) There are Chen Jiao (40) and Xu Huang(41) battles that are not triggerable, but
    //    we'll fill them anyway (with duplicates of course)
    // 2) Cao Pi must be in the Hei Nei Fortress (39) & Ru Nan 1 (3D) fights, and only
    //    in those fights.
    // 3) Sima Yi must be in the Ru Nan 2 (4E), Chen Liu Fortress (3E) & Luo Yang 2 (47) fights,
    //    and only in those fights.

    std::vector<BYTE> generalIds = generals.GetGeneralIdsFromZone(8);

    // Assign Cao Pi & Sima Yi to their battles
    PlaceGeneralInBattle(CAO_PI_ID, 0x39);
    PlaceGeneralInBattle(CAO_PI_ID, 0x3D);
    PlaceGeneralInBattle(SIMA_YI_ID, 0x4E);
    PlaceGeneralInBattle(SIMA_YI_ID, 0x3E);
    PlaceGeneralInBattle(SIMA_YI_ID, 0x47);

    // Now get another general vector that doesn't have Cao Pi or Sima Yi
    std::vector<BYTE> otherGeneralIds;
    for (auto& id : generalIds)
    {
        if (id != CAO_PI_ID && id != SIMA_YI_ID)
        {
            otherGeneralIds.push_back(id);
        }
    }

    // Other than the battles mentioned above, we can now fill the battles like normal.
    vector<int> battlesToFill = {0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
                                 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x4E};

    // Make sure every general goes to a relevant battle
    for (auto& id : otherGeneralIds)
    {
        int battleIndex = rng.GetRandomInt(0, battlesToFill.size() - 1);
        int battleId = battlesToFill[battleIndex];
        while (!PlaceGeneralInBattle(id, battleId))
        {
            battleIndex = ((battleIndex + 1) % battlesToFill.size());
            battleId = battlesToFill[battleIndex];
        }
    }

    // Now add the battles we only want duplicates in
    battlesToFill.push_back(0x40);
    battlesToFill.push_back(0x41);

    // Now fill all battles with generals/rebel forces
    FillBattlesRandomly(otherGeneralIds, battlesToFill, rng);
}


void BattleRandomizer::UpdateBattles(ROM& rom)
{
    const int BATTLE0_OFFSET = 0x30D70;

    for (auto& battle : myBattles)
    {
        if (battle.battleModified)
        {
            printf("Battle 0x%02X is now 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
                   battle.battleId, battle.generals[0], battle.generals[1],
                   battle.generals[2], battle.generals[3], battle.generals[4]);
            int battleOffset = BATTLE0_OFFSET + (5 * battle.battleId);
            for (int i = 0; i < 5; ++i)
            {
                rom.WriteByte(battleOffset + i, battle.generals[i]);
            }
        }
    }
}



}

