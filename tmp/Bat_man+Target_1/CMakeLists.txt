cmake_minimum_required(VERSION 3.22)

set(CONTEXT Bat_man+Target_1)
set(TARGET ${CONTEXT})
set(OUT_DIR "${SOLUTION_ROOT}/out/Bat_man/Target_1")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(LD_SCRIPT "${SOLUTION_ROOT}/RTE/Device/STM32F767ZITx/ac6_linker_script.sct.src")
set(LD_REGIONS "${SOLUTION_ROOT}/RTE/Device/STM32F767ZITx/regions_STM32F767ZITx.h")
set(LD_SCRIPT_PP "${CMAKE_CURRENT_BINARY_DIR}/ac6_linker_script.sct")

# Processor Options
set(CPU Cortex-M7)
set(FPU DP_FPU)
set(BYTE_ORDER Little-endian)

# Toolchain config map
include("toolchain.cmake")

# Setup project
project(${CONTEXT} LANGUAGES C ASM)

# Compilation database
add_custom_target(database COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" "${OUT_DIR}")
set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})

# Setup context
add_executable(${CONTEXT})
set_target_properties(${CONTEXT} PROPERTIES PREFIX "" SUFFIX ".axf" OUTPUT_NAME "Bat_man")
set_target_properties(${CONTEXT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUT_DIR})
add_library(${CONTEXT}_GLOBAL INTERFACE)

# Includes
target_include_directories(${CONTEXT} PUBLIC
  ${SOLUTION_ROOT}/RTE/_Target_1
  ${SOLUTION_ROOT}/RTE/Device/STM32F767ZITx
  ${SOLUTION_ROOT}/RTE/Device/STM32F767ZITx/STCubeGenerated
  ${SOLUTION_ROOT}/RTE/Device/STM32F767ZITx/STCubeGenerated/Inc
  ${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Include
  ${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/PrivateInclude
  ${CMSIS_PACK_ROOT}/ARM/CMSIS/6.1.0/CMSIS/Core/Include
  ${CMSIS_PACK_ROOT}/ARM/CMSIS/6.1.0/CMSIS/Driver/Include
  ${CMSIS_PACK_ROOT}/ARM/CMSIS/6.1.0/CMSIS/RTOS2/Include
  ${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/CMSIS/Device/ST/STM32F7xx/Include
  ${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Inc
  ${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy
)

# Defines
target_compile_definitions(${CONTEXT} PUBLIC
  $<$<COMPILE_LANGUAGE:C,CXX>:
    STM32F767xx
    _RTE_
  >
)

# Compile options
target_compile_options(${CONTEXT} PUBLIC
  $<$<COMPILE_LANGUAGE:ASM>:
    "SHELL:${ASM_CPU}"
    "SHELL:${ASM_FLAGS}"
    "SHELL:${ASM_BYTE_ORDER}"
    "SHELL:-masm=auto"
  >
  $<$<COMPILE_LANGUAGE:C>:
    "SHELL:${CC_CPU}"
    "SHELL:${CC_FLAGS}"
    "SHELL:${CC_BYTE_ORDER}"
    "SHELL:-std=c99"
    "SHELL:-fno-rtti"
    "SHELL:-funsigned-char"
    "SHELL:-fshort-enums"
    "SHELL:-fshort-wchar"
    "SHELL:-ffunction-sections"
    "SHELL:-Wall"
    "SHELL:-Wextra"
    "SHELL:-Wno-packed"
    "SHELL:-Wno-reserved-id-macro"
    "SHELL:-Wno-unused-macros"
    "SHELL:-Wno-documentation-unknown-command"
    "SHELL:-Wno-documentation"
    "SHELL:-Wno-license-management"
    "SHELL:-Wno-parentheses-equality"
  >
  $<$<COMPILE_LANGUAGE:C,CXX>:
    "SHELL:${_PI}\"${SOLUTION_ROOT}/RTE/_Target_1/Pre_Include_Global.h\""
  >
)

# Compile Options Abstractions
add_library(${CONTEXT}_ABSTRACTIONS INTERFACE)
cbuild_set_options_flags(CC "debug" "on" "" "" CC_OPTIONS_FLAGS_${CONTEXT})
cbuild_set_options_flags(ASM "debug" "on" "" "" ASM_OPTIONS_FLAGS_${CONTEXT})
target_compile_options(${CONTEXT}_ABSTRACTIONS INTERFACE
  $<$<COMPILE_LANGUAGE:C>:
    "SHELL:${CC_OPTIONS_FLAGS_${CONTEXT}}"
  >
  $<$<COMPILE_LANGUAGE:ASM>:
    "SHELL:${ASM_OPTIONS_FLAGS_${CONTEXT}}"
  >
)

# Add groups and components
include("groups.cmake")
include("components.cmake")

target_link_libraries(${CONTEXT} PUBLIC
  Group_STM32CubeMX_Common_Sources
  ARM_CMSIS_Driver_Ethernet_Custom_1_0_0
  ARM_CMSIS_Driver_GPIO_Custom_1_0_0
  ARM_CMSIS_CORE_6_1_0
  ARM_CMSIS_DSP_Source_1_16_2
  ARM_CMSIS_OS_Tick_SysTick_1_0_5
  Keil_CMSIS_Driver_CAN_1_15_0
  Keil_CMSIS_Driver_SPI_1_15_0
  Keil_CMSIS_Driver_USART_1_21_0
  Keil_Device_STM32Cube_Framework_STM32CubeMX_1_1_0
  Keil_Device_STM32Cube_HAL_CAN_1_3_0
  Keil_Device_STM32Cube_HAL_Common_1_3_0
  Keil_Device_STM32Cube_HAL_Cortex_1_3_0
  Keil_Device_STM32Cube_HAL_DMA_1_3_0
  Keil_Device_STM32Cube_HAL_ETH_1_3_0
  Keil_Device_STM32Cube_HAL_GPIO_1_3_0
  Keil_Device_STM32Cube_HAL_PWR_1_3_0
  Keil_Device_STM32Cube_HAL_RCC_1_3_0
  Keil_Device_STM32Cube_HAL_SPI_1_3_0
  Keil_Device_STM32Cube_HAL_UART_1_3_0
  Keil_Device_STM32Cube_HAL_USART_1_3_0
  Keil_Device_STM32Cube_LL_Common_1_3_0
  Keil_Device_STM32Cube_LL_PWR_1_3_0
  Keil_Device_STM32Cube_LL_RCC_1_3_0
  Keil_Device_STM32Cube_LL_UTILS_1_3_0
  Keil_Device_Startup_1_2_5
)

# Linker options
target_link_options(${CONTEXT} PUBLIC
  "SHELL:${LD_CPU}"
  "SHELL:${_LS}\"${LD_SCRIPT_PP}\""
  "SHELL:--map"
  "SHELL:--load_addr_map_info"
  "SHELL:--xref"
  "SHELL:--callgraph"
  "SHELL:--symbols"
  "SHELL:--info sizes"
  "SHELL:--info totals"
  "SHELL:--info unused"
  "SHELL:--info veneers"
  "SHELL:--strict"
  "SHELL:--summary_stderr"
  "SHELL:--info summarysizes"
)
set_target_properties(${CONTEXT} PROPERTIES LINK_DEPENDS ${LD_SCRIPT})

# Linker script pre-processing
add_custom_command(TARGET ${CONTEXT} PRE_LINK COMMAND ${CPP} ARGS ${CPP_ARGS_LD_SCRIPT} BYPRODUCTS ${LD_SCRIPT_PP})
