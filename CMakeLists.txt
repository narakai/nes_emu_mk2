cmake_minimum_required(VERSION 3.24)
project(NES_emu)

set(CMAKE_CXX_STANDARD 11)

# 设置工作目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# 添加可执行文件
add_executable(NES_emu src/main.cpp include/Chip.h include/MainBus.h include/CPU.h include/CPUOpcodes.h src/CPU.cpp src/MainBus.cpp src/Log.cpp include/Log.h src/Cartridge.cpp include/Cartridge.h)