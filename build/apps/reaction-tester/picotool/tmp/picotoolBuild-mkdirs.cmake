# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/andrea/pico/pico-reaction-tester/build/_deps/picotool-src"
  "/home/andrea/pico/pico-reaction-tester/build/_deps/picotool-build"
  "/home/andrea/pico/pico-reaction-tester/build/_deps"
  "/home/andrea/pico/pico-reaction-tester/build/apps/reaction-tester/picotool/tmp"
  "/home/andrea/pico/pico-reaction-tester/build/apps/reaction-tester/picotool/src/picotoolBuild-stamp"
  "/home/andrea/pico/pico-reaction-tester/build/apps/reaction-tester/picotool/src"
  "/home/andrea/pico/pico-reaction-tester/build/apps/reaction-tester/picotool/src/picotoolBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/andrea/pico/pico-reaction-tester/build/apps/reaction-tester/picotool/src/picotoolBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/andrea/pico/pico-reaction-tester/build/apps/reaction-tester/picotool/src/picotoolBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
