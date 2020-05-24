#include "Generals.h"

namespace DoaERando {

const BYTE UV10_ID = 0xB3; // This is a random NPC in the midst of generals
const BYTE LIU_BEI_ID = 0xA8;
const BYTE CAO_PI_ID = 0x71;
const BYTE SIMA_YI_ID = 0x88;
const BYTE SUN_QUAN_ID = 0x57;

using namespace std;

Generals::Generals(ROM& rom)
    : myRom(rom)
    , myGenerals()
    , myRestrictedGeneralAddresses()
{
    ReadGenerals();
}

int Generals::GetGeneralAddress(BYTE id)
{
    const int BANK_OFFSET = 0x30010 - 0x8000;
    int address = GetGeneralBankAddress(id) + BANK_OFFSET;
    return address;
}

int Generals::GetGeneralBankAddress(BYTE id)
{
    const int GENERAL_ADDRESSES_LSB_START = 0x32610;
    const int GENERAL_ADDRESSES_MSB_START = 0x32710;
    int msb = myRom.ReadByte(GENERAL_ADDRESSES_MSB_START + id);
    int lsb = myRom.ReadByte(GENERAL_ADDRESSES_LSB_START + id);
    int address = (msb << 8) + lsb;
    return address;
}

int Generals::ConvertRomAddressToBankAddress(int romAddress)
{
    const int BANK_OFFSET = 0x30010 - 0x8000;
    return romAddress - BANK_OFFSET;
}

void Generals::ReadGenerals()
{
    for (BYTE i = 5; i < 0xD8; ++i)
    {
        if (i != UV10_ID)
        {
            AddGeneral(i);
            if (i == LIU_BEI_ID || i == CAO_PI_ID ||
                i == SIMA_YI_ID || i == SUN_QUAN_ID)
            {
                myRestrictedGeneralAddresses.push_back(GetGeneralAddress(i));
            }
        }
    }
}

void Generals::AddGeneral(BYTE id)
{
    if (GeneralExists(id))
    {
        AddCopy(id);
    }
    else
    {
        auto address = GetGeneralAddress(id);
        General general(address, id, myRom);
        myGenerals.push_back(general);
    }
}

void Generals::AddCopy(BYTE id)
{
    auto address = GetGeneralAddress(id);
    for (auto& general: myGenerals)
    {
        if (general.address == address)
        {
            general.count++;
        }
    }
}

bool Generals::GeneralExists(BYTE id)
{
    for (int i = 0; i < myGenerals.size(); ++i)
    {
        if(myGenerals[i].address == GetGeneralAddress(id))
        {
            return true;
        }
    }

    return false;
}

void Generals::AdjustZone1Generals()
{
    // These are generals that are not found in zone 1 but are marked as zone 1, making
    //  zone 1 over populated:

    const BYTE ZHOU_CHAO_ID = 0x05;
    const BYTE WANG_GUI_ID = 0x09;
    const BYTE SUN_GAN_ID = 0x9E;
    const BYTE HUO_HU_ID = 0xB8;
    const BYTE YANG_JIN_ID = 0xB9;
    // Zhou Chao is in Chang An - he belongs in chapter 3
    int zhouChaoIndex = GetGeneralIndexById(ZHOU_CHAO_ID);
    myGenerals[zhouChaoIndex].zone = 3;
    // Wang Gui is in Luo Yang - he belongs in chapter 2
    int wangGuiIndex = GetGeneralIndexById(WANG_GUI_ID);
    myGenerals[wangGuiIndex].zone = 2;
    // Sun Gan is not in the game - he belongs in chapter 0
    int sunGanIndex = GetGeneralIndexById(SUN_GAN_ID);
    myGenerals[sunGanIndex].zone = 0;
    // Huo Hu is in Lu Bu's fortress - he belongs in chapter 2
    int huoHuIndex = GetGeneralIndexById(HUO_HU_ID);
    myGenerals[huoHuIndex].zone = 2;
    // Yang Jin is in Hua Xiong's fortress - he belongs in chapter 2
    int yangJinIndex = GetGeneralIndexById(YANG_JIN_ID);
    myGenerals[yangJinIndex].zone = 2;
}

void Generals::SwapIDs(int slot1, int slot2)
{
    if (!isRestrictedGeneralAddress(myGenerals[slot1].address) &&
        !isRestrictedGeneralAddress(myGenerals[slot2].address))
    {
        auto tempId = myGenerals[slot1].id;
        auto tempCount = myGenerals[slot1].count;
        myGenerals[slot1].id = myGenerals[slot2].id;
        myGenerals[slot1].count = myGenerals[slot2].count;
        myGenerals[slot2].id = tempId;
        myGenerals[slot2].count = tempCount;
    }
}

void Generals::SwapZones(int slot1, int slot2)
{
    if (!isRestrictedGeneralAddress(myGenerals[slot1].address) &&
        !isRestrictedGeneralAddress(myGenerals[slot2].address))
    {
        auto tempZone = myGenerals[slot1].zone;
        myGenerals[slot1].zone = myGenerals[slot2].zone;
        myGenerals[slot2].zone = tempZone;
    }
}

void Generals::SetZonesForZone0(Generator& generator)
{
    for (auto& general : myGenerals)
    {
        if (general.zone == 0)
        {
            // Zone 1 already has too many generals, let's only allow zone 2 and above:
            uniform_int_distribution<BYTE> zoneDistribution(2,8);
            general.zone = zoneDistribution(generator);
        }
    }
}

void Generals::SetAllGeneralsEncounterable()
{
    for (auto& general : myGenerals)
    {
        general.encounterable = true;
    }
}

void Generals::SetAllGeneralsRecruitable()
{
    for (auto& general : myGenerals)
    {
        general.recruitable = true;
    }
}

bool Generals::isRestrictedGeneralAddress(int address)
{
    for (auto &i : myRestrictedGeneralAddresses)
    {
        if (i == address)
        {
            return true;
        }
    }
    return false;
}

bool Generals::isStartingGeneralAddress(int address)
{
    for (auto &i : myStartingGeneralAddresses)
    {
        if (i == address)
        {
            return true;
        }
    }
    return false;
}


void Generals::ScaleForZone(Generator& generator)
{
    for (auto& general : myGenerals)
    {
        if (general.id != CAO_PI_ID &&
            general.id != SIMA_YI_ID &&
            general.id != SUN_QUAN_ID)
        {
            general.ScaleWeaponForZone(generator);
            general.ScaleArmorForZone(generator);
            general.ScaleMaxTacticForZone(generator);
            general.ScaleSoldiersForZone(generator);
        }
    }
}

void Generals::UpdateGenerals()
{
    const int GENERAL_ADDRESSES_LSB_START = 0x32610;
    const int GENERAL_ADDRESSES_MSB_START = 0x32710;
    for (auto& general : myGenerals)
    {
        general.UpdateGeneral(myRom);
        general.DumpGeneral();

        for (int i = 0; i < general.count; ++i)
        {
            int bankAddress = ConvertRomAddressToBankAddress(general.address);
            BYTE lsb = bankAddress & 0xFF;
            BYTE msb = (bankAddress >> 8) & 0xFF;
            myRom.WriteByte(GENERAL_ADDRESSES_MSB_START + general.id + i, msb);
            myRom.WriteByte(GENERAL_ADDRESSES_LSB_START + general.id + i, lsb);
        }
    }
}

int Generals::GetGeneralIndexById(BYTE id)
{
    int i = 0;
    for (i = 0; i < myGenerals.size(); ++i)
    {
        for (int j = myGenerals[i].id; j < myGenerals[i].id + myGenerals[i].count; ++j)
        {
            if (j == id)
            {
                return i;
            }
        }
    }
    return -1;
}

void Generals::ScaleSpecialGenerals(Generator& generator)
{
    uniform_int_distribution<BYTE> tacticDistribution(35, 51);

    BYTE maxTacticLevel = tacticDistribution(generator);
    int index = GetGeneralIndexById(CAO_PI_ID);
    myGenerals[index].enemyMaxTacticLevel = maxTacticLevel;
    
    maxTacticLevel = tacticDistribution(generator);
    index = GetGeneralIndexById(SIMA_YI_ID);
    myGenerals[index].enemyMaxTacticLevel = maxTacticLevel;

    maxTacticLevel = tacticDistribution(generator);
    index = GetGeneralIndexById(SUN_QUAN_ID);
    myGenerals[index].enemyMaxTacticLevel = maxTacticLevel;

}

void Generals::SetStartingGenerals(vector<BYTE>& startingGeneralIds)
{
    for (auto& generalId : startingGeneralIds)
    {
        int index = GetGeneralIndexById(generalId);
        myStartingGeneralAddresses.push_back(myGenerals[index].address);
        myGenerals[index].encounterable = false;
    }
}

std::vector<BYTE> Generals::GetGeneralIdsFromZone(BYTE zone)
{
    std::vector<BYTE> generalIdsForZone;
    for (auto& general : myGenerals)
    {
        if (isStartingGeneralAddress(general.address) || isRestrictedGeneralAddress(general.address))
        {
            continue;
        }
        if (general.zone == zone)
        {
            // The game likes to use the last id if there are multiples
            generalIdsForZone.push_back(general.id + (general.count - 1));
        }
    }
    return generalIdsForZone;
}

}
