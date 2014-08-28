.PHONY: cpp python

cpp:    add_robot_cpp    list_team_cpp
python: add_robot_python list_robot_python

protoc_middleman: team.proto
	protoc --cpp_out=. --python_out=. team.proto
	@touch protoc_middleman

add_robot_cpp: add_robot.cc protoc_middleman
	pkg-config --cflags protobuf  # fails if protobuf is not installed
	c++ add_robot.cc team.pb.cc -o add_robot_cpp `pkg-config --cflags --libs protobuf`

list_team_cpp: list_team.cc protoc_middleman
	pkg-config --cflags protobuf  # fails if protobuf is not installed
	c++ list_team.cc team.pb.cc -o list_team_cpp `pkg-config --cflags --libs protobuf`

add_robot_python: add_person.py protoc_middleman
	@echo "Writing shortcut script add_robot_python..."
	@echo '#! /bin/sh' > add_robot_python
	@echo './add_robot.py "$$@"' >> add_robot_python
	@chmod +x add_robot_python

list_robot_python: list_robot.py protoc_middleman
	@echo "Writing shortcut script list_robot_python..."
	@echo '#! /bin/sh' > list_robot_python
	@echo './list_robot.py "$$@"' >> list_robot_python
	@chmod +x list_robot_python

clean:
	rm -f add_robot_cpp list_team_cpp add_robot_python list_robot_python
	rm -f protoc_middleman team.pb.cc team.pb.h robot_pb2.py
	rm -f *.pyc
	rm -r html latex

doc:
	doxygen Doxyfile
