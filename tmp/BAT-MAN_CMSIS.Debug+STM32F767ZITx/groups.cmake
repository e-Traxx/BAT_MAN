# groups.cmake

# group Source Files
add_library(Group_Source_Files OBJECT
  "${SOLUTION_ROOT}/BAT-MAN_CMSIS/main.c"
)
target_include_directories(Group_Source_Files PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Group_Source_Files PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Group_Source_Files PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Group_Source_Files PUBLIC
  ${CONTEXT}_ABSTRACTIONS
)

# group Inc
add_library(Group_Source_Files_Inc INTERFACE)
target_include_directories(Group_Source_Files_Inc INTERFACE
  $<TARGET_PROPERTY:Group_Source_Files,INTERFACE_INCLUDE_DIRECTORIES>
  ${SOLUTION_ROOT}/BAT-MAN_CMSIS/Inc
)
target_compile_definitions(Group_Source_Files_Inc INTERFACE
  $<TARGET_PROPERTY:Group_Source_Files,INTERFACE_COMPILE_DEFINITIONS>
)

# group Inc_c
add_library(Group_Source_Files_Inc_c OBJECT
  "${SOLUTION_ROOT}/BAT-MAN_CMSIS/Inc_c/uart_handler.c"
  "${SOLUTION_ROOT}/BAT-MAN_CMSIS/Inc_c/can_handler.c"
  "${SOLUTION_ROOT}/BAT-MAN_CMSIS/Inc_c/spi_handler.c"
  "${SOLUTION_ROOT}/BAT-MAN_CMSIS/Inc_c/can_vector_handler.c"
  "${SOLUTION_ROOT}/BAT-MAN_CMSIS/Inc_c/current_handler.c"
  "${SOLUTION_ROOT}/BAT-MAN_CMSIS/Inc_c/can_rx.c"
)
target_include_directories(Group_Source_Files_Inc_c PUBLIC
  $<TARGET_PROPERTY:Group_Source_Files,INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Group_Source_Files_Inc_c PUBLIC
  $<TARGET_PROPERTY:Group_Source_Files,INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(Group_Source_Files_Inc_c PUBLIC
  $<TARGET_PROPERTY:Group_Source_Files,INTERFACE_COMPILE_OPTIONS>
)
