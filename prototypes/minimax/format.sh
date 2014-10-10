#!/bin/sh
exec clang-format -style=file -i inc/*.h src/*.cpp
