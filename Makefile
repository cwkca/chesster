CCFLAGS = -ansi `sdl2-config --cflags`
#LDFLAGS = `sdl2-config --libs` -lSDL2_ttf -lSDL2_image

bin/chess: chess.c
	mkdir -p bin
	$(CC) $(CCFLAGS) -o bin/chess chess.c $(LDFLAGS)
