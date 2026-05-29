.PHONY: run

CC=clang
CFLAGS=-ansi -Iinclude `sdl2-config --cflags`
LDFLAGS = `sdl2-config --libs` -lSDL2_ttf -lSDL2_image
OBJS = obj/draw.o obj/game.o obj/graphics.o obj/main.o obj/term.o

run: bin/chess
	bin/chess

bin/chess: $(OBJS)
	mkdir -p bin; $(CC) -o bin/chess $(OBJS) $(LDFLAGS)

obj/%.o: src/%.c include/%.h
	mkdir -p obj; $(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj bin
