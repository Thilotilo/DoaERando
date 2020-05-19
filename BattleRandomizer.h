#include "Rom.h"
#include "Generals.h"
#include <vector>

namespace DoaERando {

class Battle
{
    public:
        Battle(int id);
        void OrganizeBattle();

        int zone;
        int battleId;
        BYTE generals[5];
        int slotsRemaining;
        bool battleModified;
};

class BattleRandomizer
{
    public:
        BattleRandomizer();
        bool PlaceGeneralInBattle(BYTE id, int battleId);
        void FillBattlesRandomly(std::vector<BYTE> generalIds, std::vector<int> battleIds, Generator& generator);
        void FillBattleWithOnlyGenerals(std::vector<BYTE> generalIds, int battleId, Generator& generator);
        void RandomizeZone1(Generals& generals, Generator& generator);
        void RandomizeZone2(Generals& generals, Generator& generator);
        void RandomizeZone3(Generals& generals, Generator& generator);
        void RandomizeZone4(Generals& generals, Generator& generator);
        void RandomizeZone5(Generals& generals, Generator& generator);
        void RandomizeZone6(Generals& generals, Generator& generator);
        void RandomizeZone7(Generals& generals, Generator& generator);
        void RandomizeZone8(Generals& generals, Generator& generator);
        void UpdateBattles(ROM& rom);
        
    private:
        std::vector<Battle> myBattles;
};

}
