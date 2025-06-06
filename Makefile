CXX = g++
CXXFLAGS = -std=c++20 -Wall -pedantic -pthread -O3
LDFLAGS = -pthread
SRC = src/main.cpp

EXE ?= Sigmoid

.PHONY: all clean

all: $(EXE)

$(EXE): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(EXE) $(LDFLAGS)

clean:
	rm -f $(EXE)
