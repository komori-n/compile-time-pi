COMPILER    = clang++
CPPFLAGS   += -MMD -MP -Wall -Wextra -O3 -std=c++20
INCLUDE     = -I./src
TARGET      = ./calculate_pi.out
TEST_TARGET = ./test.out
SRCDIR      = ./src
OBJDIR      = ./obj
TEST_SRCS = $(wildcard src/tests/*.cpp)
TEST_OBJS = $(addprefix $(OBJDIR)/, $(notdir $(TEST_SRCS:.cpp=.o)))
TEST_DEPS = $(TEST_OBJS:.o=.d)

$(TARGET): $(OBJDIR)/main.o
	$(COMPILER) -o $@ $^

$(TEST_TARGET): $(TEST_OBJS)
	$(COMPILER) $(CPPFLAGS) $(INCLUDE) -o $@ $^ -lgtest -lgtest_main

$(OBJDIR)/main.o: $(SRCDIR)/main.cpp
	-mkdir -p $(OBJDIR)
	$(COMPILER) $(CPPFLAGS) $(INCLUDE) -o $@ -c $<

$(OBJDIR)/%.o: $(SRCDIR)/tests/%.cpp
	-mkdir -p $(OBJDIR)
	$(COMPILER) $(CPPFLAGS) $(INCLUDE) -o $@ -c $<

all: $(TARGET) test

test: $(TEST_TARGET)

clean:
	-rm -rf $(OBJDIR) $(TARGET) $(TEST_TARGET)

-include $(TEST_DEPS)