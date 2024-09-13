# components.cmake

# component ARM::CMSIS Driver:GPIO:Custom@1.0.0
add_library(ARM_CMSIS_Driver_GPIO_Custom_1_0_0 INTERFACE)
target_include_directories(ARM_CMSIS_Driver_GPIO_Custom_1_0_0 INTERFACE
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
  ${CMSIS_PACK_ROOT}/ARM/CMSIS/6.1.0/CMSIS/Driver/Include
)
target_compile_definitions(ARM_CMSIS_Driver_GPIO_Custom_1_0_0 INTERFACE
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)

# component ARM::CMSIS Driver:USART:Custom@1.0.0
add_library(ARM_CMSIS_Driver_USART_Custom_1_0_0 INTERFACE)
target_include_directories(ARM_CMSIS_Driver_USART_Custom_1_0_0 INTERFACE
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
  ${CMSIS_PACK_ROOT}/ARM/CMSIS/6.1.0/CMSIS/Driver/Include
)
target_compile_definitions(ARM_CMSIS_Driver_USART_Custom_1_0_0 INTERFACE
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)

# component ARM::CMSIS:CORE@6.1.0
add_library(ARM_CMSIS_CORE_6_1_0 INTERFACE)
target_include_directories(ARM_CMSIS_CORE_6_1_0 INTERFACE
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
  ${CMSIS_PACK_ROOT}/ARM/CMSIS/6.1.0/CMSIS/Core/Include
)
target_compile_definitions(ARM_CMSIS_CORE_6_1_0 INTERFACE
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)

# component ARM::CMSIS:OS Tick:SysTick@1.0.5
add_library(ARM_CMSIS_OS_Tick_SysTick_1_0_5 OBJECT
  "${CMSIS_PACK_ROOT}/ARM/CMSIS/6.1.0/CMSIS/RTOS2/Source/os_systick.c"
)
target_include_directories(ARM_CMSIS_OS_Tick_SysTick_1_0_5 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(ARM_CMSIS_OS_Tick_SysTick_1_0_5 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(ARM_CMSIS_OS_Tick_SysTick_1_0_5 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(ARM_CMSIS_OS_Tick_SysTick_1_0_5 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)

# component Keil::CMSIS Driver:USART@1.21.0
add_library(Keil_CMSIS_Driver_USART_1_21_0 OBJECT
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/CMSIS/Driver/USART_STM32F7xx.c"
)
target_include_directories(Keil_CMSIS_Driver_USART_1_21_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Keil_CMSIS_Driver_USART_1_21_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_CMSIS_Driver_USART_1_21_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_CMSIS_Driver_USART_1_21_0 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)

# component Keil::Device:STM32Cube Framework:Classic@1.3.0
add_library(Keil_Device_STM32Cube_Framework_Classic_1_3_0 INTERFACE)
target_include_directories(Keil_Device_STM32Cube_Framework_Classic_1_3_0 INTERFACE
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
  ${SOLUTION_ROOT}/BAT-MAN_CMSIS/RTE/Device/STM32F767ZITx
)
target_compile_definitions(Keil_Device_STM32Cube_Framework_Classic_1_3_0 INTERFACE
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)

# component Keil::Device:STM32Cube HAL:Common@1.3.0
add_library(Keil_Device_STM32Cube_HAL_Common_1_3_0 OBJECT
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c"
)
target_include_directories(Keil_Device_STM32Cube_HAL_Common_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
  ${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Inc
)
target_compile_definitions(Keil_Device_STM32Cube_HAL_Common_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_Device_STM32Cube_HAL_Common_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_Device_STM32Cube_HAL_Common_1_3_0 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)

# component Keil::Device:STM32Cube HAL:Cortex@1.3.0
add_library(Keil_Device_STM32Cube_HAL_Cortex_1_3_0 OBJECT
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c"
)
target_include_directories(Keil_Device_STM32Cube_HAL_Cortex_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Keil_Device_STM32Cube_HAL_Cortex_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_Device_STM32Cube_HAL_Cortex_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_Device_STM32Cube_HAL_Cortex_1_3_0 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)

# component Keil::Device:STM32Cube HAL:DMA@1.3.0
add_library(Keil_Device_STM32Cube_HAL_DMA_1_3_0 OBJECT
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c"
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma_ex.c"
)
target_include_directories(Keil_Device_STM32Cube_HAL_DMA_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Keil_Device_STM32Cube_HAL_DMA_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_Device_STM32Cube_HAL_DMA_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_Device_STM32Cube_HAL_DMA_1_3_0 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)

# component Keil::Device:STM32Cube HAL:GPIO@1.3.0
add_library(Keil_Device_STM32Cube_HAL_GPIO_1_3_0 OBJECT
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c"
)
target_include_directories(Keil_Device_STM32Cube_HAL_GPIO_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Keil_Device_STM32Cube_HAL_GPIO_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_Device_STM32Cube_HAL_GPIO_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_Device_STM32Cube_HAL_GPIO_1_3_0 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)

# component Keil::Device:STM32Cube HAL:PWR@1.3.0
add_library(Keil_Device_STM32Cube_HAL_PWR_1_3_0 OBJECT
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr.c"
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c"
)
target_include_directories(Keil_Device_STM32Cube_HAL_PWR_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Keil_Device_STM32Cube_HAL_PWR_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_Device_STM32Cube_HAL_PWR_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_Device_STM32Cube_HAL_PWR_1_3_0 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)

# component Keil::Device:STM32Cube HAL:RCC@1.3.0
add_library(Keil_Device_STM32Cube_HAL_RCC_1_3_0 OBJECT
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c"
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c"
)
target_include_directories(Keil_Device_STM32Cube_HAL_RCC_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Keil_Device_STM32Cube_HAL_RCC_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_Device_STM32Cube_HAL_RCC_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_Device_STM32Cube_HAL_RCC_1_3_0 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)

# component Keil::Device:STM32Cube LL:Common@1.3.0
add_library(Keil_Device_STM32Cube_LL_Common_1_3_0 INTERFACE)
target_include_directories(Keil_Device_STM32Cube_LL_Common_1_3_0 INTERFACE
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
  ${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Inc
  ${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy
)
target_compile_definitions(Keil_Device_STM32Cube_LL_Common_1_3_0 INTERFACE
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)

# component Keil::Device:STM32Cube LL:PWR@1.3.0
add_library(Keil_Device_STM32Cube_LL_PWR_1_3_0 OBJECT
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_pwr.c"
)
target_include_directories(Keil_Device_STM32Cube_LL_PWR_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Keil_Device_STM32Cube_LL_PWR_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_Device_STM32Cube_LL_PWR_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_Device_STM32Cube_LL_PWR_1_3_0 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)

# component Keil::Device:STM32Cube LL:RCC@1.3.0
add_library(Keil_Device_STM32Cube_LL_RCC_1_3_0 OBJECT
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_rcc.c"
)
target_include_directories(Keil_Device_STM32Cube_LL_RCC_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Keil_Device_STM32Cube_LL_RCC_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_Device_STM32Cube_LL_RCC_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_Device_STM32Cube_LL_RCC_1_3_0 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)

# component Keil::Device:STM32Cube LL:UTILS@1.3.0
add_library(Keil_Device_STM32Cube_LL_UTILS_1_3_0 OBJECT
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_utils.c"
)
target_include_directories(Keil_Device_STM32Cube_LL_UTILS_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Keil_Device_STM32Cube_LL_UTILS_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_Device_STM32Cube_LL_UTILS_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_Device_STM32Cube_LL_UTILS_1_3_0 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)

# component Keil::Device:Startup@1.2.5
add_library(Keil_Device_Startup_1_2_5 OBJECT
  "${SOLUTION_ROOT}/BAT-MAN_CMSIS/RTE/Device/STM32F767ZITx/startup_stm32f767xx.s"
  "${SOLUTION_ROOT}/BAT-MAN_CMSIS/RTE/Device/STM32F767ZITx/system_stm32f7xx.c"
)
target_include_directories(Keil_Device_Startup_1_2_5 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
  ${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/CMSIS/Device/ST/STM32F7xx/Include
)
target_compile_definitions(Keil_Device_Startup_1_2_5 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_Device_Startup_1_2_5 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_Device_Startup_1_2_5 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)
set(COMPILE_DEFINITIONS
  STM32F767xx
  _RTE_
)
cbuild_set_defines(AS_ARM COMPILE_DEFINITIONS)
set_source_files_properties("${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/MDK/Device/Source/ARM/STM32F76x_77x_OPT.s" PROPERTIES
  COMPILE_FLAGS "${COMPILE_DEFINITIONS}"
)
set(COMPILE_DEFINITIONS
  STM32F767xx
  _RTE_
)
cbuild_set_defines(AS_ARM COMPILE_DEFINITIONS)
set_source_files_properties("${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/MDK/Device/Source/ARM/STM32F76x_77x_OTP.s" PROPERTIES
  COMPILE_FLAGS "${COMPILE_DEFINITIONS}"
)
set(COMPILE_DEFINITIONS
  STM32F767xx
  _RTE_
)
cbuild_set_defines(AS_ARM COMPILE_DEFINITIONS)
set_source_files_properties("${SOLUTION_ROOT}/BAT-MAN_CMSIS/RTE/Device/STM32F767ZITx/startup_stm32f767xx.s" PROPERTIES
  COMPILE_FLAGS "${COMPILE_DEFINITIONS}"
)
