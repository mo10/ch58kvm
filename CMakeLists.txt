cmake_minimum_required(VERSION 3.10)
include(cmake/toolchain.cmake)

project(ch58kvm)

enable_language(C)
enable_language(CXX)
enable_language(ASM)

# Custom linker script
set(TARGET_LD_SCRIPT ${CMAKE_SOURCE_DIR}/ld/Link.ld)
# Custom startup
#set(TARGET_STARTUP_ASM ${CMAKE_SOURCE_DIR}/startup/startup_CH583.S)

# Add OpenWCH SDK
add_subdirectory(openwch)

# Add third-party libraries
add_subdirectory(libs)
target_include_directories(CherryUSB PUBLIC src/usb)

# Add project sources
add_executable(ch58kvm src/main.c src/usb/hid.c)
target_include_directories(ch58kvm PUBLIC src src/usb)

target_link_libraries(ch58kvm OpenWCH58x CherryUSB)

# Generate .bin and .hex
generate_binary_file(ch58kvm)
generate_hex_file(ch58kvm)
