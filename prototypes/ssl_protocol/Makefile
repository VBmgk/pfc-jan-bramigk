.PHONY: cpp python

CXXFLAGS := -I/usr/local/include -L/usr/local/lib

all: add_robot list_team list_team_server hwserver

protoc_middleman: discrete.proto
	protoc --cpp_out=. discrete.proto
	@touch protoc_middleman

add_robot: add_robot.cc protoc_middleman
	pkg-config --cflags protobuf  # fails if protobuf is not installed
	$(CXX) add_robot.cc discrete.pb.cc $(CXXFLAGS) -o add_robot `pkg-config --cflags --libs protobuf` -lzmq

list_team: list_team.cc protoc_middleman
	pkg-config --cflags protobuf  # fails if protobuf is not installed
	$(CXX) list_team.cc discrete.pb.cc $(CXXFLAGS) -o list_team `pkg-config --cflags --libs protobuf`

list_team_server: list_team.cc protoc_middleman
	pkg-config --cflags protobuf  # fails if protobuf is not installed
	$(CXX) list_team_server.cpp discrete.pb.cc $(CXXFLAGS) -o list_team_server `pkg-config --cflags --libs protobuf` -lzmq

hwserver: list_team.cc protoc_middleman
	pkg-config --cflags protobuf  # fails if protobuf is not installed
	$(CXX) hwserver.cc discrete.pb.cc $(CXXFLAGS) -o hwserver `pkg-config --cflags --libs protobuf` -lzmq

clean:
	rm -f add_robot list_team_server list_team hwserver
	rm -f protoc_middleman discrete.pb.cc discrete.pb.h
	rm -f *.pyc
	rm -r html latex

doc:
	doxygen Doxyfile
