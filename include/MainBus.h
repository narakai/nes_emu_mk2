//
// Created by lai leon on 9/6/2023.
//

#ifndef NES_EMU_MAINBUS_H
#define NES_EMU_MAINBUS_H

#include <Chip.h>
#include <Cartridge.h>
#include <Mapper.h>
#include <vector>

class MainBus {
public:
    MainBus();

//    MainBus(Cartridge &cartridge);

    Byte Read(Address addr);

    void Write(Address addr, Byte val);

    bool SetMapper(Mapper *mapper);

private:
    //内存信息
    std::vector<Byte> m_RAM;
    //扩展内存
    std::vector<Byte> m_extRAM;
//    Cartridge cartridge;
    Mapper *m_mapper;
};


#endif //NES_EMU_MAINBUS_H
