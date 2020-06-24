#include "RNG.h"

using namespace std;

namespace DoaERando {

RandomNumberGenerator::RandomNumberGenerator(int seed)
    : myGenerator(seed)
{
}

BYTE RandomNumberGenerator::GetRandomByte(BYTE lowValue, BYTE highValue)
{
    uniform_int_distribution<BYTE> distribution(lowValue, highValue);
    return distribution(myGenerator);
}

int RandomNumberGenerator::GetRandomInt(int lowValue, int highValue)
{
    uniform_int_distribution<int> distribution(lowValue, highValue);
    return distribution(myGenerator);
}

int RandomNumberGenerator::GetRandomIndexFromByteVector(std::vector<BYTE>& values)
{
    return GetRandomInt(0, values.size() - 1);
}

BYTE RandomNumberGenerator::GetRandomValueFromByteVector(std::vector<BYTE>& values)
{
    return values[GetRandomIndexFromByteVector(values)];
}

}
