# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
TARGET = main
SRC = main.cpp

# Default target
all: $(TARGET)

# Build the target
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Clean build files
clean:
	rm -f $(TARGET)

