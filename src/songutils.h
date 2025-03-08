#include <stdint.h>

#ifdef linux
#include <unistd.h>
#endif
#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif

typedef struct song {
	char name[512];
	char artist[512];
	char id[12];
	uint64_t id_n;
} song_t;


typedef struct playlist {
	size_t count;
	song_t** songs;
	char name[512];
} playlist_t;

typedef struct song_list {
	song_t* songs;
	uint32_t count;
} song_list_t;


uint8_t read_songs(song_list_t* songs, const char* base_dir);
void print_songs(song_list_t* songs);
song_t* get_song(const song_list_t* sl, const char* id);
uint8_t load_playlist(playlist_t* pl, const song_list_t* sl, const char* name, const char* base_dir);
void print_playlist(playlist_t* pl);

