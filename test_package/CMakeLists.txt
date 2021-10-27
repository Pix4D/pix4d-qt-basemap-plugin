cmake_minimum_required(VERSION 3.12)
project(PackageTest CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_set_find_paths()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

find_package(Qt5 COMPONENTS
    Core
    Location
    REQUIRED QUIET)
find_package(QtBasemapPlugin REQUIRED QUIET)

qt5_add_resources(IMAGES_RCC "res/mapbox.qrc")

add_executable(example example.cpp ${IMAGES_RCC})
get_property(MAP_PLUGIN TARGET qtgeoservices_basemap_pix4d PROPERTY IMPORTED_LOCATION_RELEASE)
message(STATUS "Map plugin path ${MAP_PLUGIN}")

set(MAP_PLUGIN_DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins/geoservices)
file(MAKE_DIRECTORY ${MAP_PLUGIN_DESTINATION})
file(COPY ${MAP_PLUGIN} DESTINATION ${MAP_PLUGIN_DESTINATION})

target_link_libraries(example PRIVATE
    Qt5::Core
    Qt5::Location
)
# don't link with it because this is needed only by the Qt runtime to give a full path to the folder containing it
#target_link_libraries(example PRIVATE
#    qtgeoservices_basemap_pix4d
#)

