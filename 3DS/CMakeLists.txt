cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME "3DSController")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/src/*.c ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB PROJECT_HEADERS ${PROJECT_SOURCE_DIR}/src/*.h ${PROJECT_SOURCE_DIR}/src/*.hpp)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)

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
