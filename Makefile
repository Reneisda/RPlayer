cc = gcc
all:
	$(cc) -fpic -DRAYGUI_IMPLEMENTATION -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 main.c -o player

run: all
	./player

