#ifndef __GENERALS_H__
#define __GENERALS_H__

#include "Rom.h"
#include "General.h"

#include <vector>

namespace DoaERando {

class Generals
{
    public:
        Generals(ROM& rom);

        int GetGeneralAddress(BYTE id);
        void SwapIDs(int slot1, int slot2);
        void SwapZones(int slot1, int slot2);
        void AdjustZone1Generals();
        void SetZonesForZone0(Generator& generator);
        void ScaleForZone(Generator& generator);
        void ScaleSpecialGenerals(Generator& generator);
        void UpdateGenerals();

        const General& GetGeneralById(BYTE id);

        int size() {return myGenerals.size();}

    private:
        int GetGeneralBankAddress(BYTE id);
        int ConvertRomAddressToBankAddress(int romAddress);
        int GetGeneralIndexById(BYTE id);
        void ReadGenerals();
        void AddGeneral(BYTE id);
        void AddCopy(BYTE id);
        bool GeneralExists(BYTE id);
        bool isRestrictedGeneralAddress(int address);

        std::vector<General> myGenerals;
        std::vector<int> myRestrictedGeneralAddresses;
        ROM& myRom;
};

}

#endif
