#include "NPCManipulator.h"

namespace DoaERando {

void NPCManipulator::ReplaceSongYongAndSongRen(BYTE generalId1, BYTE generalId2)
{
    // This sets the NPC Data.
    myRom.WriteByte(0x31AC9, generalId1);
    myRom.WriteByte(0x31AD0, generalId2);
    // This sets the text strings.
    myRom.WriteByte(0x36207, generalId1);
    myRom.WriteByte(0x3620B, generalId2);
}

void NPCManipulator::ReplaceMiZhu(BYTE generalId)
{
    myRom.WriteByte(0x311B8, generalId);
}

void NPCManipulator::ReplaceChenDeng(BYTE generalId)
{
    myRom.WriteByte(0x311C0, generalId);
    // This replaces the name given by Chen Deng's father
    myRom.WriteByte(0x2963D, generalId);
}

}
