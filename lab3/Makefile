CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O2

TARGET = lab3
SOURCES = main.cpp belt.cpp
HEADERS = BELT_HPP

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean