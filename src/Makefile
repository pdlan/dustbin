CXX=clang++
INCLUDE=-I ../../recycled/include/ -I ../include/ -I /usr/include/python2.7
LIB=../../recycled/*.a -lpcre -levent -lpython2.7 -lboost_python -ljsoncpp -lssl -lcrypto -ldl
CXXFLAGS=-std=c++11 -Wall -g
all: main.o dustbin.o handlers.o theme.o dustbin
main.o: main.cpp
	$(CXX) $(INCLUDE) $(CXXFLAGS) main.cpp -c
handlers.o: handlers.cpp
	$(CXX) $(INCLUDE) $(CXXFLAGS) handlers.cpp -c
theme.o: theme.cpp
	$(CXX) $(INCLUDE) $(CXXFLAGS) theme.cpp -c
utils.o: utils.cpp
	$(CXX) $(INCLUDE) $(CXXFLAGS) utils.cpp -c
filters.o: filters.cpp
	$(CXX) $(INCLUDE) $(CXXFLAGS) filters.cpp -c
dustbin.o: dustbin.cpp
	$(CXX) $(INCLUDE) $(CXXFLAGS) dustbin.cpp -c
dustbin: main.o dustbin.o handlers.o theme.o utils.o filters.o
	$(CXX) main.o dustbin.o handlers.o theme.o utils.o filters.o -L/usr/lib/i386-linux-gnu $(LIB) -o dustbin
	mv dustbin ../build/
clean:
	rm *.o
