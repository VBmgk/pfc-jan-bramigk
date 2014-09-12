CXXFLAGS := -std=c++11 -I/usr/local/include -L/usr/local/lib `pkg-config --cflags --libs protobuf` -lzmq

all: minimax

discrete.pb.cc discrete.pb.h: discrete.proto
	protoc --cpp_out=. discrete.proto

update.pb.cc update.pb.h: update.proto
	protoc --cpp_out=. update.proto

minimax: minimax.cpp discrete.pb.cc update.pb.cc
	$(CXX) minimax.cpp discrete.pb.cc update.pb.cc $(CXXFLAGS) -o minimax

clean:
	rm -f minimax discrete.pb.cc discrete.pb.h

doc:
	doxygen Doxyfile

test:
	echo $(CXX)
