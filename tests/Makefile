CXX = clang++-3.6
BOOST_DIR ?= boost_dir
PREFIX_DIR ?= prefix_dir

SRC = main.cpp
LIBS = -lmat2cpp -llapack -llapacke -lblas -lm
CXXFLAGS = -Wall -std=c++11 -g -O3
EXEC = solver

$(EXEC): $(SRC) Makefile
	$(CXX) $(SRC) -o $(EXEC) $(CXXFLAGS) -isystem $(BOOST_DIR)/include -isystem $(PREFIX_DIR)/include -L$(PREFIX_DIR)/lib $(LIBS)

.PHONY: run
run: $(EXEC)
	./$(EXEC)

.PHONY: clean
clean:
	rm -f $(EXEC)
