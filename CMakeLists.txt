#Set the mimum required version of CMake
cmake_minimum_required(VERSION 3.12)

#Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

#Set the project name, the programming language
project(project1 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

#Initialize the SDK
pico_sdk_init()

#Add an executable target to be build from the source files
add_executable(train_cpu train_cpu.c)

#Pull in our pico_stdlib which aggregates commonly used features
target_link_libraries(train_cpu pico_stdlib hardware_pwm)

#Enable USB output, disable UART output
pico_enable_stdio_usb(train_cpu 0)
pico_enable_stdio_uart(train_cpu 1)

#Create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(train_cpu)