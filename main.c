#include "raylib.h"
#include <stdio.h>
#include "raygui.h"
#include <stdint.h>
#include "songutils.h"
#include <string.h>
#include <malloc.h>
#include <math.h>


#ifdef linux
#include <unistd.h>
#endif
#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif

#define APP_NAME "Player"
#define START_WIDTH 1200
#define START_HEIGHT 680
#define MIN_WIDTH 800
#define MIN_HEIGHT 600
#define PLAYLISTS_FLODER "playlists/"
#define SONG_FOLDER "songs2/"
#define THUMBNAIL_FOLDER "thumbnails/"


typedef struct timestamp {
	uint8_t hours;
	uint8_t mins;
	uint8_t secs;
} timestamp_t;
 
void timestamp_set(timestamp_t* t, float f) {
	uint32_t i = (uint32_t) f;
	t->secs = 	i % 60;
	t->mins = 	i / 60;
	t->hours = 	i / 3600;
}

void timestamp_get(timestamp_t* t, char* time) {
	if (t->hours <= 1)
		sprintf(time, "%.2d:%.2d", t->mins, t->secs);
	else
		sprintf(time, "%.2d:%.2d:%.2d", t->hours, t->mins, t->secs);
}

char* get_file_name(char* path) {
	char* p;
	for (p = path; *p != 0; ++p) {
		if (*p == '/') path = p + 1;
	}	
	return path;
}

char* to_stringf(char* buff, float f) {
	sprintf(buff, "%.0f", f);
	return buff;
}

float volume_function(float f) {
	return powf(f, 3);
}

int main() {
	int height, width;
	int refresh_rate = GetMonitorRefreshRate(GetCurrentMonitor());
	refresh_rate = refresh_rate == 0 ? 60 : refresh_rate;
	char search[256];
	memset(search, 0, 256);
	printf("Rendering at %dfps\n", refresh_rate);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
	SetWindowMinSize(MIN_WIDTH, MIN_HEIGHT);
	InitWindow(START_WIDTH, START_HEIGHT, APP_NAME);
	InitAudioDevice();
	SetTargetFPS(refresh_rate);
	GuiLoadStyle("assets/style_def.rgs");
	//GuiLoadStyle("assets/dark.rgs");
	Font fontBm = LoadFontEx("assets/UbuntuSansNerdFont-Bold.ttf", 36, 0, 250);
	GuiSetFont(fontBm);
	FilePathList files = LoadDirectoryFiles("./songs2");
	Music current_song = {0}; 
	
	float current_song_len = 0;
	float current_song_pos = 0;
	timestamp_t cur_timestamp = {.hours = 0, .mins = 0, .secs = 0};
	timestamp_t end_timestamp = {.hours = 0, .mins = 0, .secs = 0};
	
	timestamp_set(&cur_timestamp, 0.f);
	timestamp_set(&end_timestamp, 0.f);
	float volume = 0.2;
	SetMasterVolume(volume_function(volume));

	int scroll = 0; 
	int cur_playlist_size = files.count;
	// read all Playlists
	FilePathList playlists_files = LoadDirectoryFiles(PLAYLISTS_FLODER);
	playlist_t* playlists;
	uint32_t playlist_count = playlists_files.count;
	playlists = (playlist_t*) malloc(sizeof(playlist_t) * playlist_count);
	playlist_t all_songs;
	playlist_t* cur_pl;
	song_list_t sl;
	read_songs(&sl);
	for (uint32_t i = 0; i < playlist_count; ++i) {
		char playlist_name[1024];
		strcpy(playlist_name, get_file_name(playlists_files.paths[i]));
		playlist_name[strlen(playlist_name) - 3] = 0;
		printf("playlistName: %s\n", playlist_name);
		load_playlist(&playlists[i], &sl, playlist_name);
	}
	
	cur_pl = &playlists[0];
	song_t* cur_song;
	char def_thumb_path[256];
	strcpy(def_thumb_path, THUMBNAIL_FOLDER);
	strcat(def_thumb_path, cur_pl->songs[0]->id);
	strcat(def_thumb_path, ".jpg");
	Texture backgound_texture = LoadTexture(def_thumb_path);

	while (!WindowShouldClose()) {
		BeginDrawing();
		width = GetScreenWidth();
		height = GetScreenHeight();
		if (width < MIN_WIDTH)												// Enforce Minimum Window size
			width = MIN_WIDTH;
		if (height < MIN_HEIGHT)
			height = MIN_HEIGHT;

		SetWindowSize(width, height);


		ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
		GuiPanel((Rectangle) {0, 0, width, 70}, "RPlayer");									// TopPanel
		GuiPanel((Rectangle) {0, 80, 260, height - 300}, "Playlists");						// Sidepanel
		GuiPanel((Rectangle) {270, 80, (width - 270 * 2), height - 190}, "Songs");			// SongPanel
		GuiTextBox((Rectangle) {270, 30, width - 270 * 2, 25}, search, 256, true);			// SearchBar
		GuiScrollBar((Rectangle) {width - 265, 80, 10, height - 190}, scroll, 0, cur_playlist_size * 10);
		DrawText("Search: ", 180, 34, 20, GetColor(0xFFFFFFFF));							// Searchbar Text 
		if (GuiSliderBar((Rectangle) {width - 200, height - 55, 160, 7}, "", "", &volume, 0, 1)) 	// VolumeBar
			SetMasterVolume(volume_function(volume));
		
		
		float scroll_wheel = GetMouseWheelMove();
		if (scroll_wheel < 0 && (scroll / 10) < cur_pl->count - 1)						// scroll songs
			scroll += 10;

		else if (scroll_wheel > 0 && scroll >= 10)
			scroll -= 10;

		if (IsKeyPressed(KEY_SPACE)) {
			if (IsMusicStreamPlaying(current_song)) {
				PauseMusicStream(current_song);
			}
			else {
				ResumeMusicStream(current_song);
			}
		}

		for (size_t i = 0; i < playlists_files.count; ++i) {
			if (GuiButton((Rectangle) {5, 110 + i * 50, 250, 50}, playlists[i].name)) {	
				scroll = 0;
				cur_pl = &playlists[i];
			}
		}

		char s_name[256 + 10];
		cur_playlist_size = cur_pl->count;
		for (size_t i = 0; i + scroll / 10 < cur_pl->count; ++i) {	
			if (110 + i * 45 + 40 > height - 120)
				break;

			if (GuiButton((Rectangle) {280, 110 + i * 45, width - 280 * 2 , 40} , cur_pl->songs[i + scroll / 10]->name)) {
				strcpy(s_name, "songs2/");
				strcat(s_name, cur_pl->songs[i + scroll / 10]->id);
				strcat(s_name, ".wav");
				if (access(s_name, F_OK) != 0)
					continue;
				
				cur_song = cur_pl->songs[i + scroll / 10];
				current_song = LoadMusicStream(s_name);
				PlayMusicStream(current_song);
				current_song_len = GetMusicTimeLength(current_song);
				timestamp_set(&end_timestamp, current_song_len);
				// render backgound_texture
				char thumbnail_path[256 + 10];
				strcpy(thumbnail_path, THUMBNAIL_FOLDER);
				strcat(thumbnail_path, cur_song->id);
				strcat(thumbnail_path, ".jpg");
				backgound_texture = LoadTexture(thumbnail_path);
			}
		}
		
		DrawTexturePro(backgound_texture, 
				(Rectangle) {0, 0, backgound_texture.width, backgound_texture.height},
				(Rectangle) {0, 0, width, height},
				(Vector2) {0, 0},
				0,
				CLITERAL(Color){255, 255, 255, 40});

			
		current_song_pos = GetMusicTimePlayed(current_song);
		timestamp_set(&cur_timestamp, current_song_pos);
		char timestamp_cur_str[16];
		char timestamp_end_str[16];
		timestamp_get(&cur_timestamp, timestamp_cur_str);
		timestamp_get(&end_timestamp, timestamp_end_str);
		// Song-ProgressBar
		if (GuiSliderBar((Rectangle) {(int) (width / 4), height - 60, (int) (width / 2), 20},
				timestamp_cur_str, timestamp_end_str, &current_song_pos, 0, current_song_len)) {
			
			SeekMusicStream(current_song, current_song_pos);
		}


		UpdateMusicStream(current_song);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}

