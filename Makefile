SRC=Compressor.cpp MagicStore.cpp Triple.cpp Query.cpp
TESTS= BinaryCodeTest.cpp
OBJECTS=$(SRC:%.cpp=%.o)
CXX=g++
CC=g++
CPPFLAGS=-std=c++11 `redland-config --cflags` `pkg-config --cflags raptor2` -Wall
LDFLAGS=`redland-config --libs` `pkg-config --libs raptor2`

all: .depends MagicStore

MagicStore: $(OBJECTS)

BinaryCodeTest: LDFLAGS += -lboost_unit_test_framework
BinaryCodeTest: Compressor.o BinaryCodeTest.o

.depends:
	$(CXX) $(CPPFLAGS) -M $(SRC) $(TESTS) >.depends

.PHONY: all .depends clean

clean:
	rm -f $(OBJECTS) MagicStore

$(include .depends)
