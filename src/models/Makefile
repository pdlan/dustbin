CXX=clang++
INCLUDE_MONGO=-I ../../include/
LIB_MONGO=-ljsoncpp -lmongoclient -lboost_thread -lboost_filesystem -lboost_system -lboost_regex -lpthread -lssl -lcrypto
CXXFLAGS=-std=c++11 -Wall -g
mongo: mongo.cpp
	$(CXX) $(INCLUDE_MONGO) $(CXXFLAGS) mongo.cpp $(LIB_MONGO) -shared  -o mongo.so
	mv mongo.so ../../build/models/
clean:
	rm -rf ../../build/models/*