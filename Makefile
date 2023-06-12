COMPILER    = clang++
CPPFLAGS   += -Wall -Wextra -O3 -std=c++20
INCLUDE     = -I./src
TARGET      = ./calculate_pi.out
TEST_TARGET = ./test.out
SRCDIR      = ./src
OBJDIR      = ./obj
TEST_SRCS = $(wildcard src/tests/*.cpp)

$(TARGET): $(OBJDIR)/main.o
	$(COMPILER) -o $@ $^

$(TEST_TARGET): $(TEST_SRCS)
	$(COMPILER) $(CPPFLAGS) $(INCLUDE) -o $@ $^ -lgtest -lgtest_main

$(OBJDIR)/main.o: $(SRCDIR)/komori/main.cpp
	-mkdir -p $(OBJDIR)
	$(COMPILER) $(CPPFLAGS) $(INCLUDE) -o $@ -c $<

all: $(TARGET) test

test: $(TEST_TARGET)

clean:
	-rm -rf $(OBJDIR) $(TARGET) $(TEST_TARGET)
