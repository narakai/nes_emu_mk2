//
// Created by lai leon on 9/6/2023.
//
#include "../include/MainBus.h"

/*  0x800 = 2KB */
// memory 2KB
MainBus::MainBus() : m_RAM(0x800, 0) {

}

Byte MainBus::Read(Address addr) {
    /* 0x2000 =  8KB RAM  */
    // 0x7ff 的二进制表示是 0000011111111111
    if (addr < 0x2000) {
        /* 实际只有2KB RAM(如上定义的0x800)，所以采用addr & 0x7ff的操作 */
        //将 addr 的高位清零，只保留低 11 位的值。因为这段代码实际上是模拟了一个只有 2KB（0x800）大小的 RAM，
        // 通过 addr & 0x7ff 可以确保地址范围在 0 到 2047 之间
        return m_RAM[addr & 0x7ff];
    }

    return 0;
}

void MainBus::Write(Address addr, Byte val) {
    if (addr < 0x2000) {
        m_RAM[addr & 0x7ff] = val;
    }
}

