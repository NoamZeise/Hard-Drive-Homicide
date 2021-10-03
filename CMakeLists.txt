cmake_minimum_required(VERSION 3.10)

# set the project name
set (CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "-O2 -std=c++11")

set(CMAKE_EXE_LINKER_FLAGS "-v")

project(Hard-Drive-Homicide VERSION 1.0)



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
add_executable(Hard-Drive-Homicide ${SOURCES}
                                    ${FRAMEWORK}
                                    ${GAME}
                                    icon.rc)


set_target_properties(Hard-Drive-Homicide PROPERTIES OUTPUT_NAME "Hard-Drive-Homicide")
#link
if(UNIX)
    target_include_directories(Hard-Drive-Homicide PUBLIC ${GLFW_INCLUDE_DIRS}
                                                 PUBLIC ${VULKAN_INCLUDE_DIRS}
                                                 PUBLIC ${FREETYPE_INCLUDE_DIRS}
                                                 PUBLIC ${PROJECT_BINARY_DIR})

    target_link_libraries(Hard-Drive-Homicide glfw)
    target_link_libraries(Hard-Drive-Homicide Vulkan::Vulkan)
    target_link_libraries(Hard-Drive-Homicide ${FREETYPE_LIBRARIES})
else()
    target_include_directories(Hard-Drive-Homicide PUBLIC D:/noam4/Libraries/VS/include)
    target_link_libraries(Hard-Drive-Homicide libs/glfw3)
    target_link_libraries(Hard-Drive-Homicide Winmm)
    target_link_libraries(Hard-Drive-Homicide libs/freetyped)
    target_link_libraries(Hard-Drive-Homicide libs/vulkan-1)
endif()
