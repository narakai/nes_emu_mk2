//
// Created by lai leon on 29/6/2023.
//

#ifndef NES_EMU_MAPPERNROM_H
#define NES_EMU_MAPPERNROM_H


#include <Mapper.h>

class MapperNROM : public Mapper {
public:
    MapperNROM(Cartridge &cart);

    void WritePRG(Address addr, Byte value);

    Byte ReadPRG(Address addr);

    Byte ReadCHR(Address addr);

    void WriteCHR(Address addr, Byte value);

private:
    bool m_oneBank;
    bool m_usesCharacterRAM;

    std::vector<Byte> m_characterRAM;
};


#endif //NES_EMU_MAPPERNROM_H
