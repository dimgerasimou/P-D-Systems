CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lmatio -lm

SRC = main.c io.c
OBJ = $(patsubst %.c,obj/%.o,$(SRC))  # obj/main.o obj/io.o
TARGET = pardis0

all: $(TARGET)

# Link step
$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(LIBS) -o $@

# Compile step
obj/%.o: %.c | obj
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure obj directory exists
obj:
	mkdir -p obj

clean:
	rm -rf obj $(TARGET)
