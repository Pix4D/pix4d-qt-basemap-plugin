cmake_minimum_required(VERSION 3.12)
project(QtBasemapPlugin VERSION 1.0.0 LANGUAGES CXX)

set(LIB_NAME qtgeoservices_basemap_pix4d)
set(PACKAGE_NAME QtBasemapPlugin)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

if (NOT ${BUILD_SHARED_LIBS})
    message(STATUS " ***** Build plugin as static library *****")
    set(DEFINES "-DQT_STATICPLUGIN -DQT_PLUGIN")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${DEFINES}")
    set(CXXFLAGS "${CXXFLAGS} ${DEFINES}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEFINES}")
endif()

find_package(Qt6 COMPONENTS
    Core
    Positioning
    Location
    REQUIRED QUIET)

set(SOURCES
   src/qgeoserviceproviderpluginmapbox.cpp 
   src/qgeoserviceproviderpluginmapbox.h
   src/qgeotiledmappingmanagerenginemapbox.cpp 
   src/qgeotiledmappingmanagerenginemapbox.h
   src/qgeotilefetchermapbox.cpp 
   src/qgeotilefetchermapbox.h
   src/qgeomapreplymapbox.cpp 
   src/qgeomapreplymapbox.h
   src/qgeofiletilecachemapbox.cpp 
   src/qgeofiletilecachemapbox.h
   src/qgeoroutingmanagerenginemapbox.cpp 
   src/qgeoroutingmanagerenginemapbox.h
   src/qgeoroutereplymapbox.cpp 
   src/qgeoroutereplymapbox.h
   src/qplacecategoriesreplymapbox.cpp 
   src/qplacecategoriesreplymapbox.h
   src/qplacemanagerenginemapbox.cpp 
   src/qplacemanagerenginemapbox.h
   src/qplacesearchsuggestionreplymapbox.cpp 
   src/qplacesearchsuggestionreplymapbox.h
   src/qplacesearchreplymapbox.cpp 
   src/qplacesearchreplymapbox.h
   src/qgeocodingmanagerenginemapbox.cpp 
   src/qgeocodingmanagerenginemapbox.h
   src/qgeocodereplymapbox.cpp 
   src/qgeocodereplymapbox.h
   src/qmapboxcommon.cpp 
   src/qmapboxcommon.h
)

qt_add_plugin(${LIB_NAME} SHARED ${SOURCES} )

set_target_properties(${LIB_NAME} PROPERTIES AUTORCC ON AUTOMOC ON)

target_link_libraries(${LIB_NAME} PRIVATE
    Qt6::Core
    Qt6::Location
    Qt6::LocationPrivate
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
