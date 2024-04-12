cmake_minimum_required(VERSION 3.21)
project(PackageTest CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_set_find_paths()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

find_package(Qt6 COMPONENTS
    Core
    Positioning
    Location
    REQUIRED QUIET)

find_package(QtBasemapHelpers REQUIRED QUIET)
find_package(QtBasemapPlugin REQUIRED QUIET)


qt_add_executable(example example.cpp)

set_target_properties(example PROPERTIES AUTORCC ON AUTOMOC ON)

get_property(MAP_PLUGIN TARGET qtgeoservices_basemap_pix4d PROPERTY IMPORTED_LOCATION_RELEASE)
message(STATUS "Map plugin path ${MAP_PLUGIN}")

set(MAP_PLUGIN_DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins/geoservices)
file(MAKE_DIRECTORY ${MAP_PLUGIN_DESTINATION})
file(COPY ${MAP_PLUGIN} DESTINATION ${MAP_PLUGIN_DESTINATION})

target_link_libraries(example PRIVATE
    Qt6::Core
    Qt6::Location
    Qt6::LocationPrivate
    QtBasemapHelpers
)
# don't link with it because this is needed only by the Qt runtime to give a full path to the folder containing it
#target_link_libraries(example PRIVATE
#    qtgeoservices_basemap_pix4d
#)

