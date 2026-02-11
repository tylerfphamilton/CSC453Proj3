CC := cc
CFLAGS := -std=c99 -D_POSIX_C_SOURCE=200112L -O2 -Wall -Wextra
LDFLAGS := -pthread	# need to add a different one I think


all: memSim

memSim: main.c 
	$(CC) $(CFLAGS) -o $@ main.c $(LDFLAGS)

clean:
	rm -f memSim

.PHONY: all clean
