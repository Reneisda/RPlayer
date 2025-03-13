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


char* strremove(char* str, const char* sub) {
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
    int file_size = ftell(file);
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


