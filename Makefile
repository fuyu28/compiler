CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = sc
SRCS = decl.c expcode.c expres.c scan.c ssclib.c sscmain.c st.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c sscdef.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
