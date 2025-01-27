#include "raylib.h"
#include <stdio.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <stdint.h>

#define APP_NAME "Player"
#define START_WIDTH 1200
#define START_HEIGHT 800
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
	t->hours = 	i / 360;
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
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	SetWindowMinSize(MIN_WIDTH, MIN_HEIGHT);
	InitWindow(START_WIDTH, START_HEIGHT, APP_NAME);
	InitAudioDevice();
	SetTargetFPS(144);
	GuiLoadStyle("assets/style_def.rgs");
	Font fontBm = LoadFontEx("assets/UbuntuSansNerdFont-Bold.ttf", 36, 0, 250);
	GuiSetFont(fontBm);

	FilePathList files = LoadDirectoryFiles("./songs");
	Music current_song ={0}; 

	float current_song_len = 0;
	float current_song_pos = 0;
	timestamp_t cur_timestamp = {.hours = 0, .mins = 0, .secs = 0};
	timestamp_t end_timestamp = {.hours = 0, .mins = 0, .secs = 0};

	timestamp_set(&cur_timestamp, 0.f);
	timestamp_set(&end_timestamp, 0.f);
	SetMasterVolume(0.05f);

	while (!WindowShouldClose()) {
		BeginDrawing();
		width = GetScreenWidth();
		height = GetScreenHeight();
		if (width < MIN_WIDTH)
			width = MIN_WIDTH;
		if (height < MIN_HEIGHT)
			height = MIN_HEIGHT;
		
		SetWindowSize(width, height);

		ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
		GuiPanel((Rectangle) {0, 0, 260, height - 120}, "Playlists");					// sidepanel
		GuiPanel((Rectangle) {270, 0, (width - 270 * 2), height - 120}, "Songs");	
		for (int i = 0; i < files.count; ++i) 
			if (GuiButton((Rectangle) {4, 40 + i * 45, 250 , 40} , get_file_name(files.paths[i]))) {
				current_song = LoadMusicStream(files.paths[i]);
				PlayMusicStream(current_song);
				current_song_len = GetMusicTimeLength(current_song);
				timestamp_set(&end_timestamp, current_song_len);
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

