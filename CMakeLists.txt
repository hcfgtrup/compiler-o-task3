cmake_minimum_required(VERSION 3.14)
project(Compiler)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Compiler 
    main.cpp 
    parser/Parser.cpp
    sourceCodeDriver/Driver.cpp
    error/Error.cpp
    scanner/Scanner.cpp
    table/TableOfName.cpp
    virtualMachine/ovm.cpp
    virtualMachine/generateCode.cpp
)

target_include_directories(Compiler PRIVATE ${PROJECT_SOURCE_DIR}/parser/)
target_include_directories(Compiler PRIVATE ${PROJECT_SOURCE_DIR}/error/)
target_include_directories(Compiler PRIVATE ${PROJECT_SOURCE_DIR}/sourceCodeDriver/)
target_include_directories(Compiler PRIVATE ${PROJECT_SOURCE_DIR}/scanner/)
target_include_directories(Compiler PRIVATE ${PROJECT_SOURCE_DIR}/table/)
target_include_directories(Compiler PRIVATE ${PROJECT_SOURCE_DIR}/virtualMachine/)