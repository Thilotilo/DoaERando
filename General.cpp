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

void General::ScaleWeaponForZone(Generator& generator)
{ 
    const BYTE MIN_WEAPON[9] = {0x00, 0x00, 0x04, 0x05, 0x06, 0x06, 0x09, 0x0B, 0x0E};
    const BYTE MAX_WEAPON[9] = {0x13, 0x05, 0x05, 0x06, 0x06, 0x07, 0x09, 0x0D, 0x13};

    uniform_int_distribution<BYTE> weaponDistribution(MIN_WEAPON[zone], MAX_WEAPON[zone]);
    weaponId = weaponDistribution(generator);
}

void General::ScaleArmorForZone(Generator& generator)
{ 
    const BYTE MIN_ARMOR[9] = {0x0, 0x0, 0x4, 0x5, 0x4, 0x7, 0xA, 0xC, 0xE};
    const BYTE MAX_ARMOR[9] = {0xF, 0x3, 0x5, 0x6, 0x5, 0x7, 0xB, 0xF, 0xF};

    uniform_int_distribution<BYTE> armorDistribution(MIN_ARMOR[zone], MAX_ARMOR[zone]);
    enemyDefenseId = armorDistribution(generator);
}

void General::ScaleMaxTacticForZone(Generator& generator)
{ 
    const BYTE MAX_TACTIC[9] = {50, 10, 15, 20, 25, 30, 35, 40, 50};

    uniform_int_distribution<BYTE> tacticDistribution(0, MAX_TACTIC[zone]);
    enemyMaxTacticLevel = tacticDistribution(generator);
}

void General::ScaleSoldiersForZone(Generator& generator)
{ 
    const BYTE MIN_SOLDIERS[9] = {1,   10,  24, 33, 48, 53, 64, 82,  99};
    const BYTE MAX_SOLDIERS[9] = {101, 27,  32, 47, 61, 60, 79, 111, 121};

    uniform_int_distribution<BYTE> soldierDistribution(MIN_SOLDIERS[zone], MAX_SOLDIERS[zone]);
    BYTE randomizedSoldiers = soldierDistribution(generator);
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
}

void General::ReadName(ROM& rom)
{
    const int NAME_OFFSET = 20;
    int nameOffset = 0;
    while (rom.ReadByte(address + nameOffset + NAME_OFFSET) != 0xFF)
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
    printf("\tzone: %d\n", zone);
    printf("\tHP exp: %d, %d\n", enemySoldierExponent, allySoldierExponent);
    printf("\tDefense: %d\n", enemyDefenseId);
    printf("\tMax Tactic Level: %d\n", enemyMaxTacticLevel);
    printf("\tWeapon: 0x%02X\n", weaponId);
}

}