cmake_minimum_required(VERSION 3.23)

project(mai LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(thirdParty/glfw)
add_subdirectory(thirdParty/assimp)
find_package(Vulkan REQUIRED)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(${PROJECT_NAME} ${MY_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/"
    SHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders/"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
		assimp
    Vulkan::Vulkan
)

