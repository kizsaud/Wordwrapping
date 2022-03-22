CC = gcc 
CFLAGS = -g -Wall -fsanitize=address,undefined

all:ww_mehmet

%.o: %.c 
    $(CC) $(CFLAGS) -o $^


