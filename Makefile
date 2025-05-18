CXX = g++
CXXFLAGS = -std=c++14 -O2 -Wall
LDFLAGS = -lm

# Source files
SOURCES = main.cpp compiler.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = math-compiler.exe

# Build the executable
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	del /Q *.o $(EXECUTABLE)

# Run tests
test: $(EXECUTABLE)
	./$(EXECUTABLE) "3 4 +"
	./$(EXECUTABLE) "1 3 / 9 *"
	./$(EXECUTABLE) "pi 2 * sin"

# Install dependencies (no-op on Windows as we use standard libraries)
deps:
	@echo "No external dependencies required."

.PHONY: all clean test deps 