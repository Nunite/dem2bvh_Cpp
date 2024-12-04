cmake_minimum_required(VERSION 3.10)
project(demo_analysis)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 定义源文件
set(OPENDEMO_SOURCES 
    cpp-exporter/OpenDemo/OpenDemo.cpp
    cpp-exporter/OpenDemo/filereader.cpp
    cpp-exporter/OpenDemo/memoryreader.cpp
    cpp-exporter/OpenDemo/OpenDemoMain.cpp
)

# 显示找到的源文件
message(STATUS "Using OpenDemo sources: ${OPENDEMO_SOURCES}")

# OpenDemo 库
add_library(OpenDemo ${OPENDEMO_SOURCES})

# 添加编译定义以支持 Unicode
target_compile_definitions(OpenDemo PUBLIC 
    UNICODE 
    _UNICODE
)

target_include_directories(OpenDemo PUBLIC
    ${CMAKE_SOURCE_DIR}/cpp-exporter/OpenDemo
)

# demo_analysis 可执行文件
add_executable(demo_analysis 
    ${CMAKE_SOURCE_DIR}/cpp-exporter/OpenDemo/demo_analysis.cpp
)

target_link_libraries(demo_analysis PRIVATE OpenDemo)

# 为 demo_analysis 也添加 Unicode 支持
target_compile_definitions(demo_analysis PUBLIC 
    UNICODE 
    _UNICODE
) 