//
// Created by lai leon on 9/6/2023.
//
#include "../include/CPU.h"
#include "../include/CPUOpcodes.h"
#include "../include/MainBus.h"

/* CPU 6502 */
CPU::CPU(MainBus &mem) : m_bus(mem) {}

Address CPU::ReadAddress(Address addr) {
//    将两个字节的数据合并成一个 16 位的值，其中高字节位于高位，低字节位于低位
    return m_bus.Read(addr) | m_bus.Read(addr + 1) << 8;
}

void CPU::Reset() {
    Reset(ReadAddress(ResetVector));
}

void CPU::Reset(Address start_addr) {
    m_skipCycles = m_cycles = 0;
    r_A = r_X = r_Y = 0;
    f_I = true;
    f_C = f_D = f_N = f_V = f_Z = false;
    r_PC = start_addr;
    /* Sp start at 0xfd */
    r_SP = 0xfd; //documented startup state
}

void CPU::Step() {
    // Byte opcode = m_bus.Read(r_PC++);
    // 待实现解码执行
}

void CPU::PushStack(Byte val) {
    m_bus.Write(0x100 | r_SP, val);
    --r_SP;
}

Byte CPU::PullStack() {
    //由 0x100 的高字节和 ++r_SP 的低字节组成
    //0x100 即二进制的 00000001 00000000
    return m_bus.Read(0x100 | ++r_SP);
}
