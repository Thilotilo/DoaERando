#ifndef __NPCMANIPULATOR_H__
#define __NPCMANIPULATOR_H__

#include "Rom.h"
#include <vector>

namespace DoaERando {

// Wrap these in a class just to prevent passing a ROM to each parameter
class NPCManipulator
{

public:
    NPCManipulator(ROM& rom) : myRom(rom) {}

    // Listing out all NPC manipulation methods, making them empty.
    // As they get filled, the implementation will move to .cpp, but I want
    // a reference list here.
    void ReplaceSongYongAndSongRen(BYTE generalId1, BYTE generalId2);
    void ReplaceMiZhu(BYTE generalId);
    void ReplaceChenDeng(BYTE generalId);
    void ReplaceYangJin(BYTE generalId);
    void ReplaceWangGui(BYTE generalId);
    void ReplaceZhouChao(BYTE generalId);
    void ReplaceZhaoYun(BYTE generalId);
    void ReplaceXuZhe(BYTE generalId);
    void ReplaceGuanPing(BYTE generalId);
    void ReplaceLiuFeng(BYTE generalId);
    void ReplaceZhouCang(BYTE generalId);
    void ReplaceHuoHu(BYTE generalId);
    void ReplaceMaSu(BYTE generalId);
    void ReplaceMaLiang(BYTE generalId);
    void ReplaceHuangZhongAndWeiYan(BYTE generalId1, BYTE generalId2);

    void ReplaceXuZheBridgeTrigger(BYTE generalId)
    {
    }


    void ReplaceMaChaoAndMaDai(BYTE generalId1, BYTE generalId2);
    void ReplaceZhangBao(BYTE generalId);
    void ReplaceGuanXing(BYTE generalId);
    void ReplaceJiangWei(BYTE generalId);
    void ReplacePangTong(BYTE generalId);
    void ReplaceZhugeLiang(BYTE generalId);

    // NOTE: This will replace the id of the first general in the cave's
    // NPC Data.  If the cave is reconfigured, after this is called, then
    // the replacment will be overwritten.
    void PlaceGeneralInQingZhouCave(BYTE generalId);
    void ReplaceHanZhongBridgeTrigger(BYTE generalId);

private:

    ROM& myRom;

};

}

#endif
