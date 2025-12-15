cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(mai)

add_subdirectory(thirdParty/glfw) 

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(${PROJECT_NAME} ${MY_SOURCES})

target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC SHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders/")
set(VULKAN_PATH $ENV{VK_SDK_PATH})

target_include_directories(${PROJECT_NAME} PUBLIC 
	"${CMAKE_CURRENT_SOURCE_DIR}/include/"
	${VULKAN_PATH}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw "${VULKAN_PATH}/Lib/vulkan-1.lib")
