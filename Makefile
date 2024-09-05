CC?=gcc
CFLAGS?=-g -O0 -fPIC -Wall -Werror
INCLUDES=$(HOME)/workspace-cpp/readcsv

objs= \
	src/readcsv.o

%.o: %.c
	$(CC) $(CFLAGS) -I$(INCLUDES) -c -o $@ $<

libs= \
	lib/libreadcsv.so

lib/libreadcsv.so: $(objs)
	$(CC) -shared -o $@ $(objs)

all: $(libs)

clean:
	rm -f lib/libreadcsv.so
	rm -f src/*.o
	