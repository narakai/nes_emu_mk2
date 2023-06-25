//
// Created by lai leon on 9/6/2023.
//

#ifndef NES_EMU_CPU_H
#define NES_EMU_CPU_H

#include "MainBus.h"

//Program Counter
//
//16 bit，程序计数器 PC，存放下一条指令的地址，一条指令执行时就会更新这个寄存器的值，使它指向下一条指令的地址，与我们熟悉的 PC 一样，可以被分支指令修改等等，不再多说。
//
//Stack Pointer
//
//栈指针 SP，6502 架构下的栈也是上下颠倒向下扩展的，即 push 一个元素 SP 减小，POP 一个元素 SP 增加。
//
//这个栈指针并不是直接指向栈内存的地址，而是一个最大值为 \$FF 的偏移量。6502 的栈没有溢出检测，栈指针的值就是从 $00 到 $FF 之间回绕(wrap around)，意思就是说 当前值为 $FF 时再往下移时就变成了 $00

//Stack Pointer 总是指向栈的顶部，随着数据的推入和弹出，SP 的值会动态地改变，以指示栈顶的位置。

class CPU {
public:
    CPU(MainBus &mem);

    void Reset();

    void Reset(Address start_address);

    void Step();

    Address GetPC() { return r_PC; }

private:
    MainBus &m_bus;

    /* 内存addr处保存着一个地址信息 */
    Address ReadAddress(Address addr);

    /* 模拟指令执行 */
    bool ExecuteImplied(Byte opcode);

    bool ExecuteBranch(Byte opcode);

    bool ExecuteType0(Byte opcode);

    bool ExecuteType1(Byte opcode);

    bool ExecuteType2(Byte opcode);

    void SetPageCrossed(Address a, Address b, int inc = 1);

    /* CPU exec instruction*/
    //* 从内存的栈中读/写数据
    void PushStack(Byte value);

    Byte PullStack();

    void SetZN(Byte value);

    //clock
    int m_skipCycles;
    int m_cycles;

    //Registers
    Address r_PC;    /* Program Counter */ /*存储下一条将要执行的指令的内存地址*/
    Byte r_SP;       /* Stack Pointer */ /*保存寄存器的值、局部变量以及其他需要临时存储的数据*/
    Byte r_A;        /* Accumulator_ */  /*执行算术和逻辑操作*/
    Byte r_X;        /* index Reg X */ /*数据移动、索引计算和循环等操作*/
    Byte r_Y;        /* index Reg Y */ /*数据移动、索引计算和循环等操作*/

    /* Processor Status Reg */
    //通过检查和设置状态寄存器的不同标志位，程序可以根据运算结果进行条件判断、流程控制和错误处理

    /// # Status Register (P) http://wiki.nesdev.com/w/index.php/Status_flags
///
///  7 6 5 4 3 2 1 0
///  N V _ B D I Z C
///  | |   | | | | +--- Carry Flag
///  | |   | | | +----- Zero Flag
///  | |   | | +------- Interrupt Disable
///  | |   | +--------- Decimal Mode (not used on NES)
///  | |   +----------- Break Command
///  | +--------------- Overflow Flag
///  +----------------- Negative Flag
///

    bool f_C;         /* Carry */
    bool f_Z;         /* Zero */
    bool f_I;         /* IRQ disable */
    bool f_B;         /* Brk command */
    bool f_D;         /* Decimal mode */
    bool f_V;         /* Overflow  */
    bool f_N;         /* Negative */
};

#endif //NES_EMU_CPU_H
