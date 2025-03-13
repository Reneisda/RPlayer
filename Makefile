cc = gcc
wincc = x86_64-w64-mingw32-gcc
SOURCES = src/*.c

all:
	$(cc) -g -fpic -DRAYGUI_IMPLEMENTATION -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 $(SOURCES) -o player

run: all
	./player

clean:
	rm -rf player __pycache__ src/__pycache__

build-win: main.c songutils.c Makefile
	$(wincc) -o rplayer.exe main.c songutils.c \
    -I./\
    -L./win\
    -lraylib -lopengl32 -lgdi32 -lwinmm

