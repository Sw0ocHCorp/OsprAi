cmake_minimum_required(VERSION 3.10)

project(OsprAiCompanion LANGUAGES CXX)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option 1: Explicitly list the sources
set(SOURCES
    src/CustomCompanion.cpp
)

# Option 2 (optional): Automatically collect all .cpp in src
# file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE gpiod pthread)

# Add include directory for header files (*.hpp)
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)