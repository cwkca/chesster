.PHONY: run-native run-web web

CFLAGS=-ansi -Iinclude `sdl2-config --cflags`


#
# Native
#

CC=clang
LDFLAGS = `sdl2-config --libs` -lSDL2_ttf -lSDL2_image
NATIVE_OBJS = $(shell ls src | sed 's|\(.*\)\.c|native/obj/\1.o|g') native/obj/main.o

native/chess: $(NATIVE_OBJS)
	$(CC) -o native/chess $(NATIVE_OBJS) $(LDFLAGS)

native/obj/%.o: src/%.c include/%.h
	mkdir -p native/obj; $(CC) $(CFLAGS) -c $< -o $@

native/obj/main.o: main/native.c
	mkdir -p native/obj; $(CC) $(CFLAGS) -c $< -o $@

run-native: native/chess
	native/chess


#
# Web (via emscripten)
#

EMFLAGS = -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]'
WEB_OBJS = $(shell ls src | sed 's|\(.*\)\.c|web/obj/\1.o|g') web/obj/main.o

web: web/chess.js static $(shell ls static/*)
	cp static/* web

web/chess.js: $(WEB_OBJS)
	emcc -o web/chess.js $(WEB_OBJS) $(EMFLAGS) --preload-file pieces

web/obj/%.o: src/%.c include/%.h
	mkdir -p web/obj; emcc -g $(CFLAGS) -c $< -o $@

web/obj/main.o: main/web.c
	mkdir -p web/obj; emcc -g $(CFLAGS) -c $< -o $@

run-web: web
	echo "\nServing at http://localhost:8000/chess.html\n"
	python3 -m http.server --directory web


#
# Util
#

clean:
	rm -rf native web
