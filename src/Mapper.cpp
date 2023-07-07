//
// Created by lai leon on 29/6/2023.
//

#include <Mapper.h>
#include <MapperNROM.h>

NameTableMirroring Mapper::getNameTableMirroring() {
    return static_cast<NameTableMirroring>(m_cartridge.GetNameTableMirroring());
}

std::unique_ptr<Mapper> Mapper::CreateMapper(Mapper::Type t, Cartridge &cart, std::function<void(void)> mirroring_cb) {
    std::unique_ptr<Mapper> ret(nullptr);
    switch (t) {
        case NROM:
            ret.reset(new MapperNROM(cart));
            break;
        case SxROM:
            // to do
            break;
        case UxROM:
            // to do
            break;
        case CNROM:
            // to do
            break;
    }
    return ret;
}
