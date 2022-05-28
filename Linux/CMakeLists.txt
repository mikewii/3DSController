cmake_minimum_required(VERSION 3.5)

project(3DSController)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/src/*.c ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB PROJECT_HEADERS ${PROJECT_SOURCE_DIR}/src/*.h ${PROJECT_SOURCE_DIR}/src/*.hpp)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS} ${PROJECT_NAME}.service)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_options(${PROJECT_NAME} PUBLIC -static -static-libgcc -static-libstdc++)

install(TARGETS ${PROJECT_NAME}
    DESTINATION bin
    COMPONENT binaries)
install(FILES ${PROJECT_NAME}.service
    DESTINATION /etc/systemd/user/
    COMPONENT config)
