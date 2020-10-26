cmake_minimum_required(VERSION 3.12)
project(QtBasemapPlugin VERSION 1.0.0 LANGUAGES CXX)

set(LIB_NAME qtgeoservices_basemap_pix4d)
set(PACKAGE_NAME QtBasemapPlugin)

if (NOT ${BUILD_SHARED_LIBS})
    message(STATUS " ***** Build plugin as static library *****")
    set(DEFINES "-DQT_STATICPLUGIN -DQT_PLUGIN")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${DEFINES}")
    set(CXXFLAGS "${CXXFLAGS} ${DEFINES}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEFINES}")
endif()

find_package(Qt5 COMPONENTS
    Core
    Location
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
    Qt5::Location
    Qt5::LocationPrivate
)

target_include_directories(${LIB_NAME} INTERFACE
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_compile_features(${LIB_NAME} PRIVATE cxx_std_14)

install(TARGETS ${LIB_NAME} EXPORT ${PACKAGE_NAME}Config
  LIBRARY DESTINATION plugins/geoservices
  RUNTIME DESTINATION plugins/geoservices
  ARCHIVE DESTINATION lib
)

install(EXPORT ${PACKAGE_NAME}Config DESTINATION share/${PACKAGE_NAME}/cmake)
