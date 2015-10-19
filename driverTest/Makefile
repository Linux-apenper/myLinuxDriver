
OBJS=pollmonitor.o
CC=gcc
CFLAGS=-Wall -g -O
main:$(OBJS)
	$(CC) $(OBJS) -o pollmonitor

$(OBJS):%o:%c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -fr *.o *.symvers *.mod.c pollmonitor
