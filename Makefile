CC = gcc

CFLAGS = `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0` -rdynamic
TARGET = ToDoList
SRCS = todolist.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean

# gcc `pkg-config --cflags gtk+-3.0` -o ToDoList test.c `pkg-config --libs gtk+-3.0` -rdynamic
