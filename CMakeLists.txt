cmake_minimum_required(VERSION 3.24)
project(NES_emu)

set(CMAKE_CXX_STANDARD 11)

# 设置源文件目录
file(GLOB SOURCES src/*.cpp)

# 设置头文件目录
include_directories(include)

# 设置 SFML 相关的头文件和库目录
set(SFML_INCLUDE_DIR /opt/homebrew/Cellar/sfml/2.5.1_2/include)
set(SFML_LIBRARY_DIR /opt/homebrew/Cellar/sfml/2.5.1_2/lib)

# 查找 SFML 库
find_package(SFML 2.5.1 COMPONENTS graphics window system REQUIRED)

# 添加可执行文件
add_executable(NES_emu ${SOURCES})

# 链接 SFML 库
target_link_libraries(NES_emu PRIVATE sfml-graphics sfml-window sfml-system)