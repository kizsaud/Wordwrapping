CC = gcc
CFLAGS = -g -Wall -fsanitize=address,undefined

ww_mehmet: ww_mehmet.c
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $^

