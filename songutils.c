#include "songutils.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef linux
#include <unistd.h>
#endif
#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif


#define SONG_INFO_PATH "songs2/info.pl"
#define PLAYLIST_FILE_SUFFIX ".pl"
#define PLAYLIST_DIR "playlists/"

uint8_t read_songs(song_list_t* songs) {
	if (access(SONG_INFO_PATH, F_OK) != 0)
		return 1;

	FILE* f = fopen(SONG_INFO_PATH, "r");
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
	
	songs->count = line_count / 3;
	songs->songs = (song_t*) malloc(sizeof(song_t) * songs->count);
	size_t offset = 0;
	for (size_t i = 0; i < songs->count; ++i) {
		strcpy(songs->songs[i].id, fbuff + offset);
		offset += strlen(fbuff + offset) + 1;
		strcpy(songs->songs[i].name, fbuff + offset);
		offset += strlen(fbuff + offset) + 1;
		strcpy(songs->songs[i].artist, fbuff + offset);
		offset += strlen(fbuff + offset) + 1;
	}
	free(fbuff);
	fclose(f);
	return 0;
}


void print_songs(song_list_t* songs) {
	for (size_t i = 0; i < songs->count; ++i) {
		printf("ID:     %s\n", songs->songs[i].id);
		printf("Name:   %s\n", songs->songs[i].name);
		printf("Artist: %s\n", songs->songs[i].artist);
	}
}

song_t* get_song(const song_list_t* sl, const char* id) {			// TODO speed-up
	for (size_t i = 0; i < sl->count; ++i)
		if (strcmp(sl->songs[i].id, id) == 0)
			return &sl->songs[i];
	
	return NULL;
}


uint8_t load_playlist(playlist_t* pl, const song_list_t* sl, const char* name) {
	char filename[1024];
	strcpy(filename, PLAYLIST_DIR);
	strcat(filename, name);
	strcat(filename, PLAYLIST_FILE_SUFFIX);
	if (access(filename, F_OK) != 0) {
		fprintf(stderr, "Playlist %s not found\n", name);
		return 1; 
	}

	strcpy(pl->name, name);
	FILE* f = fopen(filename, "r");
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
	
	pl->count = line_count;
	pl->songs = (song_t**) malloc(sizeof(song_t*) * pl->count);
	size_t offset = 0;
	for (size_t i = 0; i < pl->count; ++i) {
		pl->songs[i] = get_song(sl, fbuff + offset);
		offset += strlen(fbuff + offset) + 1;
	}
	free(fbuff);
	fclose(f);
	
	
	return 0;
}

void print_playlist(playlist_t* pl) {
	for (size_t i = 0; i < pl->count; ++i) {
		printf("ID:     %s\n", pl->songs[i]->id);
		printf("Name:   %s\n", pl->songs[i]->name);
		printf("Artist: %s\n", pl->songs[i]->artist);
	}
}

