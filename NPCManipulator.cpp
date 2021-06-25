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
    // This is a temporary fix for extra long names before
    // we overhaul the text stuff.
    // It rewrites (so)n to s(on) so that it can create a
    // newline before the general name.
    myRom.WriteByte(0x29639, 0x2E);
    myRom.WriteByte(0x2963A, 0xE0);
    myRom.WriteByte(0x2963B, 0xEC);
    myRom.WriteByte(0x2963C, 0xD2);
}

void NPCManipulator::ReplaceYangJin(BYTE generalId)
{
    myRom.WriteByte(0x319FA, generalId);
    // This replaces the text in the general's introduction
    myRom.WriteByte(0x2A354, generalId);
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
    // Update text for receiving the Zhou Letr
    myRom.WriteByte(0x2DE5A, generalId);
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

void NPCManipulator::ReplacePangTong(BYTE generalId)
{
    // Set the flags for the new general after the "Pang Tong went out
    // with his soldiers" Text
    myRom.WriteByte(0x36EAF, generalId);
    // Read the new general's recruitment flags, which are used to determine
    // whether to launch the battle.
    myRom.WriteByte(0x3B3C5, generalId);
    // Actual replacement for who joined our army after the battle
    myRom.WriteByte(0x36ECC, generalId);
    // Text replace for Pang Tong went out with his soldiers
    myRom.WriteByte(0x2EA09, generalId);
    // Text replace for Pang Tong joined our army
    myRom.WriteByte(0x2EA62, generalId);
}

void NPCManipulator::ReplaceZhugeLiang(BYTE generalId)
{
    // Replace 2 versions of Zhuge Liang in his house.
    // This is the lying down version
    myRom.WriteByte(0x312C5, generalId);
    // This is the standing up version
    myRom.WriteByte(0x312D3, generalId);
    // Replace Zhuge Liang in Gui Yang
    myRom.WriteByte(0x31343, generalId);
    // Text replace for "X is sleeping"
    myRom.WriteByte(0x2B9B2, generalId);
    // Text replace for "My brother X is not home"
    myRom.WriteByte(0x2B8F7, generalId);
    // Text replace for "X has joined them!"
    myRom.WriteByte(0x2BAE5, generalId);
}

void NPCManipulator::PlaceGeneralInQingZhouCave(BYTE generalId)
{
    // This replaces the 1st general in the NPC data
    // for the Qing Zhou cave
    myRom.WriteByte(0x3118E, generalId);
}

void NPCManipulator::PlaceGeneralInLiuKuiTent(BYTE generalId)
{
    // This replaces the 1st general in the NPC data
    // for the Qing Zhou cave
    myRom.WriteByte(0x31426, generalId);
}

void NPCManipulator::ReplaceHanZhongBridgeTrigger(BYTE generalId)
{
    // Replace the general that the brigand looks for
    myRom.WriteByte(0x363DB, generalId);
    // Replace the general in the NPC text identifying the trigger general
    myRom.WriteByte(0x2BEE2, generalId);
    // Replace "Isn't that bridge working yet" text
    myRom.WriteByte(0x2D6BD, generalId);
}

}
