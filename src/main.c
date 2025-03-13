#include "raylib.h"
#include <stdio.h>
#include "raygui.h"
#include <stdint.h>
#include "songutils.h"
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include "paths.h"
#include "utils.h"


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
#define STYLE "styles/style_def.rgs"
#define ASSETS "assets/"
#define FONTS "fonts/"

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
	uint8_t seeking = 2;
	uint8_t adding_playlist_toggle = 0;
	uint8_t is_playing = 0;
	float current_song_len = 0;
	float current_song_pos = 0;
	float current_slider_pos = 0;
	char* current_song_name[255];
	current_song_name[0] = 0;

	char search[256];

	SetTraceLogLevel(LOG_ALL);
	memset(search, 0, 256);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
	InitWindow(START_WIDTH, START_HEIGHT, APP_NAME);
	InitAudioDevice();

	SetWindowMinSize(MIN_WIDTH, MIN_HEIGHT);		// TODO Crashes on windows i think
	int refresh_rate = GetMonitorRefreshRate(GetCurrentMonitor());
	refresh_rate = refresh_rate == 0 ? 60 : refresh_rate;
	printf("Rendering at %dfps\n", refresh_rate);
	SetTargetFPS(refresh_rate);
	SetExitKey(0);
	
	GuiLoadStyle(STYLE);
	GuiSetStyle(DEFAULT, TEXT_SIZE, 18);
	GuiSetStyle(DEFAULT, TEXT_PADDING, 10);
	// GuiLoadStyle("assets/dark.rgs");
	int codepoints[255 + (0x9faf - 0x3000)];
	// generating codepoints for ascii
	int i = 0;
	for (i = 0; i < 255; ++i)
		codepoints[i] = i;

	// generating codepoints for kanji
	for (int c = 0x3000; c < 0x9faf; ++c)
		codepoints[i++] = c;

	Font font = LoadFontEx("fonts/MPLUS1-Bold.ttf", 18, codepoints, 255 + (0x9faf - 0x3000));
		
	// Font font = LoadFontEx("fonts/MPLUS1-Bold.ttf", 18, codepoints, 6);
	GuiSetFont(font);
	config_t* config = utils_read_config();
	if (config == NULL) {
		fprintf(stderr, "Could not open config file\n");
		exit(1);
	}

	Music current_song = {0};
	timestamp_t cur_timestamp = {.hours = 0, .mins = 0, .secs = 0};
	timestamp_t end_timestamp = {.hours = 0, .mins = 0, .secs = 0};
	float volume = 0.2;
	SetMasterVolume(volume_function(volume));

	// FilePathList files = LoadDirectoryFiles(songs_dir);
	song_list_t sl;
	if (!read_songs(&sl, config->base_dir)) {
		sl.songs = NULL;
		sl.count = 0;
	}
	timestamp_set(&cur_timestamp, 0.f);
	timestamp_set(&end_timestamp, 0.f);

	int scroll = 0;
	// int cur_playlist_size = files.count;
	// read all Playlists
	char playlists_files_dir[2048];
	snprintf(playlists_files_dir, sizeof(playlists_files_dir), "%s%s", config->base_dir, PLAYLISTS_DIR);
	printf("LOADING PLAYLISTS FROM: [>%s<]\n", playlists_files_dir);
	FilePathList playlists_files = LoadDirectoryFiles(playlists_files_dir);

	playlist_t* playlists;
	uint32_t playlist_count = playlists_files.count;
	playlists = (playlist_t*) malloc(sizeof(playlist_t) * playlist_count);
	playlist_t all_songs = {0};
	playlist_t* cur_pl;

	read_songs(&sl, config->base_dir);
	printf("found %d songs\n", sl.count);
	for (int i = i; i < sl.count; ++i) {
		printf("Songs from Songlist: %s %s\n", sl.songs[i].id, sl.songs[i].name);
	}

	all_songs.count = sl.count;
	all_songs.songs = &sl.songs;
	strcpy(all_songs.name, "All Songs");

	for (uint32_t i = 0; i < playlist_count; ++i) {
		char playlist_file_name[2048];
		strcpy(playlist_file_name, get_file_name(playlists_files.paths[i]));
		playlist_file_name[strlen(playlist_file_name) - (sizeof(PLAYLIST_FILE_SUFFIX) - 1)] = 0;
		printf("playlistName: %s\n", playlist_file_name);
		load_playlist(&playlists[i], &sl, playlist_file_name, config->base_dir);
	}

	
	// Preloading textures
	Texture backgound_texture;
	Texture txt_playing = LoadTexture(ASSETS "btn_pause_30.png");
	Texture txt_paused = LoadTexture(ASSETS "btn_play_30.png");

	song_t* cur_song;
	cur_song = NULL;
	if (playlists->count > 0) {
		cur_pl = &playlists[0];		// TODO make this better
		char def_thumb_path[2048];
		printf("Using Thumbnail_dir: %s%s%s%s\n", config->base_dir, THUMBNAIL_DIR, cur_pl->songs[0]->id, ".jpg");
		snprintf(def_thumb_path, sizeof(def_thumb_path), "%s%s%s%s", config->base_dir,
			THUMBNAIL_DIR, cur_pl->songs[0]->id, ".jpg");

	backgound_texture = LoadTexture(def_thumb_path);
	}
	else {
		cur_pl = NULL;
	}

	while (!WindowShouldClose()) {
		BeginDrawing();
		width = GetScreenWidth();
		height = GetScreenHeight();
		is_playing = IsMusicStreamPlaying(current_song);

		ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
		GuiPanel((Rectangle) {0, 0, width, 70}, "RPlayer");									// TopPanel
		GuiPanel((Rectangle) {0, 80, 260, height - 300}, "Playlists");						// Sidepanel
		GuiPanel((Rectangle) {270, 80, (width - 270 * 2), height - 190}, "Songs");			// SongPanel
		GuiTextBox((Rectangle) {270, 30, width - 270 * 2, 25}, search, 256, 1);				// SearchBar
		GuiScrollBar((Rectangle) {width - 265, 80, 10, height - 190}, scroll, 0, (int) cur_pl->count * 10);
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

		if (GuiButton((Rectangle) {5, 135, 250, 50}, "All Songs")) {
			scroll = 0;
			cur_pl = &all_songs;
		}	
		// list playlists
		for (size_t i = 0; i < playlists_files.count; ++i) {
			if (GuiButton((Rectangle) {5, 190 + i * 55, 250, 50}, playlists[i].name)) {
				scroll = 0;
				cur_pl = &playlists[i];
			}	
		}
		char s_name[2048];
		for (size_t i = 0; i + scroll / 10 < cur_pl->count; ++i) {	
			if (110 + i * 45 + 40 > height - 120)
				break;
			
			if (GuiButton((Rectangle) {280, 110 + i * 45, width - 280 * 2 , 40} , cur_pl->songs[i + scroll / 10]->name)) {
				printf("AudioPath: %s%s%s%s", config->base_dir, SONG_DIR, cur_pl->songs[i + scroll / 10]->id, ".mp3");
				
				snprintf(s_name, sizeof(s_name), "%s%s%s%s", config->base_dir, SONG_DIR, cur_pl->songs[i + scroll / 10]->id, ".mp3");
				if (access(s_name, F_OK) != 0)
					continue;
					
				cur_song = cur_pl->songs[i + scroll / 10];
				current_song = LoadMusicStream(s_name);
				PlayMusicStream(current_song);
				current_song_len = GetMusicTimeLength(current_song);
				timestamp_set(&end_timestamp, current_song_len);
				// render backgound_texture
				char thumbnail_path[2048];
				snprintf(thumbnail_path, sizeof(thumbnail_path), "%s%s%s%s", config->base_dir, THUMBNAIL_DIR, cur_song->id, ".jpg");
				backgound_texture = LoadTexture(thumbnail_path);
			}
			//DrawTextEx(font, cur_pl->songs[i + scroll / 10]->name,	(Vector2) {280, 110 + i * 45}, 25, 1, WHITE);
		}
		
		DrawTexturePro(backgound_texture, 
				(Rectangle) {0, 0, backgound_texture.width, backgound_texture.height},
				(Rectangle) {0, 0, width, height},
				(Vector2) {0, 0},
				0,
				CLITERAL(Color){255, 255, 255, 40});

		DrawRectangle(0, height - 100, width, 100, CLITERAL(Color){0, 0, 0, 95});
				
		current_song_pos = GetMusicTimePlayed(current_song);
		timestamp_set(&cur_timestamp, current_song_pos);
		char timestamp_cur_str[16];
		char timestamp_end_str[16];
		timestamp_get(&cur_timestamp, timestamp_cur_str);
		timestamp_get(&end_timestamp, timestamp_end_str);
		// Song-ProgressBar
		if (GuiSliderBar((Rectangle) {(int) (width / 4), height - 30, (int) (width / 2), 10},
				timestamp_cur_str, timestamp_end_str, &current_slider_pos, 0, current_song_len)) {
			
			seeking = 2;
		}

		UpdateMusicStream(current_song);
		if (!seeking) {
			current_slider_pos = current_song_pos;
		}
		
		else if (seeking == 1 && IsMouseButtonUp(0)) {
			if (current_song.stream.buffer != NULL) {
				SeekMusicStream(current_song, current_slider_pos);
				--seeking;
			}
		}
		else if (seeking > 1) {
			--seeking;
		}
		// Play Button
		if (GuiButton((Rectangle) {(int) (width / 2) - 15, height - 75, 30, 30}, "")) {
			if (is_playing)
				PauseMusicStream(current_song);
			else
				ResumeMusicStream(current_song);
		}

		// skip Button
		if (GuiButton((Rectangle) {(int) (width / 2) + 50 - 15, height - 75, 30, 30}, "")) {
			;	
		}
		// back Button
		if (GuiButton((Rectangle) {(int) (width / 2) - 50 - 15, height - 75, 30, 30}, "")) {
			;	
		}
		// TODO implement textures or icons
		
		if (is_playing)
			DrawTexture(txt_playing, (int) (width / 2) - 15, height - 75, WHITE);
		else
			DrawTexture(txt_paused, (int) (width / 2) - 15, height - 75, WHITE);
		
		// Add Playlist Button
		if (GuiButton((Rectangle){5, 110, 40, 20}, "+")) {
			printf("adding new Playlist\n");
			uint8_t adding_playlist_toggle = 1;
		}

		if (adding_playlist_toggle) {
			;
		}
		if (cur_song != NULL) {
			GuiLabel(CLITERAL(Rectangle) {20, height - 60, 250, 20}, cur_song->name);
			GuiLabel(CLITERAL(Rectangle) {20, height - 40, 250, 20}, cur_song->artist);
		}
		EndDrawing();
	}

	CloseWindow();
	return 0;
}

