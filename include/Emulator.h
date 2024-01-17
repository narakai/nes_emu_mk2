//
// Created by lai leon on 29/6/2023.
//

#ifndef NES_EMU_EMULATOR_H
#define NES_EMU_EMULATOR_H

#include <CPU.h>
#include <Cartridge.h>
#include <MainBus.h>
#include <Mapper.h>
#include <SFML/Graphics.hpp>
#include <VirtualScreen.h>
#include <PPU.h>
#include <chrono>
#include <PictureBus.h>


const int NESVideoWidth = ScanlineVisibleDots;
const int NESVideoHeight = VisibleScanlines;

class Emulator {
public:
    Emulator();

    void Run(std::string rom_path);

private:
    CPU m_cpu;
    Cartridge m_cartridge;
    MainBus m_bus;
    std::unique_ptr<Mapper> m_mapper;
    sf::RenderWindow m_window;
    VirtualScreen m_emulatorScreen;
    // 控制屏幕缩放
    float m_screenScale;

    void DMA(Byte page);
    PPU m_ppu;
    PictureBus m_pictureBus;

    // 计时
    std::chrono::high_resolution_clock::time_point m_cycleTimer;
    std::chrono::high_resolution_clock::duration m_elapsedTime;
    std::chrono::nanoseconds m_cpuCycleDuration;
};


#endif //NES_EMU_EMULATOR_H
