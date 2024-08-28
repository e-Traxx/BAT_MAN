# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/akame/Documents/BAT-MAN/tmp/Bat_man+Target_1"
  "/Users/akame/Documents/BAT-MAN/tmp/1"
  "/Users/akame/Documents/BAT-MAN/tmp/Bat_man+Target_1"
  "/Users/akame/Documents/BAT-MAN/tmp/Bat_man+Target_1/tmp"
  "/Users/akame/Documents/BAT-MAN/tmp/Bat_man+Target_1/src/Bat_man+Target_1-stamp"
  "/Users/akame/Documents/BAT-MAN/tmp/Bat_man+Target_1/src"
  "/Users/akame/Documents/BAT-MAN/tmp/Bat_man+Target_1/src/Bat_man+Target_1-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/akame/Documents/BAT-MAN/tmp/Bat_man+Target_1/src/Bat_man+Target_1-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/akame/Documents/BAT-MAN/tmp/Bat_man+Target_1/src/Bat_man+Target_1-stamp${cfgdir}") # cfgdir has leading slash
endif()
