cmake_minimum_required(VERSION 3.10)

# set the project name
project(2D-Vulkan-Environment VERSION 1.0)

set (CMAKE_CXX_STANDARD 11)

#packages
find_package(Vulkan REQUIRED)

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})

add_definitions(-DVK_USE_PLATFORM_XLIB_KHR)

find_package(glfw3 3.3 REQUIRED)

find_package(Freetype REQUIRED)

SET(CMAKE_CXX_FLAGS "-O2 -std=c++11")

SET(CMAKE_EXE_LINKER_FLAGS "-v")


#add executables
file(GLOB SOURCES src/*)
add_executable(2D-Vulkan-Environment ${SOURCES})


#link
target_include_directories(2D-Vulkan-Environment PUBLIC ${GLFW_INCLUDE_DIRS}
                                                 PUBLIC ${VULKAN_INCLUDE_DIRS}
                                                 PUBLIC ${FREETYPE_INCLUDE_DIRS}
                                                 PUBLIC ${PROJECT_BINARY_DIR})

target_link_libraries(2D-Vulkan-Environment glfw)
target_link_libraries(2D-Vulkan-Environment Vulkan::Vulkan)
target_link_libraries(2D-Vulkan-Environment ${FREETYPE_LIBRARIES})
