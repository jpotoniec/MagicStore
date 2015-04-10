TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += link_pkgconfig
PKGCONFIG += raptor2 redland
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS += -std=c++11

CONFIG(debug, debug|release) {
QMAKE_CXXFLAGS += -pg
QMAKE_LFLAGS += -pg
}

SOURCES += \
    Compressor.cpp MagicStore.cpp Triple.cpp TreePattern.cpp BinaryTriples.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    BinaryTriples.hpp BinaryTriples.hpp  Compressor.hpp    TreePattern.hpp  Triple.hpp \
    Merger.hpp

