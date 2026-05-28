CCFLAGS = -ansi `sdl2-config --cflags`
LDFLAGS = `sdl2-config --libs` -lSDL2_ttf -lSDL2_image

chess: chess.c
	$(CC) $(CCFLAGS) -o chess chess.c $(LDFLAGS)
