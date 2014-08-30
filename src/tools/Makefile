CXX=clang++
INCLUDE=-I ../../include/
LIB=-ljsoncpp -lssl -lcrypto -ldl
CXXFLAGS=-std=c++11 -Wall -g
all: tools
tools: tools.cpp
	$(CXX) $(INCLUDE) $(CXXFLAGS) $(LIB) tools.cpp -o tools
	mv tools ../../build/
clean:
	rm ../../build/tools