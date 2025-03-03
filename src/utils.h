#ifndef UTILS_H
#define UTILS_H

#define CONFIG_PATH ".config/rplayer/rplayer.conf"


typedef struct config {
	float volume;
	char music_path[2048];
} config_t;


char* utils_read_whole_file(const char* filepath);
config_t* utils_read_config();
float utils_get_volume();


#endif
