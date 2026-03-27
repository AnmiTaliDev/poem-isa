CC      = cc
CFLAGS  = -std=c99 -Wall -Wextra -Wpedantic -O2

TARGET  = poem-as
SRCS    = main.c asm.c
OBJS    = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c asm.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean
