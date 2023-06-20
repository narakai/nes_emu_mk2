//
// Created by lai leon on 9/6/2023.
//
#include <ios>
#include "../include/CPU.h"
#include "../include/CPUOpcodes.h"
#include "../include/MainBus.h"
#include "../include/Log.h"

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

// & 取值操作，取mask为1的位
// ｜ 赋值操作，设mask为0的位

const auto CARRY = 0b00000001;
const auto ZERO = 0b00000010;
const auto INTERRUPT_DISABLE = 0b00000100;
const auto DECIMAL_MODE = 0b00001000;
const auto BREAK = 0b00010000;
const auto BREAK2 = 0b00100000;
const auto OVERFLOW1 = 0b01000000;
const auto NEGATIVE = 0b10000000;

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
    // 初始Stack Pointer位置, 6502栈是向下增长的
    r_SP = 0xfd; //documented startup state
}

void CPU::Step() {
    ++m_cycles;

    if (m_skipCycles-- > 1) {
        //m_skipCycles大于1直接减1返回
        return;
    }

    m_skipCycles = 0;

    Byte opcode = m_bus.Read(r_PC++);
    auto CycleLength = OperationCycles[opcode];
    if (CycleLength && (ExecuteImplied(opcode) || ExecuteBranch(opcode) ||
                        ExecuteType1(opcode) || ExecuteType2(opcode))) {
        m_skipCycles += CycleLength;
    } else {
        LOG(Error) << "Unrecognized opcode: " << std::hex << +opcode << std::endl;
    }
}

//r_SP 范围 00～ff 1111 1111
void CPU::PushStack(Byte val) {
    m_bus.Write(0x100 | --r_SP, val);
}

Byte CPU::PullStack() {
    //由 0x100 的高字节和 ++r_SP 的低字节组成
    //0x100 即二进制的 00000001 00000000
    Byte value = m_bus.Read(0x100 | r_SP);
    ++r_SP;
    return value;
}

bool CPU::ExecuteImplied(Byte opcode) {
    switch (static_cast<OperationImplied>(opcode)) {
        case NOP:
            //no operation
            break;
        case JSR:
            /* jump to new location Saving Return Address */
            //Push address of next instruction - 1, thus r_PC + 1 instead of r_PC + 2
            //since r_PC and r_PC + 1 are address of subroutine
            // 将PC+1的高低位写入SP中
            PushStack(static_cast<Byte>((r_PC + 1) >> 8));
            PushStack(static_cast<Byte>((r_PC + 1)));
            //Jump to New Location
            r_PC = ReadAddress(r_PC);
            break;
        case RTI:
            //Return from Interrupt
            //The status register is pulled with the break flag and bit 5 ignored.
            //Then PC is pulled from the stack.
        {
            Byte flags = PullStack();
            f_N = flags & NEGATIVE;
            f_V = flags & OVERFLOW1;
            f_D = flags & DECIMAL_MODE;
            f_I = flags & INTERRUPT_DISABLE;
            f_Z = flags & ZERO;
            f_C = flags & CARRY;
        }
            r_PC = PullStack();
            r_PC |= PullStack() << 8;
            break;
        case RTS:
            //Return from Subroutine
            r_PC = PullStack();
            r_PC |= PullStack() << 8;
            ++r_PC;
            break;
        case JMP:
            //Jump to New Location
            r_PC = ReadAddress(r_PC);
            break;
        case JMPI: {
            Address location = ReadAddress(r_PC);
            //6502 has a bug such that the when the vector of anindirect address begins at the last byte of a page,
            //the second byte is fetched from the beginning of that page rather than the beginning of the next
            //Recreating here:
            Address Page = location & 0xff00; //取location高位
            r_PC = m_bus.Read(location) |
                   m_bus.Read(Page | ((location + 1) & 0xff))
                           << 8;
        }
            break;
        case PHP:
            //Push Processor Status on Stack
            //The status register will be pushed with the break flag and bit 5 set to 1.
        {
            Byte flags = f_N << 7 |
                         f_V << 6 |
                         1 << 5 | //supposed to always be 1
                         1 << 4 | //PHP pushes with the B flag as 1, no matter what
                         f_D << 3 |
                         f_I << 2 |
                         f_Z << 1 |
                         f_C;
            PushStack(flags);
        }
            break;
        case PLP:
            // Pull Processor Status from Stack
            // The status register will be pulled with the break flag and bit 5 ignored.
        {
            Byte flags = PullStack();
            f_N = flags & NEGATIVE;
            f_V = flags & OVERFLOW1;
            f_D = flags & DECIMAL_MODE;
            f_I = flags & INTERRUPT_DISABLE;
            f_Z = flags & ZERO;
            f_C = flags & CARRY;
        }
            break;
        case PHA:
            PushStack(r_A);
            break;
        case PLA:
            r_A = PullStack();
            SetZN(r_A);
            break;
        case DEY:
            //Decrement Index Y by One
            --r_Y;
            SetZN(r_Y);
            break;
        case DEX:
            --r_X;
            SetZN(r_X);
            break;
        case TAY:
            // Transfer Accumulator to Index Y
            r_Y = r_A;
            SetZN(r_Y);
            break;
        case INY:
            ++r_Y;
            SetZN(r_Y);
            break;
        case INX:
            ++r_X;
            SetZN(r_X);
            break;
        case CLC:
            // Clear Carry Flag
            f_C = false;
            break;
        case SEC:
            f_C = true;
            break;
        case CLI:
            // Clear Interrupt Disable Bit
            f_I = false;
            break;
        case SEI:
            f_I = true;
            break;
        case TYA:
            // Transfer Index Y to Accumulator
            r_A = r_Y;
            SetZN(r_A);
            break;
        case CLV:
            f_V = false;
            break;
        case CLD:
            f_D = false;
            break;
        case SED:
            f_D = true;
            break;
        case TXA:
            r_A = r_X;
            SetZN(r_A);
            break;
        case TXS:
            r_SP = r_X;
            break;
        case TAX:
            r_X = r_A;
            SetZN(r_X);
            break;
        case TSX:
            r_X = r_SP;
            SetZN(r_X);
            break;
    }
    return true;
}

bool CPU::ExecuteBranch(Byte opcode) {
    return false;
}

bool CPU::ExecuteType0(Byte opcode) {
    return false;
}

bool CPU::ExecuteType1(Byte opcode) {
    return false;
}

bool CPU::ExecuteType2(Byte opcode) {
    return false;
}

void CPU::SetPageCrossed(Address a, Address b, int inc) {
    //Page is determined by the high byte
    if ((a & 0xFF00) != (b & 0xFF00)) {
        m_skipCycles += inc;
    }
}

void CPU::SetZN(Byte value) {
    //如果值为0，设置ZERO标志为1
    f_Z = !value;
    //直接取标志位的值
    f_N = value & NEGATIVE;
}
