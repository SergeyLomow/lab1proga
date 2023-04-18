CC=gcc
CFLAGS=-Wall -Wextra -Werror -O3

SRCS=main.c
OBJS=$(SRCS:.c=.o)

TARGET=lab11pamN32491

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
