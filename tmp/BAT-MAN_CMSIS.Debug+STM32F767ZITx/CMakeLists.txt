cmake_minimum_required(VERSION 3.22)

set(CONTEXT BAT-MAN_CMSIS.Debug+STM32F767ZITx)
set(TARGET ${CONTEXT})
set(OUT_DIR "${SOLUTION_ROOT}/out/BAT-MAN_CMSIS/STM32F767ZITx/Debug")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(LD_SCRIPT "${SOLUTION_ROOT}/BAT-MAN_CMSIS/RTE/Device/STM32F767ZITx/ac6_linker_script.sct.src")
set(LD_REGIONS "${SOLUTION_ROOT}/BAT-MAN_CMSIS/RTE/Device/STM32F767ZITx/regions_STM32F767ZITx.h")
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
set_target_properties(${CONTEXT} PROPERTIES PREFIX "" SUFFIX ".axf" OUTPUT_NAME "BAT-MAN_CMSIS")
set_target_properties(${CONTEXT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUT_DIR})
add_library(${CONTEXT}_GLOBAL INTERFACE)

# Includes
target_include_directories(${CONTEXT} PUBLIC
  ${SOLUTION_ROOT}/BAT-MAN_CMSIS/RTE/_Debug_STM32F767ZITx
  ${SOLUTION_ROOT}/BAT-MAN_CMSIS/RTE/Device/STM32F767ZITx
  ${CMSIS_PACK_ROOT}/ARM/CMSIS/6.1.0/CMSIS/Core/Include
  ${CMSIS_PACK_ROOT}/ARM/CMSIS/6.1.0/CMSIS/Driver/Include
  ${CMSIS_PACK_ROOT}/ARM/CMSIS/6.1.0/CMSIS/RTOS2/Include
  ${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/CMSIS/Device/ST/STM32F7xx/Include
  ${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Inc
  ${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy
  ${SOLUTION_ROOT}/BAT-MAN_CMSIS/Inc
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
    "SHELL:-std=gnu11"
    "SHELL:-Wno-macro-redefined"
    "SHELL:-Wno-pragma-pack"
    "SHELL:-Wno-parentheses-equality"
    "SHELL:-Wno-license-management"
  >
  $<$<COMPILE_LANGUAGE:C,CXX>:
    "SHELL:${_PI}\"${SOLUTION_ROOT}/BAT-MAN_CMSIS/RTE/_Debug_STM32F767ZITx/Pre_Include_Global.h\""
  >
)

# Compile Options Abstractions
add_library(${CONTEXT}_ABSTRACTIONS INTERFACE)
cbuild_set_options_flags(CC "none" "on" "" "" CC_OPTIONS_FLAGS_${CONTEXT})
cbuild_set_options_flags(ASM "none" "on" "" "" ASM_OPTIONS_FLAGS_${CONTEXT})
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
  Group_Source_Files_Inc
  Group_Source_Files_Inc_c
  Group_Source_Files
  ARM_CMSIS_Driver_GPIO_Custom_1_0_0
  ARM_CMSIS_Driver_USART_Custom_1_0_0
  ARM_CMSIS_CORE_6_1_0
  ARM_CMSIS_OS_Tick_SysTick_1_0_5
  Keil_CMSIS_Driver_USART_1_21_0
  Keil_Device_STM32Cube_Framework_Classic_1_3_0
  Keil_Device_STM32Cube_HAL_Common_1_3_0
  Keil_Device_STM32Cube_HAL_Cortex_1_3_0
  Keil_Device_STM32Cube_HAL_DMA_1_3_0
  Keil_Device_STM32Cube_HAL_GPIO_1_3_0
  Keil_Device_STM32Cube_HAL_PWR_1_3_0
  Keil_Device_STM32Cube_HAL_RCC_1_3_0
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
  "SHELL:--entry=Reset_Handler"
  "SHELL:--map"
  "SHELL:--info summarysizes"
  "SHELL:--summary_stderr"
  "SHELL:--diag_suppress=L6314W"
)
set_target_properties(${CONTEXT} PROPERTIES LINK_DEPENDS ${LD_SCRIPT})

# Linker script pre-processing
add_custom_command(TARGET ${CONTEXT} PRE_LINK COMMAND ${CPP} ARGS ${CPP_ARGS_LD_SCRIPT} BYPRODUCTS ${LD_SCRIPT_PP})
