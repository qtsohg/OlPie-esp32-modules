cmake_minimum_required(VERSION 3.16)
project(esp32-modules)
add_library(esp32-modules INTERFACE)
target_include_directories(esp32-modules INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include ${CMAKE_CURRENT_LIST_DIR}/src)
