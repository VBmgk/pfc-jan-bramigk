#!/bin/sh
exec clang-format -style=file -i src/*.{cpp,h}
