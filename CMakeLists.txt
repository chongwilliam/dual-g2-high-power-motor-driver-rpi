cmake_minimum_required(VERSION 3.5)
project(pimotor VERSION 1.0.0)

## Change to Release 
set(DEFAULT_BUILD_TYPE "Debug")  
set(CMAKE_C_FLAGS "-O3 -I/usr/local/include")
# set(CMAKE_CXX_FLAGS "-std=c++17 -O3 -I/usr/local/include")
# set(CMAKE_VERBOSE_MAKEFILE ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

include(GNUInstallDirs)

# Hiredis
find_library(HIREDIS_LIBRARY hiredis)

# Pigpio
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
find_package(pigpio REQUIRED)
include_directories(${pigpio_INCLUDE_DIR})
# target_link_libraries(${PROJECT_NAME} ${pigpio_LIBRARY} ${pigpiod_if_LIBRARY} ${pigpiod_if2_LIBRARY})

# Collect all source files 
set(SOURCE_FILES    ${CMAKE_CURRENT_SOURCE_DIR}/src/encoder.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/motor.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/filter.c)

# Directories for application or test 
# add_subdirectory(apps)
add_subdirectory(test)