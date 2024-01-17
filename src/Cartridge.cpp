//
// Created by lai leon on 27/6/2023.
//

#include <fstream>
#include <Cartridge.h>
#include <Log.h>

//.NES 文件头格式 https://wiki.nesdev.com/w/index.php/INES
//0-3: Constant $4E $45 $53 $1A ("NES" followed by MS-DOS end-of-file)
//4: Size of PRG ROM in 16 KB units
//5: Size of CHR ROM in 8 KB units (Value 0 means the board uses CHR RAM)
//6: Flags 6 - Mapper, mirroring, battery, trainer
//7: Flags 7 - Mapper, VS/Playchoice, NES 2.0
//8: Flags 8 - PRG-RAM size (rarely used extension)
//9: Flags 9 - TV system (rarely used extension)
//10: Flags 10 - TV system, PRG-RAM presence (unofficial, rarely used extension)
//11-15: Unused padding (should be filled with zero, but some rippers put their name across bytes 7-15)
//
//Flag6的具体含义如下：
//76543210
//||||||||
//|||||||+- 镜像位: 0: 水平 1：垂直。 游戏是横版还是纵版
//||||||+-- 1: 卡带包含电池供电的RAM($6000-7FFF)或其他持久性存储介质
//|||||+--- 1: trainer 标志位，可不管
//||||+---- 1: 忽略镜像控制或上述的镜像位；而是提供四屏VRAM。由于 NES 的显存只有2kb, 只能支持2屏幕. 如果卡带自带了额外的显存就可以利用4屏幕了
//++++----- Mapper号的低四位
//
//        Flag7的具体含义如下：
//76543210
//||||||||
//|||||||+- VS Unisystem
//||||||+-- PlayChoice-10 (8KB of Hint Screen data stored after CHR data)
//||||++--- If equal to 2, flags 8-15 are in NES 2.0 format
//++++----- Mapper编号的高四位
//```
//
//Flags6的高四位记录了mapper number的低四位，Flags7的高四位记录了mapper number的高四位。

Cartridge::Cartridge() :
        m_nameTableMirroring(0),
        m_mapperNumber(0),
        m_extendedRAM(false) {

}


bool Cartridge::LoadFromFile(std::string path) {
    std::ifstream romFile(path, std::ios_base::binary | std::ios_base::in);
    if (!romFile) {
        LOG(Error) << "Could not open ROM file from path: " << path << std::endl;
        return false;
    }
    LOG(Info) << "Reading ROM from path: " << path << std::endl;

    // 读取.NES文件中的 header
    std::vector<Byte> header;
    header.resize(0x10);

    //通过调用 romFile.read() 函数，从打开的文件流 romFile 中读取 0x10 个字节（16个字节），并将其内容存储到 header 容器中。
    //reinterpret_cast<char *>(&header[0]) 这部分代码将 header[0] 的地址转换为 char* 类型的指针，因为 read() 函数需要接受 char* 类型的指针作为目标缓冲区。

    //reinterpret_cast 来进行指针类型之间的转换，因为 reinterpret_cast 是用于执行低级别的指针类型转换的。
    //static_cast：用于进行静态类型转换，可以在具有关联性的类型之间进行转换，如整数类型之间的转换，或者父类指针到子类指针的转换。但是，static_cast 并不适用于指针类型之间的转换。
    //const_cast：用于移除指针或引用的 const 或 volatile 限定符，可以进行常量性的转换。但是，在给定的代码中，这并不适用，因为我们需要执行指针类型的转换，而不是常量性的转换。

    if (!romFile.read(reinterpret_cast<char *>(&header[0]), 0x10)) {
        LOG(Error) << "Reading iNES header failed." << std::endl;
        return false;
    }

    //使用 header 中的前四个字节构造了一个 std::string 对象。通过指定起始位置 &header[0] 和结束位置 &header[4]，可以提取出一个子字符串
    //\x1A 是一个转义序列，表示十六进制值为 0x1A 的字符。在这个特定的上下文中，\x1A 表示一个 iNES 文件的魔术数字，用于标识该文件是一个有效的 iNES 图像文件。
    if (std::string{&header[0], &header[4]} != "NES\x1A") {
        LOG(Error) << "Not a valid iNES image. Magic number: "
                   << std::hex << header[0] << " "
                   << header[1] << " " << header[2] << " " << int(header[3]) << std::endl
                   << "Valid magic number : N E S 1a" << std::endl;
        return false;
    }

    LOG(Info) << "Reading header, it dictates: \n-*--*--*--*--*--*--*--*-\n";

    Byte banks = header[4];
    LOG(Info) << "16KB PRG-ROM Banks: " << +banks << std::endl;
    if (!banks) {
        LOG(Error) << "ROM has no PRG-ROM banks. Loading ROM failed." << std::endl;
        return false;
    }
    // vidio banks
    Byte vbanks = header[5];
    LOG(Info) << "8KB CHR-ROM Banks: " << +vbanks << std::endl;

    // nameTableMirroring
    m_nameTableMirroring = header[6] & 0xB;
    LOG(Info) << "Name Table Mirroring: " << +m_nameTableMirroring << std::endl;
    // mapper Number
    m_mapperNumber = ((header[6] >> 4) & 0xf) | (header[7] & 0xf0);
    LOG(Info) << "Mapper number #: " << +m_mapperNumber << std::endl;

    m_extendedRAM = header[6] & 0x2;
    LOG(Info) << "Extended (CPU) RAM: " << std::boolalpha << m_extendedRAM << std::endl;

    // 模拟器暂不支持 使用 Trainer 格式的 .NES 文件
    if (header[6] & 0x4) {
        LOG(Error) << "Trainer is not supported." << std::endl;
        return false;
    }

    if ((header[0xA] & 0x3) == 0x2 || (header[0xA] & 0x1)) {
        LOG(Error) << "PAL ROM not supported." << std::endl;
        return false;
    } else LOG(Info) << "ROM is NTSC compatible.\n";

    //PRG-ROM 16KB banks
    // 将0x4000(16KB) * banks 内容写到 m_PGR_ROM 中
    m_PRG_ROM.resize(0x4000 * banks);
    if (!romFile.read(reinterpret_cast<char *>(&m_PRG_ROM[0]), 0x4000 * banks)) {
        LOG(Error) << "Reading PRG-ROM from image file failed." << std::endl;
        return false;
    }
//    for (int i = 0; i < 20; i++) {
//        std::cout << std::hex << static_cast<int>(m_PRG_ROM[i]) << " ";
//    }
//    std::cout << "\n";
    //CHR-ROM 8KB banks
    if (vbanks) {
        m_CHR_ROM.resize(0x2000 * vbanks);
        if (!romFile.read(reinterpret_cast<char *>(&m_CHR_ROM[0]), 0x2000 * vbanks)) {
            LOG(Error) << "Reading CHR-ROM from image file failed." << std::endl;
            return false;
        }

    } else LOG(Info) << "Cartridge with CHR-RAM." << std::endl;
    LOG(Info) << "-*--*--*--*--*--*--*--*-\n" << std::endl;
    return true;
}

const std::vector<Byte> &Cartridge::GetROM() {
    return m_PRG_ROM;
}

const std::vector<Byte> &Cartridge::GetVROM() {
    return m_CHR_ROM;
}

bool Cartridge::HasExtendedRAM() {
    return m_extendedRAM;
}

Byte Cartridge::GetMapper() {
    return m_mapperNumber;
}

Byte Cartridge::GetNameTableMirroring() {
    return m_nameTableMirroring;
}
