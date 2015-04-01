TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += link_pkgconfig
PKGCONFIG += raptor2 redland
QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    Compressor.cpp MagicStore.cpp Triple.cpp Query.cpp TreePattern.cpp BinaryTriples.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    BinaryTriples.hpp BinaryTriples.hpp  Compressor.hpp  Data.hpp  Query.hpp  TreePattern.hpp  Triple.hpp

