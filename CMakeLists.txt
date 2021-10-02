cmake_minimum_required(VERSION 3.10)

# set the project name
set (CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "-O2 -std=c++11")

set(CMAKE_EXE_LINKER_FLAGS "-v")

project(2D-Vulkan-Environment VERSION 1.0)


#packages
find_package(Vulkan REQUIRED)

if (UNIX)
    find_package(X11 REQUIRED)
    link_libraries(${X11_LIBRARIES})
    add_definitions(-DVK_USE_PLATFORM_XLIB_KHR)
    find_package(glfw3 3.3 REQUIRED)

    find_package(Freetype REQUIRED)
endif()


#add executables
file(GLOB SOURCES src/*)
file(GLOB FRAMEWORK src/framework/*)
file(GLOB GAME src/game/*)
add_executable(2D-Vulkan-Environment ${SOURCES}
                                    ${FRAMEWORK}
                                    ${GAME})



#link
if(UNIX)
    target_include_directories(2D-Vulkan-Environment PUBLIC ${GLFW_INCLUDE_DIRS}
                                                 PUBLIC ${VULKAN_INCLUDE_DIRS}
                                                 PUBLIC ${FREETYPE_INCLUDE_DIRS}
                                                 PUBLIC ${PROJECT_BINARY_DIR})

    target_link_libraries(2D-Vulkan-Environment glfw)
    target_link_libraries(2D-Vulkan-Environment Vulkan::Vulkan)
    target_link_libraries(2D-Vulkan-Environment ${FREETYPE_LIBRARIES})
else()
    target_include_directories(2D-Vulkan-Environment PUBLIC D:/noam4/Libraries/VS/include)
    target_link_libraries(2D-Vulkan-Environment libs/glfw3)
    target_link_libraries(2D-Vulkan-Environment libs/freetyped)
    target_link_libraries(2D-Vulkan-Environment libs/vulkan-1)
endif()
