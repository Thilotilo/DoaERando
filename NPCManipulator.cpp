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

void NPCManipulator::ReplaceYangJin(BYTE generalId)
{
    myRom.WriteByte(0x319FA, generalId);
}

void NPCManipulator::ReplaceWangGui(BYTE generalId)
{
    myRom.WriteByte(0x31976, generalId);
}

void NPCManipulator::ReplaceZhouChao(BYTE generalId)
{
    myRom.WriteByte(0x31909, generalId);
}

void NPCManipulator::ReplaceZhaoYun(BYTE generalId)
{
    // NPC Data
    myRom.WriteByte(0x313B7, generalId);
    // This is the check for the Zhou Letr
    myRom.WriteByte(0x38BBF, generalId);
}

void NPCManipulator::ReplaceXuZhe(BYTE generalId)
{
    myRom.WriteByte(0x312A3, generalId);
}

void NPCManipulator::ReplaceGuanPing(BYTE generalId)
{
    myRom.WriteByte(0x313C0, generalId);
}

void NPCManipulator::ReplaceLiuFeng(BYTE generalId)
{
    myRom.WriteByte(0x31184, generalId);
}

void NPCManipulator::ReplaceZhouCang(BYTE generalId)
{
    myRom.WriteByte(0x312AA, generalId);
}

void NPCManipulator::ReplaceHuoHu(BYTE generalId)
{
    myRom.WriteByte(0x319B8, generalId);
}

void NPCManipulator::ReplaceMaSu(BYTE generalId)
{
    myRom.WriteByte(0x312E4, generalId);
}

void NPCManipulator::ReplaceMaLiang(BYTE generalId)
{
    myRom.WriteByte(0x312DD, generalId);
}

void NPCManipulator::ReplaceHuangZhongAndWeiYan(BYTE generalId1, BYTE generalId2)
{
    myRom.WriteByte(0x31772, generalId1);
    myRom.WriteByte(0x31779, generalId2);
}

void NPCManipulator::ReplaceZhangBao(BYTE generalId)
{
    myRom.WriteByte(0x31351, generalId);
}

void NPCManipulator::ReplaceGuanXing(BYTE generalId)
{
    myRom.WriteByte(0x312FF, generalId);
}

void NPCManipulator::ReplaceJiangWei(BYTE generalId)
{
    myRom.WriteByte(0x314EB, generalId);
}

void NPCManipulator::ReplaceMaChaoAndMaDai(BYTE generalId1, BYTE generalId2)
{
    // These bytes determine the IDs of the 2 generals to join
    myRom.WriteByte(0x371D6, generalId1);
    myRom.WriteByte(0x371E4, generalId2);
}

}
