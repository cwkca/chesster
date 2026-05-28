.PHONY: run

CC=gcc
CFLAGS=-ansi -Iinclude `sdl2-config --cflags`
LDFLAGS = `sdl2-config --libs` -lSDL2_ttf -lSDL2_image
OBJS = obj/main.o obj/game.o obj/graphics.o obj/draw.o

run: bin/chess
	bin/chess

bin/chess: $(OBJS)
	$(CC) -o bin/chess $(OBJS) $(LDFLAGS)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
