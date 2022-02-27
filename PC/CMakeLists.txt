cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME "3DSController-PC")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/source/*.c)
file(GLOB PROJECT_HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS})

target_link_libraries(${PROJECT_NAME} PUBLIC ws2_32 Gdi32 gdiplus "${PROJECT_SOURCE_DIR}/lib/vJoyInterface.lib")
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_options(${PROJECT_NAME} PUBLIC -static -static-libgcc -static-libstdc++)

add_compile_options(-fms-extensions)


set(DLL_SRC "${PROJECT_SOURCE_DIR}/lib/vJoyInterface.dll")
set(DLL_DST "${CMAKE_CURRENT_BINARY_DIR}/vJoyInterface.dll")
configure_file(${DLL_SRC} ${DLL_DST} COPYONLY)
