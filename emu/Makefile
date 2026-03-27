CC      = cc
CFLAGS  = -std=c99 -Wall -Wextra -Wpedantic -O2

TARGET  = poem-emu
SRCS    = main.c vm.c
OBJS    = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c vm.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean
