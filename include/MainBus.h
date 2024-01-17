//
// Created by lai leon on 9/6/2023.
//

#ifndef NES_EMU_MAINBUS_H
#define NES_EMU_MAINBUS_H

#include <Chip.h>
#include <Cartridge.h>
#include <Mapper.h>
#include <vector>
#include <map>

/*
 * IO寄存器的具体含义参考 http://fms.komkon.org/EMUL8/NES.html#LABF
*/

enum IORegisters {
    PPUCTRL = 0x2000,  // 控制寄存器
    PPUMASK,
    PPUSTATUS,         // 状态寄存器
    OAMADDR,           // Sprite 内存地址寄存器
    OAMDATA,           // Sprite Memory Data
    PPUSCROL,          //  Screen Scroll Offsets 屏幕滚动偏移
    PPUADDR,           // PPU Memory Address
    PPUDATA,           // PPU Memory Data
    // 中间空余的为 Sound 寄存器
    OAMDMA = 0x4014,   // DMA Access to the Sprite Memory
    JOY1 = 0x4016,     // Joystick1 + Strobe
    JOY2 = 0x4017,     // Joystick2 + Strobe
};


class MainBus {
public:
    MainBus();

//    MainBus(Cartridge &cartridge);

    Byte Read(Address addr);

    void Write(Address addr, Byte val);

    bool SetMapper(Mapper *mapper);

    const Byte *GetPagePtr(Byte page);

    // IO 寄存器读写
    // 为什么不直接采用普通内存读写的实现？
    // 提供一个抽象接口，交由对应的硬件模块提供函数获得相应的寄存器值
    bool SetWriteCallback(IORegisters reg, std::function<void(Byte)> callback);

    bool SetReadCallback(IORegisters reg, std::function<Byte(void)> callback);

private:
    //内存信息
    std::vector<Byte> m_RAM;
    //扩展内存
    std::vector<Byte> m_extRAM;
//    Cartridge cartridge;
    Mapper *m_mapper;
    std::map<IORegisters, std::function<void(Byte)>> m_writeCallbacks;
    std::map<IORegisters, std::function<Byte(void)>> m_readCallbacks;
};


#endif //NES_EMU_MAINBUS_H
