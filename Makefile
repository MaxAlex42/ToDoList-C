CC = gcc

GTK_CFLAGS = `pkg-config --cflags gtk+-3.0`
GTK_LDFLAGS = `pkg-config --libs gtk+-3.0`
JANSSON_CFLAGS = `pkg-config --cflags jansson`
JANSSON_LDFLAGS = `pkg-config --libs jansson`

# Default values
CFLAGS = $(GTK_CFLAGS) $(JANSSON_CFLAGS)
LDFLAGS = $(GTK_LDFLAGS) $(JANSSON_LDFLAGS)
TARGET = ToDoList
SRCS = todolist.c
OBJS = $(SRCS:.c=.o)

# Detect the platform
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDFLAGS += -rdynamic
endif
ifeq ($(OS),Windows_NT)
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
