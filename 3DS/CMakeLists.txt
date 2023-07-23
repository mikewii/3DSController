cmake_minimum_required(VERSION 3.5)
include(helpers.cmake)

project(3DSController LANGUAGES CXX C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")
file(GLOB_RECURSE PROJECT_UI CONFIGURE_DEPENDS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui")
file(GLOB_RECURSE PROJECT_RESOURCES CONFIGURE_DEPENDS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*.qrc")
set(PROJECT_FILES
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${PROJECT_UI}
    ${PROJECT_RESOURCES}
    Makefile)

add_executable(${PROJECT_NAME} ${PROJECT_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

######################################## 3ds stuff ########################################
if(NOT DEFINED ENV{DEVKITARM})
    message(FATAL_ERROR "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
elseif(NOT DEFINED ENV{DEVKITPRO})
    message(FATAL_ERROR "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro")
elseif(NOT EXISTS $ENV{DEVKITPRO}/libctru)
    message(SEND_ERROR "Please install libctru! https://github.com/devkitPro/libctru")
else()
    target_include_directories(${PROJECT_NAME} PRIVATE $ENV{DEVKITPRO}/libctru/include)
endif()
###########################################################################################

#execute_process(COMMAND ${CMAKE_COMMAND} -E $ENV{DEVKITPRO}/cmake/3ds/arm-none-eabi-cmake)

create_target_directory_groups(${PROJECT_NAME})
