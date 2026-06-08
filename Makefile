.PHONY: all native web run-native run-web

CFLAGS=-ansi -Iinclude `sdl2-config --cflags`
BUILD_PATH=build

all: native web

#
# Native
#

CC=clang
LDFLAGS = `sdl2-config --libs` -lSDL2_ttf -lSDL2_image
NATIVE_BUILD=$(BUILD_PATH)/native
NATIVE_OBJ_PATH=$(NATIVE_BUILD)/obj
NATIVE_OBJS = $(shell ls src | sed 's|\(.*\)\.c|$(NATIVE_OBJ_PATH)/\1.o|g') $(NATIVE_OBJ_PATH)/main.o

native: $(NATIVE_BUILD)/chess

$(NATIVE_OBJ_PATH):
	mkdir -p $(NATIVE_OBJ_PATH)

$(NATIVE_BUILD)/chess: $(NATIVE_OBJS)
	$(CC) -o $(NATIVE_BUILD)/chess $(NATIVE_OBJS) $(LDFLAGS)

$(NATIVE_OBJ_PATH)/%.o: src/%.c include/%.h | $(NATIVE_OBJ_PATH)
	$(CC) $(CFLAGS) -c $< -o $@

$(NATIVE_OBJ_PATH)/main.o: main/native.c | $(NATIVE_OBJ_PATH)
	$(CC) $(CFLAGS) -c $< -o $@

run-native: $(NATIVE_BUILD)/chess
	$(NATIVE_BUILD)/chess


#
# Web (via emscripten)
#

EMFLAGS = -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_TTF=2
WEB_BUILD=$(BUILD_PATH)/web
WEB_OBJ_PATH=$(WEB_BUILD)/obj
WEB_OBJS = $(shell ls src | sed 's|\(.*\)\.c|$(WEB_OBJ_PATH)/\1.o|g') $(WEB_OBJ_PATH)/main.o

web: $(WEB_BUILD)/chess.js $(shell ls assets/web/*)
	cp assets/web/* $(WEB_BUILD)

$(WEB_BUILD)/chess.js: $(WEB_OBJS)
	emcc -o $(WEB_BUILD)/chess.js $(WEB_OBJS) $(EMFLAGS) --preload-file assets

$(WEB_OBJ_PATH)/%.o: src/%.c include/%.h
	mkdir -p $(WEB_OBJ_PATH); emcc -g $(CFLAGS) -c $< -o $@

$(WEB_OBJ_PATH)/main.o: main/web.c
	mkdir -p $(WEB_OBJ_PATH); emcc -g $(CFLAGS) -c $< -o $@

run-web: web
	echo "\nServing at http://localhost:8000/chess.html\n"
	python3 -m http.server --directory $(WEB_BUILD)


#
# Util
#

clean:
	rm -rf $(BUILD_PATH)
