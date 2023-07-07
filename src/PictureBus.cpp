//
// Created by lai leon on 7/7/2023.
//

#include <PictureBus.h>

PictureBus::PictureBus() {

}

Byte PictureBus::Read(Address addr) {
    return 0;
}

void PictureBus::Write(Address addr, Byte value) {

}

void PictureBus::UpdateMirroring() {

}

bool PictureBus::SetMapper(Mapper *mapper) {
    return false;
}

Byte PictureBus::ReadPalette(Byte paletteAddr) {
    return 0;
}
