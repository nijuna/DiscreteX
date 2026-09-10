CXX ?= g++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -Wpedantic -O2 -Iinclude

TEST_SRC = tests/test_main.cpp
TEST_BIN = tests/run_tests

all: test

$(TEST_BIN): $(TEST_SRC) $(wildcard include/discretex/**/*.hpp)
	$(CXX) $(CXXFLAGS) $(TEST_SRC) -o $(TEST_BIN)

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(TEST_BIN)

.PHONY: all test clean
