cmake_minimum_required(VERSION 3.16)

project(ProjetCPP)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SFML_STATIC_LIBRARIES TRUE)

if(MSVC)
    set(SFML_DIR "C:/SFML_3.0.2/lib/cmake/SFML")
endif(MSVC)

    set(DEFS "PATH_IMG=\"${CMAKE_CURRENT_SOURCE_DIR}/img/\"")
    add_compile_definitions(${DEFS})

find_package(SFML 3 COMPONENTS Window Graphics System REQUIRED)
link_libraries(SFML::Graphics SFML::Window SFML::System)


add_executable(ProjetCPP 
    src/Avion.cpp
    src/main.cpp
 "include/Trajectoire.h" "src/Trajectoire.cpp")

