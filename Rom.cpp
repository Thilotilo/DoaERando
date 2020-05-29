#include "Rom.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

namespace DoaERando {

void ROM::ReadRom(std::string file)
{
    ifstream fin(file, ios::binary);
    if (fin.fail()) throw 0;
    myContents = vector<BYTE>(
        (istreambuf_iterator<char>(fin)),
        (istreambuf_iterator<char>())
        );
}

void ROM::WriteRom(std::string path, int version, int seed)
{
    ostringstream oss;
    oss << path << "/DoaERando_" << version << "_" << seed << ".nes";
    cout << "Writing to file " << oss.str() << endl;
    ofstream fout(oss.str(), ios::out | ios::binary);
    if (fout.fail()) throw 0;
    fout.write((char*)myContents.data(), myContents.size());
    fout.close();
}

BYTE ROM::ReadByte(int address)
{
    return myContents[address];
}

BYTE ROM::ReadByteAtBank(int bank, int address)
{
    int baseOffset = address - 0x8000;
    return ReadByte(0x10 + (bank * 0x4000) + baseOffset);
}

void ROM::WriteByte(int address, BYTE byte)
{
    myContents[address] = byte;
}

void ROM::WriteByteAtBank(int bank, int address, BYTE byte)
{
    int baseOffset = address - 0x8000;
    WriteByte(0x10 + (bank * 0x4000) + baseOffset, byte);
}

void ROM::SwapBytes(int address1, int address2)
{
    BYTE temp = myContents[address1];
    myContents[address1] = myContents[address2];
    myContents[address2] = temp;
}

void ROM::SwapBytesAtBank(int bank, int address1, int address2)
{
    int baseOffset1 = address1 - 0x8000;
    int baseOffset2 = address2 - 0x8000;
    SwapBytes(0x10 + (bank * 0x4000) + baseOffset1,
              0x10 + (bank * 0x4000) + baseOffset2);

}

void ROM::PrintByteRange(int firstByte, int lastByte)
{
    for (int i = firstByte; i <= lastByte; ++i)
    {
        printf("0x%05X: 0x%02X\n", i, myContents[i]);
    }
}

}

