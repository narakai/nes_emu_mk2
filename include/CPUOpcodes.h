//
// Created by lai leon on 9/6/2023.
//

#ifndef NES_EMU_CPUOPCODES_H
#define NES_EMU_CPUOPCODES_H

/* 6502 CPU 操作码相关定义*/

/* 复位向量地址，当CPU复位时，CPU将从此处地址开始取指执行 */
const auto ResetVector = 0xfffc;

#endif //NES_EMU_CPUOPCODES_H
