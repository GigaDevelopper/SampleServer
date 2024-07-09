cmake_minimum_required(VERSION 3.5)

project(sampleServer LANGUAGES CXX)

#set cpp standart
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#link boost pakcage
find_package(Boost 1.70.0 REQUIRED COMPONENTS system filesystem program_options)

add_executable(${PROJECT_NAME} main.cpp
    utils/utils.h utils/utils.cpp)

#targets
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})

#recursive add project directories
file(GLOB_RECURSE headers "server/*.h*" "utils/*.h*")
file(GLOB_RECURSE sources "server/*.c*" "utils/*.c*")

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}  ${PROJECT_BINARY_DIR})
target_sources(${PROJECT_NAME} PRIVATE ${sources} ${headers})

#add tests direcotiry
#add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
