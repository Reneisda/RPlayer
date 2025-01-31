#include "raylib.h"
#include <stdio.h>
#include "raygui.h"
#include <stdint.h>

#define APP_NAME "Player"
#define START_WIDTH 1200
#define START_HEIGHT 680
#define MIN_WIDTH 800
#define MIN_HEIGHT 600

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

int main() {
	int height, width;
	int refresh_rate = GetMonitorRefreshRate(GetCurrentMonitor());
	refresh_rate = refresh_rate == 0 ? 60 : refresh_rate;
	char search[256];
	printf("Rendering at %dfps\n", refresh_rate);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	SetWindowMinSize(MIN_WIDTH, MIN_HEIGHT);
	InitWindow(START_WIDTH, START_HEIGHT, APP_NAME);
	InitAudioDevice();
	SetTargetFPS(refresh_rate);
	GuiLoadStyle("assets/style_def.rgs");
	Font fontBm = LoadFontEx("assets/UbuntuSansNerdFont-Bold.ttf", 36, 0, 250);
	GuiSetFont(fontBm);
	FilePathList files = LoadDirectoryFiles("./songs");
	Music current_song = {0}; 
	
	float current_song_len = 0;
	float current_song_pos = 0;
	timestamp_t cur_timestamp = {.hours = 0, .mins = 0, .secs = 0};
	timestamp_t end_timestamp = {.hours = 0, .mins = 0, .secs = 0};
	
	timestamp_set(&cur_timestamp, 0.f);
	timestamp_set(&end_timestamp, 0.f);
	SetMasterVolume(0.05f);
	int scroll = 0; 
	int cur_playlist_size = files.count;
	while (!WindowShouldClose()) {
		BeginDrawing();
		width = GetScreenWidth();
		height = GetScreenHeight();
		if (width < MIN_WIDTH)															// Enforce Minimum Window size
			width = MIN_WIDTH;
		if (height < MIN_HEIGHT)
			height = MIN_HEIGHT;
		
		SetWindowSize(width, height);


		ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
		GuiPanel((Rectangle) {0, 0, width, 70}, "RPlayer");									// TopPanel
		GuiPanel((Rectangle) {0, 80, 260, height - 190}, "Playlists");						// Sidepanel
		GuiPanel((Rectangle) {270, 80, (width - 270 * 2), height - 190}, "Songs");			// SongPanel
		GuiTextBox((Rectangle) {270, 30, width - 270 * 2, 25}, search, 256, true);			// SearchBar
		GuiScrollBar((Rectangle) {width - 265, 80, 10, height - 190}, scroll, 0, cur_playlist_size * 10);
		DrawText("Search: ", 180, 34, 20, GetColor(0xFFFFFFFF));
		float scroll_wheel = GetMouseWheelMove();
		
		if (scroll_wheel < 0 && (scroll / 10) < cur_playlist_size - 1)						// scroll songs
			scroll += 10;
		else if (scroll_wheel > 0 && scroll >= 10)
			scroll -= 10;
	
		for (int i = 0; i + scroll / 10 < files.count; ++i) {
			if (110 + i * 45 + 40 > height - 120)
				break;

			if (GuiButton((Rectangle) {280, 110 + i * 45, width - 280 * 2 , 40} , get_file_name(files.paths[i + scroll / 10]))) {
				current_song = LoadMusicStream(files.paths[i + scroll / 10]);
				PlayMusicStream(current_song);
				current_song_len = GetMusicTimeLength(current_song);
				timestamp_set(&end_timestamp, current_song_len);
			}
		}
		if (IsKeyPressed(KEY_SPACE)) {
			if (IsMusicStreamPlaying(current_song)) {
				PauseMusicStream(current_song);
			}
			else {
				ResumeMusicStream(current_song);
			}
		}

		current_song_pos = GetMusicTimePlayed(current_song);
		timestamp_set(&cur_timestamp, current_song_pos);
		char timestamp_cur_str[16];
		char timestamp_end_str[16];
		timestamp_get(&cur_timestamp, timestamp_cur_str);
		timestamp_get(&end_timestamp, timestamp_end_str);
		
		GuiProgressBar((Rectangle) {(int) (width / 4), height - 60, (int) (width / 2), 20},
				timestamp_cur_str, timestamp_end_str, &current_song_pos, 0, current_song_len);

		UpdateMusicStream(current_song);
		
	EndDrawing();
	}

	CloseWindow();
	return 0;
}

