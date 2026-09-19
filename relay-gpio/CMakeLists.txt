cmake_minimum_required(VERSION 3.16)
project(relay_gpio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_program(MOLD_EXECUTABLE mold)
if(MOLD_EXECUTABLE)
  add_link_options(-fuse-ld=mold)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIODCXX REQUIRED IMPORTED_TARGET libgpiodcxx)

add_executable(relay-gpio
    main.cpp
    configuration.cpp
    relay_controller.cpp
    http_server.cpp
)
target_link_libraries(relay-gpio PRIVATE PkgConfig::GPIODCXX)
target_compile_options(relay-gpio PRIVATE -Wall -Wextra -Wpedantic)

install(TARGETS relay-gpio RUNTIME DESTINATION bin)
install(DIRECTORY public DESTINATION share/relay-gpio)
