# components.cmake

# component ARM::CMSIS Driver:Ethernet:Custom@1.0.0
add_library(ARM_CMSIS_Driver_Ethernet_Custom_1_0_0 INTERFACE)
target_include_directories(ARM_CMSIS_Driver_Ethernet_Custom_1_0_0 INTERFACE
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
  ${CMSIS_PACK_ROOT}/ARM/CMSIS/6.1.0/CMSIS/Driver/Include
)
target_compile_definitions(ARM_CMSIS_Driver_Ethernet_Custom_1_0_0 INTERFACE
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)

# component ARM::CMSIS Driver:GPIO:Custom@1.0.0
add_library(ARM_CMSIS_Driver_GPIO_Custom_1_0_0 INTERFACE)
target_include_directories(ARM_CMSIS_Driver_GPIO_Custom_1_0_0 INTERFACE
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
  ${CMSIS_PACK_ROOT}/ARM/CMSIS/6.1.0/CMSIS/Driver/Include
)
target_compile_definitions(ARM_CMSIS_Driver_GPIO_Custom_1_0_0 INTERFACE
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

# component ARM::CMSIS:DSP&Source@1.16.2
add_library(ARM_CMSIS_DSP_Source_1_16_2 OBJECT
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/BasicMathFunctions/BasicMathFunctions.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/BasicMathFunctions/BasicMathFunctionsF16.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/BayesFunctions/BayesFunctions.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/BayesFunctions/BayesFunctionsF16.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/CommonTables/CommonTables.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/CommonTables/CommonTablesF16.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/ComplexMathFunctions/ComplexMathFunctions.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/ComplexMathFunctions/ComplexMathFunctionsF16.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/ControllerFunctions/ControllerFunctions.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/DistanceFunctions/DistanceFunctions.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/DistanceFunctions/DistanceFunctionsF16.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/FastMathFunctions/FastMathFunctions.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/FastMathFunctions/FastMathFunctionsF16.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/FilteringFunctions/FilteringFunctions.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/FilteringFunctions/FilteringFunctionsF16.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/InterpolationFunctions/InterpolationFunctions.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/InterpolationFunctions/InterpolationFunctionsF16.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/MatrixFunctions/MatrixFunctions.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/MatrixFunctions/MatrixFunctionsF16.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/QuaternionMathFunctions/QuaternionMathFunctions.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/SVMFunctions/SVMFunctions.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/SVMFunctions/SVMFunctionsF16.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/StatisticsFunctions/StatisticsFunctions.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/StatisticsFunctions/StatisticsFunctionsF16.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/SupportFunctions/SupportFunctions.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/SupportFunctions/SupportFunctionsF16.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/TransformFunctions/TransformFunctions.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/TransformFunctions/TransformFunctionsF16.c"
  "${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Source/WindowFunctions/WindowFunctions.c"
)
target_include_directories(ARM_CMSIS_DSP_Source_1_16_2 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
  ${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/Include
  ${CMSIS_PACK_ROOT}/ARM/CMSIS-DSP/1.16.2/PrivateInclude
)
target_compile_definitions(ARM_CMSIS_DSP_Source_1_16_2 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(ARM_CMSIS_DSP_Source_1_16_2 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(ARM_CMSIS_DSP_Source_1_16_2 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
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

# component Keil::CMSIS Driver:CAN@1.15.0
add_library(Keil_CMSIS_Driver_CAN_1_15_0 OBJECT
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/CMSIS/Driver/CAN_STM32F7xx.c"
)
target_include_directories(Keil_CMSIS_Driver_CAN_1_15_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Keil_CMSIS_Driver_CAN_1_15_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_CMSIS_Driver_CAN_1_15_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_CMSIS_Driver_CAN_1_15_0 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)

# component Keil::CMSIS Driver:SPI@1.15.0
add_library(Keil_CMSIS_Driver_SPI_1_15_0 OBJECT
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/CMSIS/Driver/SPI_STM32F7xx.c"
)
target_include_directories(Keil_CMSIS_Driver_SPI_1_15_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Keil_CMSIS_Driver_SPI_1_15_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_CMSIS_Driver_SPI_1_15_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_CMSIS_Driver_SPI_1_15_0 PUBLIC
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

# component Keil::Device:STM32Cube Framework:STM32CubeMX@1.1.0
add_library(Keil_Device_STM32Cube_Framework_STM32CubeMX_1_1_0 OBJECT
  "${SOLUTION_ROOT}/RTE/Device/STM32F767ZITx/STCubeGenerated/Src/stm32f7xx_hal_msp.c"
)
target_include_directories(Keil_Device_STM32Cube_Framework_STM32CubeMX_1_1_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
  ${SOLUTION_ROOT}/RTE/Device/STM32F767ZITx
  ${SOLUTION_ROOT}/RTE/Device/STM32F767ZITx/STCubeGenerated/Inc
)
target_compile_definitions(Keil_Device_STM32Cube_Framework_STM32CubeMX_1_1_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_Device_STM32Cube_Framework_STM32CubeMX_1_1_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_Device_STM32Cube_Framework_STM32CubeMX_1_1_0 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)

# component Keil::Device:STM32Cube HAL:CAN@1.3.0
add_library(Keil_Device_STM32Cube_HAL_CAN_1_3_0 OBJECT
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_can.c"
)
target_include_directories(Keil_Device_STM32Cube_HAL_CAN_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Keil_Device_STM32Cube_HAL_CAN_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_Device_STM32Cube_HAL_CAN_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_Device_STM32Cube_HAL_CAN_1_3_0 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
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

# component Keil::Device:STM32Cube HAL:ETH@1.3.0
add_library(Keil_Device_STM32Cube_HAL_ETH_1_3_0 OBJECT
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_eth.c"
)
target_include_directories(Keil_Device_STM32Cube_HAL_ETH_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Keil_Device_STM32Cube_HAL_ETH_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_Device_STM32Cube_HAL_ETH_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_Device_STM32Cube_HAL_ETH_1_3_0 PUBLIC
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

# component Keil::Device:STM32Cube HAL:SPI@1.3.0
add_library(Keil_Device_STM32Cube_HAL_SPI_1_3_0 OBJECT
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_spi.c"
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_spi_ex.c"
)
target_include_directories(Keil_Device_STM32Cube_HAL_SPI_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Keil_Device_STM32Cube_HAL_SPI_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_Device_STM32Cube_HAL_SPI_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_Device_STM32Cube_HAL_SPI_1_3_0 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)

# component Keil::Device:STM32Cube HAL:UART@1.3.0
add_library(Keil_Device_STM32Cube_HAL_UART_1_3_0 OBJECT
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart.c"
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart_ex.c"
)
target_include_directories(Keil_Device_STM32Cube_HAL_UART_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Keil_Device_STM32Cube_HAL_UART_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_Device_STM32Cube_HAL_UART_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_Device_STM32Cube_HAL_UART_1_3_0 PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)

# component Keil::Device:STM32Cube HAL:USART@1.3.0
add_library(Keil_Device_STM32Cube_HAL_USART_1_3_0 OBJECT
  "${CMSIS_PACK_ROOT}/Keil/STM32F7xx_DFP/2.16.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_usart.c"
)
target_include_directories(Keil_Device_STM32Cube_HAL_USART_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Keil_Device_STM32Cube_HAL_USART_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Keil_Device_STM32Cube_HAL_USART_1_3_0 PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Keil_Device_STM32Cube_HAL_USART_1_3_0 PUBLIC
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
  "${SOLUTION_ROOT}/RTE/Device/STM32F767ZITx/startup_stm32f767xx.s"
  "${SOLUTION_ROOT}/RTE/Device/STM32F767ZITx/system_stm32f7xx.c"
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
set_source_files_properties("${SOLUTION_ROOT}/RTE/Device/STM32F767ZITx/startup_stm32f767xx.s" PROPERTIES
  COMPILE_FLAGS "${COMPILE_DEFINITIONS}"
)
