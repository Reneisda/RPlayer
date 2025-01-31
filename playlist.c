#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


typedef struct playlist {
	size_t size;
	char** song_names;
	char** song_paths;
	char** id;
	char name[256];
} playlist_t;


uint8_t read_playlist(playlist_t* pl, const char* path) {
	FILE* f = fopen(path, "r");
	if (f == NULL) return 1;
	fseek(f, 0L, SEEK_END);
	size_t size = ftell(f);
	rewind(f);
	char* fbuff = malloc(sizeof(char) * size);
	fread(fbuff, sizeof(char), size, f);
	size_t line_count = 0;
	for (size_t i = 0; i < size; ++i) {
		if (fbuff[i] == '\n') { 
			++line_count;
			fbuff[i] = 0;
		}
	}
	pl->size = line_count / 2 - 1;
	printf("lc: %zu\n", line_count); 			
	pl->song_names = (char**) malloc(sizeof(char*) * (line_count) / 2);
	pl->song_paths = (char**) malloc(sizeof(char*) * (line_count) / 2);
	size_t index = 0;
	size_t flip = 0;
	if (strlen(fbuff) >= 256) {
		fprintf(stderr, "Playlist-name too long (Max 256 characters)\n");
		return 1;
	}
	memcpy(pl->name, fbuff, strlen(fbuff));

	for (size_t i = 0; index < pl->size; ++i) {
		if (fbuff[i] == 0) {
			if (flip == 0) {
				pl->song_names[index] = (char*) malloc(sizeof(char) * strlen(fbuff + i + 1));
				memcpy(pl->song_names[index], fbuff + i + 1, strlen(fbuff + i + 1));
			}
			else {
				pl->song_paths[index] = (char*) malloc(sizeof(char) * strlen(fbuff + i + 1));
				memcpy(pl->song_paths[index++], fbuff + i + 1, strlen(fbuff + i + 1));
			}
			flip ^= 1;
		}
	}

	free(fbuff);
	return 0;
}

void print_playlist(playlist_t* pl) {
	printf("Playlist: %s\n", pl->name);
	for (size_t i = 0; i < pl->size; ++i) 
		printf("%zu\t %s\t %s\n", i, pl->song_names[i], pl->song_paths[i]);
}

int main() {
	playlist_t pl;
	if (read_playlist(&pl, "./playlists/Test1.pl") == 1) {
		fprintf(stderr, "Could not read playlist");
		return 1;
	}

	print_playlist(&pl);
	return 0;
}

