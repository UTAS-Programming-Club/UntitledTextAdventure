cmake_minimum_required(VERSION 3.13)

# initialize the SDK from submodule
# note: this must happen before project()
include(../../../third_party/pico-sdk/pico_sdk_init.cmake)

project(picocalcuta C CXX)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

include_directories(../../)
include_directories(../../../)
include_directories(../../../out/gen/)

add_subdirectory(../../../third_party/PicoCalc/Code/picocalc_kbd_tester/i2ckbd/ i2ckbd/)
add_subdirectory(../../../third_party/PicoCalc/Code/picocalc_kbd_tester/lcdspi/ lcdspi/)

add_executable(uta
    ../../backend.c
    ../../coregame2.c
    ../../../out/gen/coregame.c
    ../../../out/gen/ext1.c
    frontend.c
)

target_link_libraries(uta
  i2ckbd
  lcdspi
)

pico_add_extra_outputs(uta)
