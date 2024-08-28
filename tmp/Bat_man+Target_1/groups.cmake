# groups.cmake

# group STM32CubeMX Common Sources
add_library(Group_STM32CubeMX_Common_Sources OBJECT
  "${SOLUTION_ROOT}/RTE/Device/STM32F767ZITx/STCubeGenerated/Src/main.c"
  "${SOLUTION_ROOT}/RTE/Device/STM32F767ZITx/STCubeGenerated/Src/stm32f7xx_it.c"
)
target_include_directories(Group_STM32CubeMX_Common_Sources PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
  ${SOLUTION_ROOT}/RTE/Device/STM32F767ZITx/STCubeGenerated/Inc
)
target_compile_definitions(Group_STM32CubeMX_Common_Sources PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Group_STM32CubeMX_Common_Sources PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Group_STM32CubeMX_Common_Sources PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)
