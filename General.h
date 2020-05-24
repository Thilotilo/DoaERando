#ifndef __GENERAL_H__
#define __GENERAL_H__

#include "Rom.h"

namespace DoaERando {

class General
{
    public:
        int address;
        BYTE id;
        BYTE count;
        BYTE strength;
        BYTE intelligence;
        BYTE zone;
        BYTE weaponId;
        BYTE enemyDefenseId;
        BYTE enemySoldierExponent;
        BYTE allySoldierExponent;
        BYTE enemyMaxTacticLevel;
        BYTE agility;
        char name[20];
        bool encounterable;
        bool recruitable;

    General(int address, BYTE id, ROM& rom);

    void ReadName(ROM& rom);
    void ScaleWeaponForZone(Generator& generator);
    void ScaleArmorForZone(Generator& generator);
    void ScaleMaxTacticForZone(Generator& generator);
    void ScaleSoldiersForZone(Generator& generator);
    void UpdateGeneral(ROM& rom);

    void DumpGeneral();

    static bool IsEncounterable(BYTE id, ROM& rom);
    static void WriteEncounterableBit(BYTE id, bool isEncounterable, ROM& rom);
    static bool IsRecruitable(BYTE id, ROM& rom);
    static void WriteRecruitableBit(BYTE id, bool isRecruitable, ROM& rom);
};

}

#endif
