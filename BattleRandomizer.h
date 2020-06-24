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
        void FillBattlesRandomly(std::vector<BYTE> generalIds, std::vector<int> battleIds, RNG& rng);
        void FillBattleWithOnlyGenerals(std::vector<BYTE> generalIds, int battleId, RNG& rng);
        void RandomizeZone1(Generals& generals, RNG& rng);
        void RandomizeZone2(Generals& generals, RNG& rng);
        void RandomizeZone3(Generals& generals, RNG& rng);
        void RandomizeZone4(Generals& generals, RNG& rng);
        void RandomizeZone5(Generals& generals, RNG& rng);
        void RandomizeZone6(Generals& generals, RNG& rng);
        void RandomizeZone7(Generals& generals, RNG& rng);
        void RandomizeZone8(Generals& generals, RNG& rng);
        void UpdateBattles(ROM& rom);
        
    private:
        std::vector<Battle> myBattles;
};

}
