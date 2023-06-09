//
// Created by lai leon on 9/6/2023.
//

#ifndef NES_EMU_CPU_H
#define NES_EMU_CPU_H

#include "MainBus.h"

class CPU {
public:
    CPU(MainBus &mem);

    void Reset();

    void Reset(Address start_address);

    void Step();

    Address GetPC() { return r_PC; }

private:
    MainBus &m_bus;

    Address ReadAddress(Address addr);

    /* CPU exec instruction*/
    void PushStack(Byte value);

    Byte PullStack();

    //clock
    int m_skipCycles;
    int m_cycles;

    //Registers
    //PC
    Address r_PC;
    Byte r_SP;       /* Stack Pointer */
    Byte r_A;        /* Accumulator */
    Byte r_X;        /* index Reg X */
    Byte r_Y;        /* index Reg Y */

    /* Processor Status Reg */
    bool f_C;         /* Carry */
    bool f_Z;         /* Zero */
    bool f_I;         /* IRQ disable */
    bool f_B;         /* Brk command */
    bool f_D;         /* Decimal mode */
    bool f_V;         /* Overflow  */
    bool f_N;         /* Negative */
};

#endif //NES_EMU_CPU_H
