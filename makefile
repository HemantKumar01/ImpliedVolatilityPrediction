CXX = g++
CXXFLAGS = -std=c++17 -O3 -march=native -flto -funroll-loops -ffast-math
TARGET = csv_processor
SOURCE = matching.cpp

# Default target
all: $(TARGET)

# Compile the program
$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

# Run with example files
run: $(TARGET)
	./$(TARGET) input.csv output.csv

# Clean compiled files
clean:
	rm -f $(TARGET)

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Compile the CSV processor"
	@echo "  run     - Compile and run with input.csv/output.csv"
	@echo "  clean   - Remove compiled executable"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make"
	@echo "  ./csv_processor input.csv output.csv"

.PHONY: all run clean help