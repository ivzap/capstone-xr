# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++11 -Wall -Wextra
LDFLAGS := -pthread 

CXXFLAGS += $(shell pkg-config --cflags NDI opencv4)
LDFLAGS += $(shell pkg-config --libs NDI opencv4) -Wl,-rpath,$(shell pkg-config --variable=libdir NDI)


# Target executable
TARGET := my_ndi_app

# Source files
SRCS := ndi.cpp
OBJS := $(SRCS:.cpp=.o)

# Default build rule
all: $(TARGET)

# Link object files into executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJS) $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
