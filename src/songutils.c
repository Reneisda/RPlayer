#include "songutils.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "paths.h"


#ifdef linux
#include <unistd.h>
#endif
#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif


uint8_t read_songs(song_list_t* songs, const char* base_dir) {
	char songs_info_path[2048];
	snprintf(songs_info_path, sizeof(songs_info_path), "%s%s", base_dir, SONG_INFO_DIR);
	printf("Searching for songinfo in: %s\n", songs_info_path);
	if (access(songs_info_path, F_OK) != 0) {
		fprintf(stderr, "Could not open songs-file");
		return 1;
	}	
	FILE* f = fopen(songs_info_path, "rb");
	fseek(f, 0L, SEEK_END);
	size_t size = ftell(f);
	rewind(f);
	char* fbuff = malloc(sizeof(char) * (size + 1));
	fread(fbuff, sizeof(char), size, f);
	fbuff[size] = 0;
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
	char id_[12];
	memcpy(id_, id, 11);
	id_[11] = 0;
	for (size_t i = 0; i < sl->count; ++i)
		if (strcmp(sl->songs[i].id, id_) == 0)
			return &sl->songs[i];
	
	return NULL;
}


uint8_t load_playlist(playlist_t* pl, const song_list_t* sl, const char* id, const char* base_dir) {
	char filename[2048];
	printf("Opening playlist: %s%s%s%s\n", BASE_DIR, PLAYLIST_DIR, id, PLAYLIST_FILE_SUFFIX);
	snprintf(filename, sizeof(filename), "%s%s%s%s", BASE_DIR, PLAYLIST_DIR, id, PLAYLIST_FILE_SUFFIX);

	if (access(filename, F_OK) != 0) {
		fprintf(stderr, "Could not open playlist-file");
		return 1;
	}

	FILE* f = fopen(filename, "rb");
	fseek(f, 0L, SEEK_END);
	size_t size = ftell(f);
	rewind(f);
	char* fbuff = malloc(sizeof(char) * (size + 1));
	fbuff[size] = 0;
	fread(fbuff, sizeof(char), size, f);
	size_t id_count = 0L;

	char *name;
	name = strtok(fbuff, "\n");
	if (name != NULL)	
		memcpy(pl->name, name, strlen(fbuff));

	pl->name[strlen(fbuff)] = 0;
	printf("Got Playlist name: %s\n", pl->name);
	char* id_ = strtok(NULL, "\n");
	while (id_ != NULL) {
		id_ = strtok(NULL, "\n");
		++id_count;
	}
	pl->count = id_count;
	printf("Got Playlist size: %zu\n", pl->count);
	pl->songs = (song_t**) malloc(sizeof(song_t*) * pl->count);
	uint32_t index = 0;
	for (int i = strlen(name); i < size - 1; ++i) {
		if (fbuff[i] == 0) {
			pl->songs[index++] = get_song(sl, fbuff + i + 1);
		}
	}

	free(fbuff);
	fclose(f);
	return 0;
}

void print_playlist(playlist_t* pl) {
	for (size_t i = 0; i < pl->count; ++i) {
		printf("%zu.\n", i + 1);
		printf("ID:     %s\n", pl->songs[i]->id);
		printf("Name:   %s\n", pl->songs[i]->name);
		printf("Artist: %s\n", pl->songs[i]->artist);
	}
}

/*
int main() {
	song_list_t sl;
	playlist_t pl;
	read_songs(&sl, "/home/rene/Music/rplayer/");
	printf("Found %d songs\n", sl.count);
	printf("First %s\n", sl.songs[0].name);
	print_songs(&sl);
	
	load_playlist(&pl, &sl, "PLATB9XS5FOa4pwSRJ_zzrNU4aFxhKqzFp", "/home/rene/Music/rplayer/");
	print_playlist(&pl);
	return 0;
}
*/
