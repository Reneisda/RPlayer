#ifndef UTILS_H
#define UTILS_H

#define CONFIG_PATH ".config/rplayer/rplayer.conf"
#include <stdint.h>


typedef struct config {
	float volume;
	char base_dir[2048];
} config_t;


typedef struct lines {
	uint32_t count;
	char** lines;
} lines_t;


char* utils_read_whole_file(const char* filepath);
config_t* utils_read_config();
float utils_get_volume(config_t* config);
char* util_get_base_dir(config_t* config);


#endif

