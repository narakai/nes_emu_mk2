#include <iostream>
#include <MainBus.h>
#include <CPU.h>
#include <Log.h>
#include <Emulator.h>

int main(int argc, char **argv) {
    //log setting
    Log::get().setLogStream(std::cout);
    Log::get().setLevel(Info);

//    Cartridge cartridge;
//    cartridge.LoadFromFile("./rom/example.nes");
//    std::cout << "cartridge rom size "
//              << cartridge.GetROM().size() << std::endl;
//    MainBus bus;
//    CPU test_cpu(bus);
//
//    test_cpu.Reset();
//    std::cout << "After reset the PC is :" << test_cpu.GetPC() << std::endl;
//    // CPU取指解码执行
//    int cycle = 4;
//    while (cycle--) {
//        test_cpu.Step();
//    }
//    Byte accVal = test_cpu.GetACC();
//    /* 一定要记得加强制转换，不然输出为空白，调试半天，我还以为哪里出了bug*/
//    std::cout << "[+]执行4+2的操作后，ACC寄存器的值为:"
//              << static_cast<int>(accVal) << std::endl;

    Emulator emulator;
    if (argc < 2) {
        std::cout << "Usage: ./NES_emu [ROM File Path]" << std::endl;
        return -1;
    }
    std::string romfile = argv[1];
    emulator.Run(romfile);

    return 0;
}