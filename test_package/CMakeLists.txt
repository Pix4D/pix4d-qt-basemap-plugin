project(PackageTest CXX)
cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_STANDARD_REQUIRED ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(QtBasemapPlugin)

add_executable(example example.cpp)
get_property(_libFileName TARGET qtgeoservices_basemap_pix4d PROPERTY IMPORTED_LOCATION_RELEASE)

if(EXISTS ${_libFileName})
    message(STATUS "Plugin generated at: ${_libFileName}")
else()
    message(FATAL_ERROR "Plugin not generated at: ${_libFileName}")
endif()

if (${CMAKE_INSTALL_PREFIX} STREQUAL "")
    set(DEPLOY_TARGET ${CMAKE_BINARY_DIR})
else()
    set(DEPLOY_TARGET ${CMAKE_INSTALL_PREFIX})
endif()

set(DEPLOY_TARGET ${CMAKE_BINARY_DIR})
set(MAP_PLUGIN_DESTINATION ${DEPLOY_TARGET}/Plugins/geoservices)
set(MAP_PLUGIN ${_libFileName})

message(STATUS "Map plugin path ${MAP_PLUGIN}")

file(MAKE_DIRECTORY ${MAP_PLUGIN_DESTINATION})
file(COPY ${MAP_PLUGIN} DESTINATION ${MAP_PLUGIN_DESTINATION})


# don't link with it because this is needed only by the Qt runtime to give a full path to the folder containing it
#target_link_libraries(example PRIVATE
#    qtgeoservices_basemap_pix4d
#)
            
