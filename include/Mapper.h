//
// Created by lai leon on 29/6/2023.
//

#ifndef NES_EMU_MAPPER_H
#define NES_EMU_MAPPER_H

#include <Cartridge.h>
#include <functional>

enum NameTableMirroring {
    Horizontal = 0,
    Vertical = 1,
    FourScreen = 8,
    OneScreenLower,
    OneScreenHigher,
};

//https://www.youtube.com/watch?v=xdzOvpYPmGE&list=PLrOv9FMX8xJHqMvSGB_9G9nZZ_4IgteYf&index=5
//当CPU执行指令或访问内存时，它会提供一个16位的地址。Mapper根据这个地址确定是要访问PRG还是CHR数据。
//根据映射表或逻辑规则，Mapper将地址转换为在ROM芯片中的实际地址，然后从相应的ROM芯片中读取数据并返回给CPU。

class Mapper {
public:
    enum Type {
        NROM = 0,
        SxROM = 1,
        UxROM = 2,
        CNROM = 3,
    };

    Mapper(Cartridge &cart, Type t) : m_cartridge(cart), m_type(t) {};

    /* 虚函数 */
    //PRG Program, CHR Pattern, PRG/CHR可能有多个Bank， CPU/PPU通过Mapper访问PRG和CHR数据
    virtual void WritePRG(Address addr, Byte value) = 0;

    virtual Byte ReadPRG(Address addr) = 0;

    virtual void WriteCHR(Address addr, Byte value) = 0;

    virtual Byte ReadCHR(Address addr) = 0;

    //默认有实现，非纯虚函数，不用 '= 0'
    virtual NameTableMirroring getNameTableMirroring();

    bool inline HasExtendedRAM() {
        return m_cartridge.HasExtendedRAM();
    }

    /*mirroring_cb 可选回调函数*/
    //std::unique_ptr<Mapper> 表示一个智能指针，用于管理 Mapper 对象的生命周期。
    //它提供了自动释放内存的功能，当该 std::unique_ptr 被销毁时，它会自动调用析构函数来销毁所管理的 Mapper 对象。这样可以方便地管理动态分配的 Mapper 对象，避免手动释放内存和内存泄漏的风险。

    //std::shared_ptr: std::shared_ptr 是一种共享所有权的智能指针。它可以被多个 std::shared_ptr 对象共享，
    //每个对象都维护一个引用计数来跟踪有多少个指针指向同一个对象。当最后一个引用计数变为零时，它会自动释放所管理的对象。std::shared_ptr 允许多个指针共享对象的所有权，适用于多个地方需要访问和共享同一个对象的情况。
    //std::weak_ptr: std::weak_ptr 也是一种共享所有权的智能指针，但它不会增加引用计数。
    //它通常与 std::shared_ptr 一起使用，用于解决循环引用（circular references）的问题。std::weak_ptr 允许观察一个对象的生命周期，但不能直接访问对象。

    static std::unique_ptr<Mapper>
    CreateMapper(Type t, Cartridge &cart, std::function<void(void)> mirroring_cb = nullptr);

    // 基类的虚构函数要定义为 virtual，不然会报警告
    // delete called on 'Mapper' that is abstract but has non-virtual destructor
    virtual ~Mapper() = default;


protected:
    Cartridge &m_cartridge;
    Type m_type;
};


#endif //NES_EMU_MAPPER_H
