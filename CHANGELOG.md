# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).


## [Unreleased]

### Added

### Changed

### Removed


## QtBasemapPlugin 1.1.0-2

### Changed
- used conan-package for Qt-dependency
- minor changes in recipe build configuration


## QtBasemapPlugin 1.1.0-1

### Added
- new "Custom"-MapType option which fetches map tiles from a custom url. When [Map QML Type](https://doc.qt.io/qt-5/qml-qtlocation-map.html) is used,
the new map type can be chosen as [activeMapType](https://doc.qt.io/qt-5/qml-qtlocation-map.html#activeMapType-prop).

### Changed
- if [plugin](https://doc.qt.io/qt-5/qml-qtlocation-plugin.html) is instantiated with custom url parameter, the switch between Satellite, Street (provided by mapbox) 
and Custom tiles (provided via custom basemap url) can be done without plugin [reinstantiation](https://doc.qt.io/qt-5/qml-qtlocation-map.html#plugin-prop)


## QtBasemapPlugin 2.0.0-1

### Changed
- QtBasemapPlugin is being upgraded and based on Qt 6.6.0-1 now.