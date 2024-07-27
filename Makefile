
CXX = g++
CXXFLAGS = -Wall -O2 -std=c++11
TARGET = test2.txt
SRC = Main2.cpp

# Default target
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)
	
	
	
test:
	./$(TARGET)	
	

# Clean up
clean:
	rm -f $(TARGET)

