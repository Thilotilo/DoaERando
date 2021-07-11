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
    // This is the NPC data
    myRom.WriteByte(0x31772, generalId1);
    myRom.WriteByte(0x31779, generalId2);
    // This is for the post-battle flags change
    myRom.WriteByte(0x3B062, generalId1);
    myRom.WriteByte(0x3B065, generalId2);
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

void NPCManipulator::ReplaceXuZheBridgeTrigger(BYTE generalId)
{
    // Replace the general that the bridge lady looks for
    myRom.WriteByte(0x3684F, generalId);

    // Xu Zhe, my love, where are you?
    myRom.WriteByte(0x2AF53, generalId); // Replace Id
    myRom.WriteByte(0x2AF55, 0xE0); // newline in case of long name

    // Xu Zhe!  Oh, I'm blocking the bridge...
    myRom.WriteByte(0x2AF65, generalId); // Replace ID
    // Adjust text to protect against long names.
    myRom.WriteByte(0x2AF67, 0xFB);
    myRom.WriteByte(0x2AF68, 0x18);
    myRom.WriteByte(0x2AF69, 0x0A);
    myRom.WriteByte(0x2AF6A, 0x30);
    myRom.WriteByte(0x2AF6B, 0x3C);
    myRom.WriteByte(0x2AF6C, 0x8E);
}

void NPCManipulator::ReplaceMaYuanyi(BYTE generalId)
{
    // Replace the actual NPC
    myRom.WriteByte(0x31A73, generalId);
    // This requires a bit of explanation.  There are 2 bits that can tell the NPC to
    // use text 2 instead of text 1.  One of those bits is set when the yellow scarves
    // are defeated, the other is set when Ma Yuanyi is defeated.  It doesn't matter
    // if one or both are set - as long as both aren't clear, we will move to text 2.
    // EXCEPT we can actually move one id past text 2 if both bits are set AND bit 6 of
    // the YZone is set.  Thus, we swizzle this a bit so that both of Ma Yuanyi's texts
    // are the battle text (the textId + 1 is the text we get on future interactions), and
    // then set both bits when the battle is complete.  This way, we can't lock the battle
    // out when we clear the yellow scarves.  (Note that this will go ahead and change the
    // other NPCs in Qing Zhou texts, but those aren't really relevant any more anyway)
    myRom.WriteByte(0x31A6E, 0xCB); // Set bit 6 of YZone
    myRom.WriteByte(0x31A72, 0xAB); // Set text 2 to the battle text
    myRom.WriteByte(0x3B076, 0x83); // Set bit 7 & bit 0 of the location flags
}

void NPCManipulator::ReplaceYuanShang(BYTE generalId)
{
    // Replace the actual NPC
    myRom.WriteByte(0x317C3, generalId);
    myRom.WriteByte(0x3695A, generalId);
    // Set the correct general's flags to 0x90 (recruitable & encounterable)
    // rather than setting Yuan Shang's flags to 00 after defeating the
    // Yuan Shang battle.
    myRom.WriteByte(0x3693A, 0x90);
    myRom.WriteByte(0x3693C, generalId);
}

void NPCManipulator::ReplaceLuMeng(BYTE generalId)
{
    // Lu Meng is nice because he doesn't have any special logic to deal with,
    // since his appearance is entirely based on story flags rather than his
    // flags.  However, he appears in 3 different places, so we need to update
    // his NPC data in each one.
    myRom.WriteByte(0x315A9, generalId); // Wu
    myRom.WriteByte(0x3156E, generalId); // Po Yang
    myRom.WriteByte(0x31541, generalId); // Jin Du
}

void NPCManipulator::ReplacePirate(BYTE generalId)
{
    // Only need to replace the NPC here. No other logic.
    myRom.WriteByte(0x313EA, generalId);
}

void NPCManipulator::FixLongNameTexts()
{
    // I'm X. I am happy to serve
    // This fails for long names, so we put
    // X on the next line, and all is happy
    myRom.WriteByte(0x29684, 0xE0);
}

}
