cmake_minimum_required(VERSION 3.10)
project(QtBasemapPlugin VERSION 1.0.0 LANGUAGES CXX)

set(VERSION_STRING "1.0.0" CACHE STRING "Package version")
set(LIB_NAME qtgeoservices_basemap_pix4d)
set(PACKAGE_NAME QtBasemapPlugin)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_set_find_paths()

set(PIX4D_BUILDTOOLS_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pix4d_build_tools/cmake)
set(PIX4D_BUILDTOOLS_GITHOOKS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pix4d_build_tools/git_hooks)

include(${PIX4D_BUILDTOOLS_CMAKE_DIR}/detect_conan_updated.cmake REQUIRED)

# Project settings
include(${PIX4D_BUILDTOOLS_CMAKE_DIR}/globalOptions.cmake        REQUIRED)
# Common cmake configuration and helpers
include(${PIX4D_BUILDTOOLS_CMAKE_DIR}/cmakeDefinitions.cmake     REQUIRED)
include(${PIX4D_BUILDTOOLS_CMAKE_DIR}/installOptions.cmake       REQUIRED)


if (NOT ${BUILD_SHARED_LIBS})
    message(STATUS " ***** Build plugin as static library *****")
    set(DEFINES "-DQT_STATICPLUGIN -DQT_PLUGIN")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${DEFINES}")
    set(CXXFLAGS "${CXXFLAGS} ${DEFINES}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEFINES}")
endif()

find_package(Qt5 COMPONENTS
    Core
    Gui
    OpenGL
    Quick
    Xml
    Positioning
    Location 
    Network 
    REQUIRED QUIET)

set(SOURCES
   src/GeoServiceProviderPlugin.cpp
   src/GeoTiledMappingManagerEngine.cpp
   src/GeoTileFetcher.cpp
   src/GeoMapReply.cpp
   src/GeoFileTileCache.cpp
   
   src/GeoServiceProviderPlugin.h
   src/GeoTiledMappingManagerEngine.h
   src/GeoTileFetcher.h
   src/GeoMapReply.h
   src/GeoFileTileCache.h
)

add_library(${LIB_NAME} ${SOURCES} )
set_target_properties(${LIB_NAME} PROPERTIES AUTORCC ON AUTOMOC ON)

target_link_libraries(${LIB_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Quick
    Qt5::Positioning
    Qt5::Xml
    Qt5::Location
    Qt5::LocationPrivate
    Qt5::Network
    Qt5::OpenGL
)

target_include_directories(${LIB_NAME} INTERFACE
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
   PRIVATE src
   ${Qt5Location_PRIVATE_INCLUDE_DIRS}
)

target_compile_features(${LIB_NAME} PRIVATE cxx_std_14)

include(${PIX4D_BUILDTOOLS_CMAKE_DIR}/componentDefinitions.cmake REQUIRED)
include(${PIX4D_BUILDTOOLS_CMAKE_DIR}/packaging.cmake            REQUIRED)

install(TARGETS ${LIB_NAME} EXPORT ${PACKAGE_NAME}Config
  LIBRARY DESTINATION plugins/geoservices
  RUNTIME DESTINATION plugins/geoservices
  ARCHIVE DESTINATION lib
)

install(EXPORT ${PACKAGE_NAME}Config DESTINATION share/${PACKAGE_NAME}/cmake)
