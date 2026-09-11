CXX ?= g++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -Wpedantic -O2 -Iinclude

TEST_SRC = tests/test_main.cpp
TEST_BIN = tests/run_tests

EXAMPLE_SRCS = $(wildcard examples/*.cpp)
EXAMPLE_BINS = $(patsubst examples/%.cpp,bin/examples/%,$(EXAMPLE_SRCS))

all: test examples

$(TEST_BIN): $(TEST_SRC) $(wildcard include/discretex/**/*.hpp)
	@mkdir -p tests
	$(CXX) $(CXXFLAGS) $(TEST_SRC) -o $(TEST_BIN)

test: $(TEST_BIN)
	./$(TEST_BIN)

bin/examples/%: examples/%.cpp $(wildcard include/discretex/**/*.hpp)
	@mkdir -p bin/examples
	$(CXX) $(CXXFLAGS) $< -o $@

examples: $(EXAMPLE_BINS)
	@for ex in $(EXAMPLE_BINS); do \
		echo "--- Running $$ex ---"; \
		./$$ex; \
		echo ""; \
	done

clean:
	rm -f $(TEST_BIN)
	rm -rf bin/

.PHONY: all test examples clean

