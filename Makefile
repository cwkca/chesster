CFLAGS := $(shell sdl2-config --cflags)
LDFLAGS := $(shell sdl2-config --libs)

hello: hello.c
	gcc $(CFLAGS) -o hello hello.c $(LDFLAGS)
