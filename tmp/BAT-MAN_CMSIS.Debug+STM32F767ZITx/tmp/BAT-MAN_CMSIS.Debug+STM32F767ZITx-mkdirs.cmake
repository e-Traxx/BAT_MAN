# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/akame/Documents/Etraxx/BAT-MAN/tmp/BAT-MAN_CMSIS.Debug+STM32F767ZITx"
  "/Users/akame/Documents/Etraxx/BAT-MAN/tmp/1"
  "/Users/akame/Documents/Etraxx/BAT-MAN/tmp/BAT-MAN_CMSIS.Debug+STM32F767ZITx"
  "/Users/akame/Documents/Etraxx/BAT-MAN/tmp/BAT-MAN_CMSIS.Debug+STM32F767ZITx/tmp"
  "/Users/akame/Documents/Etraxx/BAT-MAN/tmp/BAT-MAN_CMSIS.Debug+STM32F767ZITx/src/BAT-MAN_CMSIS.Debug+STM32F767ZITx-stamp"
  "/Users/akame/Documents/Etraxx/BAT-MAN/tmp/BAT-MAN_CMSIS.Debug+STM32F767ZITx/src"
  "/Users/akame/Documents/Etraxx/BAT-MAN/tmp/BAT-MAN_CMSIS.Debug+STM32F767ZITx/src/BAT-MAN_CMSIS.Debug+STM32F767ZITx-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/akame/Documents/Etraxx/BAT-MAN/tmp/BAT-MAN_CMSIS.Debug+STM32F767ZITx/src/BAT-MAN_CMSIS.Debug+STM32F767ZITx-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/akame/Documents/Etraxx/BAT-MAN/tmp/BAT-MAN_CMSIS.Debug+STM32F767ZITx/src/BAT-MAN_CMSIS.Debug+STM32F767ZITx-stamp${cfgdir}") # cfgdir has leading slash
endif()
