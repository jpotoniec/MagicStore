TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += link_pkgconfig
PKGCONFIG += raptor2 redland OpenCL
QMAKE_CXXFLAGS += -std=c++11 -D__CL_ENABLE_EXCEPTIONS=1
QMAKE_LFLAGS += -std=c++11

CONFIG(debug, debug|release) {
QMAKE_CXXFLAGS += -pg
QMAKE_LFLAGS += -pg
}

SOURCES += \
    Compressor.cpp MagicStore.cpp Triple.cpp TreePattern.cpp BinaryTriples.cpp \
    GPU.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    BinaryTriples.hpp BinaryTriples.hpp  Compressor.hpp    TreePattern.hpp  Triple.hpp \
    Merger.hpp \
    GPU.hpp \
    FindArgs.h \
    BinaryHelpers.h

KERNELS = kernel.cl
preprocess.name = Preprocess OpenCL kernels
preprocess.input = KERNELS
preprocess.output = ${QMAKE_FILE_BASE}.cpp
preprocess.commands = ${PWD}/compile_kernel.sh ${QMAKE_FILE_IN} ${QMAKE_FILE_BASE}.cpp
preprocess.variable_out = SOURCES
QMAKE_EXTRA_COMPILERS += preprocess
