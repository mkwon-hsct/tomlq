##+++++++++++++++++++++++++++++++++++++++++++++++++++++++##
##                    Initial Setting                    ##
##+++++++++++++++++++++++++++++++++++++++++++++++++++++++##

## CMake Version
cmake_minimum_required(VERSION 3.1)

## Project Name
project(tomlq C)

## Set default build type "Release"
if("" STREQUAL "${CMAKE_BUILD_TYPE}")
  set(CMAKE_BUILD_TYPE Release)
endif()

## Set default install directory ${QHOME}/[os]
if (UNIX)
  set(OS_FLAG l)
else()
  message( FATAL_ERROR "tomlq is supported only for UNIX." )
endif()

if("/usr/local" STREQUAL "${CMAKE_INSTALL_PREFIX}")
  set(CMAKE_INSTALL_PREFIX "$ENV{QHOME}/${OS_FLAG}64")
endif()

##+++++++++++++++++++++++++++++++++++++++++++++++++++++++##
##                       Pre-Build                       ##
##+++++++++++++++++++++++++++++++++++++++++++++++++++++++##

## Download k.h
file(DOWNLOAD "https://github.com/KxSystems/kdb/raw/master/c/c/k.h" "${PROJECT_SOURCE_DIR}/include/k.h")

## Download tomlc99 files
file(DOWNLOAD "https://raw.githubusercontent.com/cktan/tomlc99/master/toml.h" "${PROJECT_SOURCE_DIR}/include/toml.h")
file(DOWNLOAD "https://raw.githubusercontent.com/cktan/tomlc99/master/toml.c" "${PROJECT_SOURCE_DIR}/src/toml.c")

##+++++++++++++++++++++++++++++++++++++++++++++++++++++++##
##                         Build                         ##
##+++++++++++++++++++++++++++++++++++++++++++++++++++++++##

## Source files
add_library(${PROJECT_NAME} SHARED
  src/toml.c
  src/tomlq.c
)

## Include directory
target_include_directories(${PROJECT_NAME} PRIVATE
  ${PROJECT_SOURCE_DIR}/include
)

## C standard
target_compile_features(${PROJECT_NAME} PUBLIC c_std_99)

## Other flags
target_compile_options(${PROJECT_NAME} PRIVATE
  -DKXVER=3
)

## Trim "lib" prefix from the artefact
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

##+++++++++++++++++++++++++++++++++++++++++++++++++++++++##
##                         Install                       ##
##+++++++++++++++++++++++++++++++++++++++++++++++++++++++##

## Install the artefact to ${CMAKE_INSTALL_PREFIX}
install(TARGETS ${PROJECT_NAME} DESTINATION .)
