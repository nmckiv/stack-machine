# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -I./ftxui

# Library search path
LDFLAGS = -L./ftxui

# Libraries
LIBS = -lftxui-component -lftxui-dom -lftxui-screen

# Source files
SRCS = main.cpp machine.cpp instructions.cpp tui.cpp console.cpp

# Output executable
TARGET = stack-machine

# Build target
$(TARGET): $(SRCS)
	$(CXX) $(SRCS) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS) $(LIBS)

# Clean target
clean:
	rm -f $(TARGET)