//
// Created by lai leon on 9/6/2023.
//
#include <ios>
#include <CPU.h>
#include <CPUOpcodes.h>
#include <Log.h>
/* CPU 6502 */

/*
参考：中文版 http://nesdev.com/nes_c.txt
     英文版： http://fms.komkon.org/EMUL8/NES.html
CPU Memory Map
--------------------------------------- $10000
 Upper Bank of Cartridge ROM            卡带的上层ROM
--------------------------------------- $C000
 Lower Bank of Cartridge ROM            卡带的下层ROM
--------------------------------------- $8000
 Cartridge RAM (maybe battery-backed)  卡带的RAM（可能有电池支持）
--------------------------------------- $6000
 Expansion Modules                      扩充的模块
--------------------------------------- $5000
 Input/Output                           输入/输出
--------------------------------------- $2000
 2kB Internal RAM, mirrored 4 times     2KB的内部RAM，做4次镜象
--------------------------------------- $0000
*/

// & 取值操作，取mask为1的位
// ｜ 赋值操作

const auto CARRY = 0b00000001; //0x1
const auto ZERO = 0b00000010; //0x2
const auto INTERRUPT_DISABLE = 0b00000100; //0x4
const auto DECIMAL_MODE = 0b00001000; //0x8
const auto BREAK = 0b00010000;
const auto BREAK2 = 0b00100000;
const auto OVERFLOW1 = 0b01000000; //0x40
const auto NEGATIVE = 0b10000000; //0x80

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
                        ExecuteType0(opcode) || ExecuteType1(opcode) || ExecuteType2(opcode))) {
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
            // Transfer Accumulator_ to Index Y
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
            // Transfer Index Y to Accumulator_
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
        default:
            return false;
    }
    return true;
}

//The conditional branch instructions all have the form xxy10000.
//The flag indicated by xx is compared with y, and the branch is taken if they are equal.
//
//xx	flag
//00	negative
//01	overflow
//10	carry
//11	zero

bool CPU::ExecuteBranch(Byte opcode) {
    // 跳转指令实现
    if ((opcode & BranchInstructionMask) == BranchInstructionMaskResult) {
        //branch is initialized to the condition required (for the flag specified later)
        bool branch = opcode & BranchConditionMask;

        //set branch to true if the given condition is met by the given flag
        //We use xnor here, it is true if either both operands are true or false
        switch (opcode >> BranchOnFlagShift) {
            case Negative:
                // opcode = 0x10 为BPL指令 在获得 高位为1时才跳转
                // 在马里奥游戏中前二十条指令存在 LDA 2020 , BPL 0xFB指令
                // 0xF8 解释为 -5，LDA指令为4字长，所以PC会再次跳转到 LDA 2002上
                // 也就是说必须等待PPU为vblank时才开始执行下一条指令
                branch = !(branch ^ f_N);
                break;
            case Overflow:
                branch = !(branch ^ f_V);
                break;
            case Carry:
                branch = !(branch ^ f_C);
                break;
            case Zero:
                branch = !(branch ^ f_Z);
                break;
            default:
                return false;
        }
        if (branch) {
            int8_t offset = m_bus.Read(r_PC++);
            ++m_skipCycles;
            auto newPC = static_cast<Address>(r_PC + offset);
            SetPageCrossed(r_PC, newPC, 2);
            r_PC = newPC;
        } else {
            ++r_PC;
        }
        return true;
    }
    return false;
}

//the cc = 00 instructions
//https://happysoul.github.io/nes/6502/
//https://www.masswerk.at/6502/6502_instruction_set.html
bool CPU::ExecuteType0(Byte opcode) {
    if ((opcode & InstructionModeMask) == 0x0) {
        //先寻址
        Address location = 0;
        // if (opcode == DEBUG_OPCODE)
        // {
        //     LOG(Info) << "Fetch Instruction :" << std::hex << static_cast<int>(DEBUG_OPCODE) << std::endl;
        // }
        switch (static_cast<AddressingMode2>((opcode & AddrModeMask) >> AddrModeShift)) {
            case Immediate_:
                location = r_PC++;
                break;
            case ZeroPage_:
                location = m_bus.Read(r_PC++);
                break;
            case Accumulator_:
                break;
            case Absolute_:
                location = ReadAddress(r_PC);
                r_PC += 2;
                break;
            case ZeroPageX_:
                // Address wraps around in the zero page
                location = (m_bus.Read(r_PC++) + r_X) & 0xff;
                break;
            case AbsoluteX_:
                location = ReadAddress(r_PC);
                r_PC += 2;
                SetPageCrossed(location, location + r_X);
                location += r_X;
                break;
            default:
                return false;
        }

        std::uint16_t operand = 0;
        switch (static_cast<Operation0>((opcode & OperationMask) >> OperationShift)) {

            case BIT:
                // bits 7 and 6 of operand are transfered to bit 7 and 6 of SR (N,V);
                // the zero-flag is set to the result of operand AND accumulator.
                operand = m_bus.Read(location);
                f_Z = !(r_A & operand);
                f_V = operand & OVERFLOW1;
                f_N = operand & NEGATIVE;
                break;
            case STY:
                m_bus.Write(location, r_Y);
                break;
            case LDY:
                r_Y = m_bus.Read(location);
                SetZN(r_Y);
                break;
            case CPY: {
                std::uint16_t diff = r_Y - m_bus.Read(location);
                f_C = !(diff & 0x100);
                SetZN(diff);
            }
                break;
            case CPX: {
                std::uint16_t diff = r_X - m_bus.Read(location);
                f_C = !(diff & 0x100);
                SetZN(diff);
            }
                break;
            default:
                return false;
        }

        return true;
    }
    return false;
}

//the cc = 01 instructions
bool CPU::ExecuteType1(Byte opcode) {
    if ((opcode & InstructionModeMask) == 0x1) {
        //first 寻址
        Address location = 0;
        auto op = static_cast<Operation1>((opcode & OperationMask) >> OperationShift);

        switch (static_cast<AddressingMode1>(
                (opcode & AddrModeMask) >> AddrModeShift)) {

            case ZeroPageX0: {
                Byte zero_addr = r_X + m_bus.Read(r_PC++);
                //Addresses wrap in zero page mode, thus pass through a mask
                location = m_bus.Read(zero_addr & 0xff) | m_bus.Read((zero_addr + 1) & 0xff) << 8;
            }
                break;
            case ZeroPage:
                location = m_bus.Read(r_PC++);
                break;
            case Immediate:
                location = r_PC++;
                break;
            case Absolute:
                location = ReadAddress(r_PC);
                r_PC += 2;
                break;
            case ZeroPageY: {
                Byte zero_addr = m_bus.Read(r_PC++);
                location = m_bus.Read(zero_addr & 0xff) | m_bus.Read((zero_addr + 1) & 0xff) << 8;
                if (op != STA)
                    SetPageCrossed(location, location + r_Y);
                location += r_Y;
            }
                break;
            case ZeroPageX:
                // Address wraps around in the zero page
                location = (m_bus.Read(r_PC++) + r_X) & 0xff;
                break;
            case AbsoluteY:
                location = ReadAddress(r_PC);
                r_PC += 2;
                if (op != STA)
                    SetPageCrossed(location, location + r_Y);
                location += r_Y;
                break;
            case AbsoluteX:
                location = ReadAddress(r_PC);
                r_PC += 2;
                if (op != STA)
                    SetPageCrossed(location, location + r_X);
                location += r_X;
                break;
            default:
                return false;
        }

        switch (op) {

            case ORA:
                r_A = m_bus.Read(location) | r_A;
                SetZN(r_A);
                break;
            case AND:
                r_A = m_bus.Read(location) & r_A;
                SetZN(r_A);
                break;
            case EOR:
                r_A = m_bus.Read(location) ^ r_A;
                SetZN(r_A);
                break;
            case ADC: {
                Byte operand = m_bus.Read(location);
                std::uint16_t sum = r_A + operand + f_C;
                //Carry forward or UNSIGNED overflow
                f_C = sum & 0x100;
                //SIGNED overflow, would only happen if the sign of sum is
                //different from BOTH the operands
                f_V = (r_A ^ sum) & (operand ^ sum) & NEGATIVE;
                r_A = static_cast<Byte>(sum);
                SetZN(r_A);
            }
                break;
            case STA:
                m_bus.Write(location, r_A);
                break;
            case LDA:
                r_A = m_bus.Read(location);
                SetZN(r_A);
//                LOG(Info) << "LDA: "
//                          << std::hex
//                          << static_cast<int>(location)
//                          << "\t R_A IS "
//                          << std::hex
//                          << static_cast<int>(r_A)
//                          << std::endl;
                break;
            case CMP: {
                std::uint16_t diff = r_A - m_bus.Read(location);
                f_C = !(diff & 0x100);
                SetZN(diff);
            }
                break;
            case SBC:
                // Subtract Memory from Accumulator with Borrow
            {
                //High carry means "no borrow", thus negate and subtract
                std::uint16_t subtrahend = m_bus.Read(location);
                std::uint16_t diff = r_A - subtrahend - !f_C;
                //if the ninth bit is 1, the resulting number is negative => borrow => low carry
                f_C = !(diff & 0x100);
                //Same as ADC, except instead of the subtrahend,
                //substitute with it's one complement
                f_V = (r_A ^ diff) & (~subtrahend ^ diff) & NEGATIVE;
                r_A = diff;
                SetZN(diff);
            }
                break;
            default:
                return false;
        }
        return true;
    }
    return false;
}

//the cc = 10 instructions
bool CPU::ExecuteType2(Byte opcode) {
    if ((opcode & InstructionModeMask) == 0x2) {
        //first 寻址
        Address location = 0;
        auto op = static_cast<Operation2>((opcode & OperationMask) >> OperationShift);
        auto addr_mode =
                static_cast<AddressingMode2>((opcode & AddrModeMask) >> AddrModeShift);
        switch (addr_mode) {
            case Immediate_:
                location = r_PC++;
                break;
            case ZeroPage_:
                location = m_bus.Read(r_PC++);
                break;
            case Accumulator_:
                break;
            case Absolute_:
                location = ReadAddress(r_PC);
                r_PC += 2;
                break;
            case ZeroPageX_:
                // Address wraps around in the zero page
            {
                location = m_bus.Read(r_PC++);
                Byte index;
                if (op == LDX || op == STX)
                    index = r_Y;
                else
                    index = r_X;
                //The mask wraps address around zero page
                location = (location + index) & 0xff;
            }
                break;
            case AbsoluteX_: {
                location = ReadAddress(r_PC);
                r_PC += 2;
                Byte index;
                if (op == LDX || op == STX)
                    index = r_Y;
                else
                    index = r_X;
                SetPageCrossed(location, location + index);
                location += index;
            }
                break;
            default:
                return false;
        }

        std::uint16_t operand = 0;
        switch (op) {
            case ASL:
            case ROL:
                if (addr_mode == Accumulator_) {
                    //update r_A
                    //将当前的进位标志位 (f_C) 保存到变量 prev_C 中
                    auto prev_C = f_C;
                    //将进位标志位 (f_C) 设置为寄存器 A 的最高位 (bit 7)
                    f_C = r_A & NEGATIVE;
                    //将寄存器 A 左移一位，相当于将其每个位向左移动一位，最低位 (bit 0) 填充为 0
                    r_A <<= 1;
                    //如果操作 (op) 是循环左移 (ROL)，将最低位 (bit 0) 设置为先前保存的进位标志位 (prev_C) 的值。
                    r_A = r_A | (prev_C && (op == ROL));
                    //根据寄存器 A 的新值，设置零标志位 (Zero Flag，标志位 Z) 和负标志位 (Negative Flag，标志位 N)
                    SetZN(r_A);
                } else {
                    //update operand
                    auto prev_C = f_C;
                    operand = m_bus.Read(location);
                    f_C = operand & NEGATIVE;
                    operand = operand << 1 | (prev_C && (op == ROL));
                    SetZN(operand);
                    m_bus.Write(location, operand);
                }
                break;
            case LSR:
            case ROR:
                if (addr_mode == Accumulator_) {
                    //update r_A
                    auto prev_C = f_C;
                    f_C = r_A & CARRY;
                    r_A >>= 1;
                    //If Rotating, set the bit-7 to the previous carry
                    r_A = r_A | (prev_C && (op == ROR)) << 7;
                    SetZN(r_A);
                } else {
                    //update operand
                    auto prev_C = f_C;
                    operand = m_bus.Read(location);
                    f_C = operand & CARRY;
                    operand = operand >> 1 | (prev_C && (op == ROL)) << 7;
                    SetZN(operand);
                    m_bus.Write(location, operand);
                }
                break;
            case STX:
                m_bus.Write(location, r_X);
                break;
            case LDX:
                r_X = m_bus.Read(location);
                SetZN(r_X);
                break;
            case DEC: {
                auto tmp = m_bus.Read(location) - 1;
                SetZN(tmp);
                m_bus.Write(location, tmp);
            }
                break;
            case INC: {
                auto tmp = m_bus.Read(location) + 1;
                SetZN(tmp);
                m_bus.Write(location, tmp);
            }
                break;
            default:
                return false;
        }
        return true;
    }
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
