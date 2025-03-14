#include "utils.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>


#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif
#ifdef linux
#include <unistd.h>
#endif

#define PRE_ALLOCATED_LINES 255
#define PRE_ALLOCATED_CHARS 255

static char* strremove(char* str, const char* sub) {
    size_t len = strlen(sub);
    if (len > 0) {
        char* p = str;
        while ((p = strstr(p, sub)) != NULL) {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
    return str;
}


char* utils_read_whole_file(const char* filepath) {
	if (access(filepath, F_OK) != 0) {
		return NULL;
	}
	
	FILE *file = fopen(filepath, "r");
    if (!file)
        return NULL;

    fseek(file, 0L, SEEK_END);
    uint32_t file_size = ftell(file);
    rewind(file);

    char *text = malloc(file_size + 1);
    if (!text) {
        fclose(file);
        return NULL;
    }

    if (fread(text, 1, file_size, file) != file_size) {
        free(text);
        fclose(file);
        return NULL;
    }

    text[file_size] = '\0';
    fclose(file);
    return text;   
}

lines_t* utils_read_lines(const char* filepath) {
	char* data = utils_read_whole_file(filepath);
	if (data == NULL) {
		fprintf(stderr, "could not read file\n");
		return NULL;
	}

	lines_t* lines_ = malloc(sizeof(lines_t));
	lines_->lines = malloc(sizeof(char*) * PRE_ALLOCATED_LINES);

	uint32_t current_line_count = 0;
	uint32_t line_length = 0;
	uint8_t line_beginn = 1;
	char c;
	
	uint32_t length = strlen(data);
	for (uint32_t i = 0; i < length; ++i) {
		if (line_beginn) {
			lines_->lines[current_line_count] = data + i;
			line_beginn = 0;
		}

		if (current_line_count >= PRE_ALLOCATED_LINES) {
			lines_->lines = realloc(lines_->lines, sizeof(char) * current_line_count * 2);
			if (lines_->lines == NULL)
				return NULL;
		}		
		if (data[i] != '\n') ++line_length;
		else {
			data[i] = 0;
			++current_line_count;
			lines_->lines = realloc(lines_->lines, ((data + i) - lines_->lines[i]) + 1);
			if (lines_->lines == NULL)
				return NULL;

			memcpy(lines_->lines[current_line_count], lines_->lines[i], ((data + i) - lines_->lines[i]));
		}
	}
	
	lines_->count = current_line_count;
	free(data);
	return lines_;
}

config_t* utils_read_config() {
	const char* homedir;
	if ((homedir = getenv("HOME")) == NULL) {
	    homedir = getpwuid(getuid())->pw_dir;
	}

	if (homedir == NULL) {
		fprintf(stderr, "Failed to get home directory!\n");
		return NULL;
	}

	config_t* cfg = (config_t*) malloc(sizeof(config_t));
	char config_path[2048];
	strcpy(config_path, homedir);
	if (config_path[strlen(config_path) - 1] != '/')
		strcat(config_path, "/");

	strcat(config_path, CONFIG_PATH); 
	char* buffer = utils_read_whole_file(config_path);
	if (buffer == NULL)
		return NULL;

	char* end_ptr;
	end_ptr = strstr(buffer, "volume: ");
	if (end_ptr != NULL) {
		end_ptr = strstr(end_ptr, ":");
		cfg->volume = strtof(end_ptr + 1, &end_ptr);
	}

	end_ptr = strstr(buffer, "music_location: ");
	if (end_ptr != NULL) {
		char* start = strstr(end_ptr, ":") + 1;
		end_ptr = strstr(buffer, "\n");
		if (end_ptr != NULL)
			*end_ptr = 0;
		
		strremove(start, "\n");
		strremove(start, "\r");
		strremove(start, " ");

		printf("read path: >%s<\n", start);	
		strcpy(cfg->base_dir, start);
		*end_ptr = '\n';
	}
	free(buffer);
	return cfg;
}

float utils_get_volume(config_t* config) {
	return config->volume;
}

int main() {
	lines_t* l = utils_read_lines("~/Music/rplayer/songs/songs.info");
	if (l == NULL)
		return 1;
	for (int i = 0; i < l->count; ++i)
		printf("%s\n", l->lines[i]);

	return 0;
}
