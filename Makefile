SRC=MagicStore.cpp
OBJECTS=$(SRC:%.cpp=%.o)
CXX=g++
CC=g++
CPPFLAGS=-std=c++11 `redland-config --cflags` `pkg-config --cflags raptor2` -Wall
LDFLAGS=`redland-config --libs` `pkg-config --libs raptor2`

all: MagicStore

MagicStore: $(OBJECTS)
