#include "General.h"

using namespace std;

namespace DoaERando {

const int STRENGTH_OFFSET = 0;
const int INTELLIGENCE_OFFSET = 1;
const int ZONE_OFFSET = 2;
const int WEAPON_OFFSET = 4;
const BYTE WEAPON_BITMASK = 0x1F;
const int ENEMY_DEFENSE_OFFSET = 5;
const int ENEMY_SOLDIERS_OFFSET = 6;
const int ALLY_SOLDIERS_OFFSET = 7;
const int TACTICS_LEVEL_OFFSET = 8;
const int AGILITY_OFFSET = 9;


General::General(int address, BYTE id, ROM& rom)
    : address(address)
    , id(id)
    , count(1)
    , encounterable(IsEncounterable(id, rom))
    , recruitable(IsRecruitable(id, rom))
{
    strength = rom.ReadByte(address + STRENGTH_OFFSET);
    intelligence = rom.ReadByte(address + INTELLIGENCE_OFFSET);
    zone = rom.ReadByte(address + ZONE_OFFSET);
    weaponId = rom.ReadByte(address + WEAPON_OFFSET) & WEAPON_BITMASK;
    enemyDefenseId = rom.ReadByte(address + ENEMY_DEFENSE_OFFSET);
    enemySoldierExponent = rom.ReadByte(address + ENEMY_SOLDIERS_OFFSET);
    allySoldierExponent = rom.ReadByte(address + ALLY_SOLDIERS_OFFSET);
    enemyMaxTacticLevel = rom.ReadByte(address + TACTICS_LEVEL_OFFSET);
    agility = rom.ReadByte(address + AGILITY_OFFSET);
    ReadName(rom);
}

void General::ScaleWeaponForZone(RNG& rng)
{ 
    const BYTE MIN_WEAPON[9] = {0x00, 0x00, 0x04, 0x05, 0x06, 0x06, 0x09, 0x0B, 0x0E};
    const BYTE MAX_WEAPON[9] = {0x13, 0x05, 0x05, 0x06, 0x06, 0x07, 0x09, 0x0D, 0x13};

    weaponId = rng.GetRandomByte(MIN_WEAPON[zone], MAX_WEAPON[zone]);
}

void General::ScaleArmorForZone(RNG& rng)
{ 
    const BYTE MIN_ARMOR[9] = {0x0, 0x0, 0x4, 0x5, 0x4, 0x7, 0xA, 0xC, 0xE};
    const BYTE MAX_ARMOR[9] = {0xF, 0x3, 0x5, 0x6, 0x5, 0x7, 0xB, 0xF, 0xF};

    enemyDefenseId = rng.GetRandomByte(MIN_ARMOR[zone], MAX_ARMOR[zone]);
}

void General::ScaleMaxTacticForZone(RNG& rng)
{ 
    const BYTE MAX_TACTIC[9] = {50, 10, 15, 20, 25, 30, 35, 40, 50};

    enemyMaxTacticLevel = rng.GetRandomByte(0, MAX_TACTIC[zone]);
}

void General::ScaleSoldiersForZone(RNG& rng)
{ 
    const BYTE MIN_SOLDIERS[9] = {1,   10,  24, 33, 48, 53, 64, 82,  99};
    const BYTE MAX_SOLDIERS[9] = {121, 27,  32, 47, 61, 60, 79, 111, 121};

    BYTE randomizedSoldiers = rng.GetRandomByte(MIN_SOLDIERS[zone], MAX_SOLDIERS[zone]);
    {
        if (allySoldierExponent & 0x80)
        {
            printf("Tiger detected, not modifying the first value.\n");
            printf("Changing %s's enemy soldier exponent from %d to %d\n", name, 
                                                                           enemySoldierExponent,
                                                                           randomizedSoldiers);
            enemySoldierExponent = randomizedSoldiers;
        }
        else
        {
            printf("Changing %s's soldier exponent from %d (and %d) to %d\n", name,
                                                                              enemySoldierExponent,
                                                                              allySoldierExponent,
                                                                              randomizedSoldiers);
            enemySoldierExponent = randomizedSoldiers;
            allySoldierExponent = randomizedSoldiers;
        }
    }
}

void General::UpdateGeneral(ROM& rom)
{
    rom.WriteByte(address + STRENGTH_OFFSET, strength);
    rom.WriteByte(address + INTELLIGENCE_OFFSET, intelligence);
    rom.WriteByte(address + ZONE_OFFSET, zone);
    BYTE weaponPaletteByte = rom.ReadByte(address + WEAPON_OFFSET);
    weaponPaletteByte &= (~WEAPON_BITMASK);
    weaponPaletteByte |= weaponId;
    rom.WriteByte(address + WEAPON_OFFSET, weaponPaletteByte);
    rom.WriteByte(address + ENEMY_DEFENSE_OFFSET, enemyDefenseId);
    rom.WriteByte(address + ENEMY_SOLDIERS_OFFSET, enemySoldierExponent);
    rom.WriteByte(address + ALLY_SOLDIERS_OFFSET, allySoldierExponent);
    rom.WriteByte(address + TACTICS_LEVEL_OFFSET, enemyMaxTacticLevel);
    rom.WriteByte(address + AGILITY_OFFSET, agility);

    for (BYTE i = id; i < id + count - 1; ++i)
    {
        WriteEncounterableBit(i, false, rom);
        WriteRecruitableBit(i, false, rom);
    }
    WriteEncounterableBit(id + count - 1, encounterable, rom);
    WriteRecruitableBit(id + count - 1, recruitable, rom);
}

bool General::IsEncounterable(BYTE id, ROM& rom)
{
    const int ENCOUNTERABLE_OFFSET = 0x35722;
    const BYTE bitmask = 1 << (7 - (id % 8));
    return ((rom.ReadByte(ENCOUNTERABLE_OFFSET + (id / 8)) & bitmask) != 0);
}

void General::WriteEncounterableBit(BYTE id, bool isEncounterable, ROM& rom)
{
    const int ENCOUNTERABLE_OFFSET = 0x35722;
    const BYTE bitmask = 1 << (7 - (id % 8));
    BYTE currentByte = rom.ReadByte(ENCOUNTERABLE_OFFSET + (id / 8));
    BYTE newByte;
    if (isEncounterable)
    {
        newByte = currentByte | bitmask;
    }
    else
    {
        newByte = currentByte & (~bitmask);
    }

    rom.WriteByte(ENCOUNTERABLE_OFFSET + (id / 8), newByte);
}

bool General::IsRecruitable(BYTE id, ROM& rom)
{
    const int RECRUITABLE_OFFSET = 0x3B153;
    const BYTE bitmask = 1 << (7 - (id % 8));
    return ((rom.ReadByte(RECRUITABLE_OFFSET + (id / 8)) & bitmask) != 0);
}

void General::WriteRecruitableBit(BYTE id, bool isRecruitable, ROM& rom)
{
    const int RECRUITABLE_OFFSET = 0x3B153;
    const BYTE bitmask = 1 << (7 - (id % 8));
    BYTE currentByte = rom.ReadByte(RECRUITABLE_OFFSET + (id / 8));
    BYTE newByte;
    if (isRecruitable)
    {
        newByte = currentByte | bitmask;
    }
    else
    {
        newByte = currentByte & (~bitmask);
    }

    rom.WriteByte(RECRUITABLE_OFFSET + (id / 8), newByte);
}

void General::ReadName(ROM& rom)
{
    const int NAME_OFFSET = 20;
    int nameOffset = 0;
    while (rom.ReadByte(address + nameOffset + NAME_OFFSET) != 0xFF && nameOffset < 19)
    {
        char character = rom.ReadByte(address + nameOffset + NAME_OFFSET) + 'A' - 0x10;
        switch(rom.ReadByte(address + nameOffset + NAME_OFFSET))
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

        name[nameOffset++] = character;
    }
    name[nameOffset] = 0x00;
}

void General::DumpGeneral()
{
    printf("%s:\n", name);
    printf("\tid: 0x%02X\n", id);
    printf("\taddress: 0x%X\n", address);
    printf("\tcount: %d\n", count);
    printf("\tSTR: %d\n", strength);
    printf("\tINT: %d\n", intelligence);
    printf("\tAGI: %d\n", agility);
    printf("\tzone: %d\n", zone);
    printf("\tHP exp: %d, %d\n", enemySoldierExponent, allySoldierExponent);
    printf("\tDefense: %d\n", enemyDefenseId);
    printf("\tMax Tactic Level: %d\n", enemyMaxTacticLevel);
    printf("\tWeapon: 0x%02X\n", weaponId);
    printf("\tEncounterable? %d\n", encounterable);
    printf("\tRecruitable? %d\n", recruitable);
}

}
