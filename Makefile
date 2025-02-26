# === Makefile ===

CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude -O3
LDFLAGS = -lm -lwaggle

# Name of the final executable
TARGET  = perfmon

# Source files
SRCS = main.c \
       src/cpu/cpu.c \
       src/cpu/cpu_context.c \
       src/cpu/cpu_frame.c \
       src/cpu/outputs/cpu_output_stdout.c \
	   src/sampling/adaptive.c
#       src/cpu/outputs/cpu_output_plugin.c \

# Object files (turn each .c into a .o)
OBJS = $(SRCS:.c=.o)

# Default rule
all: $(TARGET)

# Link step
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile each .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleanup
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
