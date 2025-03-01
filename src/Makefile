cc = gcc
wincc = x86_64-w64-mingw32-gcc

all:
	$(cc) -g -fpic -DRAYGUI_IMPLEMENTATION -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 main.c songutils.c -o player

run: all
	./player

build-win: main.c songutils.c Makefile
	$(wincc) -o rplayer.exe main.c songutils.c \
    -I./\
    -L./win\
    -lraylib -lopengl32 -lgdi32 -lwinmm

