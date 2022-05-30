cmake_minimum_required(VERSION 3.16.0)

list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/lib/esp-idf-lib/components)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Nik-see-idf)
