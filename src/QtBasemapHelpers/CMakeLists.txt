# Note: these helper classes are not related to mapbox plugin but rather to QtLocation, it is better to 
# create a separate public repository for them when we have many


cmake_minimum_required(VERSION 3.17)
project(QtBasemapHelpers VERSION 1.0.0 LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_set_find_paths()

set(HELPERS_PACKAGE_NAME QtBasemapHelpers)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(Qt6 COMPONENTS
    Core
    Location
    REQUIRED QUIET)

set(BASEMAP_HELPERS_SOURCES
    include/QtBasemapHelpers/QQuickGeoMapGestureArea.h
    src/QQuickGeoMapGestureArea.cpp
)

qt_add_library(${HELPERS_PACKAGE_NAME} SHARED ${BASEMAP_HELPERS_SOURCES})

target_compile_features(${HELPERS_PACKAGE_NAME} PRIVATE cxx_std_14)

set_target_properties(${HELPERS_PACKAGE_NAME} PROPERTIES AUTORCC ON AUTOMOC ON)

target_include_directories(${HELPERS_PACKAGE_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/QtBasemapHelpers>
)

install(TARGETS ${HELPERS_PACKAGE_NAME}
    EXPORT ${HELPERS_PACKAGE_NAME}Config
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)

target_link_libraries(${HELPERS_PACKAGE_NAME} PUBLIC
    Qt6::Core
    Qt6::Location
    Qt6::LocationPrivate
)

install(DIRECTORY include/QtBasemapHelpers DESTINATION include)

install(EXPORT ${HELPERS_PACKAGE_NAME}Config
    DESTINATION share/${HELPERS_PACKAGE_NAME}/cmake
)
