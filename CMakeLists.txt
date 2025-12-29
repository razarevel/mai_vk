cmake_minimum_required(VERSION 3.23)

project(mai LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(MAI_BUILD_STATIC "Build mai as static library" ON)

if (MAI_BUILD_STATIC)
    set(MAI_LIB_TYPE STATIC)
else()
    set(MAI_LIB_TYPE SHARED)
endif()

add_subdirectory(thirdParty/glfw)
find_package(Vulkan REQUIRED)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(mai ${MAI_LIB_TYPE} ${MY_SOURCES})
add_library(mai::mai ALIAS mai)

target_include_directories(mai
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(mai
    PUBLIC
        glfw
        Vulkan::Vulkan
)

