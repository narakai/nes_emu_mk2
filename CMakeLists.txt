cmake_minimum_required(VERSION 3.24)
project(NES_emu)

set(CMAKE_CXX_STANDARD 11)

add_executable(NES_emu src/main.cpp include/Chip.h include/MainBus.h include/CPU.h include/CPUOpcodes.h src/CPU.cpp src/MainBus.cpp)
