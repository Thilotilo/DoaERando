#ifndef __RNG_H__
#define __RNG_H__


#include "Rom.h" // For the BYTE typedef
#include <random>
#include <vector>

namespace DoaERando {


class RandomNumberGenerator
{
public:
    RandomNumberGenerator(int seed);

    BYTE GetRandomByte(BYTE lowValue, BYTE highValue);
    int GetRandomInt(int lowValue, int highValue);

    int GetRandomIndexFromByteVector(std::vector<BYTE>& values);
    BYTE GetRandomValueFromByteVector(std::vector<BYTE>& values);

private:
    RandomNumberGenerator() = delete;
    std::mt19937_64 myGenerator;
};

typedef RandomNumberGenerator RNG;

}

#endif
