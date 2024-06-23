CC = gcc

GTK_CFLAGS = `pkg-config --cflags gtk+-3.0`
GTK_LDFLAGS = `pkg-config --libs gtk+-3.0`
JANSSON_CFLAGS = `pkg-config --cflags jansson`
JANSSON_LDFLAGS = `pkg-config --libs jansson`
CFLAGS = $(GTK_CFLAGS) $(JANSSON_CFLAGS)
LDFLAGS = $(GTK_LDFLAGS) $(JANSSON_LDFLAGS) -rdynamic
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
