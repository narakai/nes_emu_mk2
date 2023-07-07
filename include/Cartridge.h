//
// Created by lai leon on 27/6/2023.
//

#ifndef NES_EMU_CARTRIDGE_H
#define NES_EMU_CARTRIDGE_H

/*
 * cartridge: 卡带
 * 卡带中 NES 文件格式：
 * |Header|Trainer|PGR|CHR|
 * |16B   |0或者512B |n*16KB|n*8KB|
 * Header 中 Trainer 的 flag 为 1，则此区域为 512 字节，否则为 0。
 * 模拟可不考虑 Trainer 段
 *
*/
#include <cstdint>
#include <vector>

using Byte = std::uint8_t;
using Address = std::uint16_t;

class Cartridge {
public:
    Cartridge();

    bool LoadFromFile(std::string path);

    const std::vector<Byte> &GetROM();

    const std::vector<Byte> &GetVROM();

    bool HasExtendedRAM();

    Byte GetMapper();

    Byte GetNameTableMirroring();

private:
    std::vector<Byte> m_PRG_ROM; // PRG (Program)：PRG是指程序存储区域，它存放着游戏的程序代码和执行逻辑。
    std::vector<Byte> m_CHR_ROM; // CHR (Character)：CHR是指图像数据存储区域，它存放着游戏的图像和图形资源。

    Byte m_nameTableMirroring;
    Byte m_mapperNumber;            // mapper号 最基础为0
    bool m_extendedRAM;             // 卡带中是否存在扩展RAM
};


#endif //NES_EMU_CARTRIDGE_H
