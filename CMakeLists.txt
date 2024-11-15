# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS components/esp32-wifi-manager)
include($ENV{ADF_PATH}/CMakeLists.txt)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

idf_build_set_property(PARTITION_TABLE_FILENAME "partitions.csv")

project(audio-player-esp32-a1s)

include_directories($ENV{ADF_PATH}/components)
include_directories($ENV{ADF_PATH}/include)