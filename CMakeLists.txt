cmake_minimum_required(VERSION 3.31)

project(ProjetCPP)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(simulation 
    src/Avion.cpp
    src/main.cpp
)

target_include_directories(simulation PRIVATE include)
