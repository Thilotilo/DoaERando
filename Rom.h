#ifndef __ROM_H__
#define __ROM_H__

#include <stdint.h>
#include <vector>
#include <string>
#include <random>

namespace DoaERando {

typedef uint8_t BYTE;

class ROM
{
public:
    void ReadRom(std::string file);
    void WriteRom(std::string path, int version, int seed);

    BYTE ReadByte(int address);
    BYTE ReadByteAtBank(int bank, int address);
    void WriteByte(int address, BYTE byte);
    void WriteByteAtBank(int bank, int address, BYTE byte);
    void SwapBytes(int address1, int address2);
    void SwapBytesAtBank(int bank, int address1, int address2);

    // Make some debugging easier
    void PrintByteRange(int firstByte, int lastByte);
private:
    std::vector<BYTE> myContents;
};

}

#endif
