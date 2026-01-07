cmake_minimum_required(VERSION 3.12...3.26)

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT is_multi_config AND NOT (CMAKE_BUILD_TYPE OR DEFINED ENV{CMAKE_BUILD_TYPE}))
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Release default")
endif()

project(tinyrenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(iwyu "Run include-what-you-use")
if(iwyu)
  find_program(IWYU_EXE NAMES include-what-you-use REQUIRED)
  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_EXE})
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU|Intel")
  add_compile_options(-Wall -O3)
endif()

find_package(OpenMP COMPONENTS CXX)
find_package(OpenCV REQUIRED)

file(GLOB SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    ${OpenCV_INCLUDE_DIRS}       
)

target_link_libraries(${PROJECT_NAME} PRIVATE $<$<BOOL:${OpenMP_CXX_FOUND}>:OpenMP::OpenMP_CXX>)
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
