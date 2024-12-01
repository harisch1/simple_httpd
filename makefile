# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Target executable
TARGET = httpd.o

# Source files
SRCS = $(wildcard httpd/*.c)

# Object files
OBJS = $(SRCS:%.c=%.o)

# Default target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean

debug: all
	gdb $(TARGET)

# Run the project
run: all
	./$(TARGET) 3000

check:
	./$(TARGET) 3000 & sleep 1; curl -I http://localhost:8069/; killall $(TARGET)

# Run the project with valgrind and kill valgrdind after the project is done
valgrind: all
	valgrind --leak-check=full ./$(TARGET) 8069 & sleep 1; curl -I http://localhost:8069/; killall valgrind