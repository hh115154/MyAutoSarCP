cmake_minimum_required(VERSION 3.16)

project(MyAutoSarCP
    VERSION 1.0.0
    DESCRIPTION "AUTOSAR Classic Platform R25-11 Reference Implementation"
    LANGUAGES C CXX
)

# ==============================================================
# C Standard (AUTOSAR CP: C99 / C11)
# ==============================================================
set(CMAKE_C_STANDARD   11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD   17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==============================================================
# Compile Options
# ==============================================================
add_compile_options(
    -Wall -Wextra -Wpedantic
    -Wshadow -Wno-unused-parameter
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -DDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O2 -DNDEBUG)
endif()

# ==============================================================
# Global Include Paths
# ==============================================================
include_directories(
    ${CMAKE_SOURCE_DIR}/BSW/SystemServices
    ${CMAKE_SOURCE_DIR}/BSW/SystemServices/Os
    ${CMAKE_SOURCE_DIR}/BSW/SystemServices/EcuM
    ${CMAKE_SOURCE_DIR}/BSW/SystemServices/Det
    ${CMAKE_SOURCE_DIR}/BSW/SystemServices/WdgM
    ${CMAKE_SOURCE_DIR}/BSW/SystemServices/BswM
    ${CMAKE_SOURCE_DIR}/BSW/CommunicationServices/ComM
    ${CMAKE_SOURCE_DIR}/BSW/CommunicationServices/Com
    ${CMAKE_SOURCE_DIR}/BSW/CommunicationServices/PduR
    ${CMAKE_SOURCE_DIR}/BSW/CommunicationServices/CanIf
    ${CMAKE_SOURCE_DIR}/BSW/CommunicationServices/SoAd
    ${CMAKE_SOURCE_DIR}/BSW/CommunicationServices/SomeIp
    ${CMAKE_SOURCE_DIR}/BSW/MemoryServices/NvM
    ${CMAKE_SOURCE_DIR}/BSW/DiagnosticServices/Dcm
    ${CMAKE_SOURCE_DIR}/BSW/DiagnosticServices/Dem
    ${CMAKE_SOURCE_DIR}/MCAL/Dio
    ${CMAKE_SOURCE_DIR}/MCAL/Can
    ${CMAKE_SOURCE_DIR}/MCAL/Mcu
    ${CMAKE_SOURCE_DIR}/RTE
    ${CMAKE_SOURCE_DIR}/Application/SwcEngine
    ${CMAKE_SOURCE_DIR}/Application/SwcBattery
)

# ==============================================================
# Libraries
# ==============================================================

# RTE
add_library(rte STATIC RTE/Rte.c)

# BSW stubs (host-build)
add_library(bsw_ecum  STATIC BSW/SystemServices/EcuM/EcuM.c)
add_library(bsw_dem   STATIC BSW/DiagnosticServices/Dem/Dem.c)
add_library(bsw_det   STATIC BSW/SystemServices/Det/Det.c)

# BSW Communication: SoAd + SOME/IP Provider + HMI Cmd Receiver
add_library(bsw_soad  STATIC BSW/CommunicationServices/SoAd/SoAd.c)
add_library(bsw_someip_provider STATIC
    BSW/CommunicationServices/SomeIp/SomeIpProvider.c)
target_link_libraries(bsw_someip_provider PRIVATE bsw_soad)

add_library(bsw_hmi_cmd STATIC
    BSW/CommunicationServices/SomeIp/HmiCmdReceiver.c)
# bsw_hmi_cmd 包含 SwcEngine.h，include 路径由全局 include_directories 覆盖
# 链接时 swc_engine 提供符号，放在 executable 的 target_link_libraries 里即可

# Application SWCs
add_library(swc_engine  STATIC Application/SwcEngine/SwcEngine.c)
add_library(swc_battery STATIC Application/SwcBattery/SwcBattery.c)

target_link_libraries(swc_engine  PRIVATE rte)
target_link_libraries(swc_battery PRIVATE rte)

# ==============================================================
# Executable
# ==============================================================
add_executable(MyAutoSarCP Application/main.c)

target_link_libraries(MyAutoSarCP
    PRIVATE
        rte
        bsw_ecum
        bsw_dem
        bsw_det
        bsw_soad
        bsw_someip_provider
        bsw_hmi_cmd
        swc_engine
        swc_battery
        pthread
)

# ==============================================================
# Unit Tests (optional)
# ==============================================================
option(BUILD_TESTS    "Build unit tests"                    OFF)
option(ENABLE_COVERAGE "Enable code coverage (Debug only)" OFF)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(ENABLE_COVERAGE)
    message(STATUS "Coverage enabled. Run: bash scripts/ci.sh coverage")
endif()

# ==============================================================
# Install
# ==============================================================
install(TARGETS MyAutoSarCP RUNTIME DESTINATION bin)
install(DIRECTORY BSW/     DESTINATION include/BSW     FILES_MATCHING PATTERN "*.h")
install(DIRECTORY MCAL/    DESTINATION include/MCAL    FILES_MATCHING PATTERN "*.h")
install(DIRECTORY RTE/     DESTINATION include/RTE     FILES_MATCHING PATTERN "*.h")
install(DIRECTORY Application/ DESTINATION include/App FILES_MATCHING PATTERN "*.h")
