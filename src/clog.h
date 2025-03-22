#ifndef __CLOG__
#define __CLOG__
// Alias for Error
#ifdef LOG_LEVEL_ERROR
#define LOG_LEVEL_ERR
#endif

#ifdef LOG_LEVEL_ALL
#define LOG_LEVEL_INFO
#endif

// Sets LOG_LEVEL_INFO as default if nothing other is provided
#if !defined(LOG_LEVEL_ERR) && !defined(LOG_LEVEL_WARN) && !defined(LOG_LEVEL_INFO) && !defined(LOG_LEVEL_NONE)

#define LOG_LEVEL_INFO
#endif

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef LOG_NO_TIMESTAMPS
#include <time.h>
#endif


/* LOG LEVEL
 * LOG_LEVEL_NONE                -> No Logging
 * LOG_LEVEL_ERR/ERROR           -> only errors
 * LOG_LEVEL_WARN                -> only warnings, errors
 * LOG_LEVEL_INFO                -> warnings, errors, info
 */

#define STDOUT                  0
#define STDERR                  1
#define __CLOG_NEWLINE          &__new_line
#define __START_BYTE            27
#define __FINAL_BYTE            109
#define __COMMAND_COLOR         91

// colors
#define CLOG_COLOR_ERR          __color_err
#define CLOG_COLOR_INFO         __color_info
#define CLOG_COLOR_WARN         __color_warn
#define CLOG_COLOR_EXTRA        __color_extra

#define CLOG_RESET_COLOR        __color_reset

void clog_info(const char* message);
void clog_error(const char* message);
void clog_warn(const char* message);
void clog_extra(const char* message);

void clogf_info(const char* format, ...);
void clogf_error(const char* format, ...);
void clogf_warn(const char* format, ...);
void clogf_extra(const char* format, ...);

uint32_t clog_err_count();
uint32_t clog_warn_count();
uint32_t clog_info_count();
uint32_t clog_extra_count();

#endif


#ifdef CLOG_IMPLEMENTATION


#ifndef LOG_NO_TIMESTAMPS
static char time_f[100];

static void update_time() {
    time_t now = time(0);
    char tmp[sizeof(time_f)];
    strftime(tmp, sizeof(tmp), "[%Y-%m-%d %H:%M:%S]", localtime (&now));
    snprintf(time_f, sizeof(time_f), "%-24s", tmp);
}
#endif

static char __new_line = '\n';
static char __color_err[] = {__START_BYTE, __COMMAND_COLOR, '3', '1', __FINAL_BYTE};
static char __color_info[] = {__START_BYTE, __COMMAND_COLOR, '3', '7', __FINAL_BYTE};
static char __color_warn[] = {__START_BYTE, __COMMAND_COLOR, '3', '3', __FINAL_BYTE};
static char __color_extra[] = {__START_BYTE, __COMMAND_COLOR, '3', '6', __FINAL_BYTE};

static char __color_reset[] = {__START_BYTE, __COMMAND_COLOR, '0', __FINAL_BYTE};


static uint32_t errors;
static uint32_t infos;
static uint32_t warnings;
static uint32_t extras;

static size_t len(const char* message) {
    char* p = (char*) message;
    while (*p++ != '\0');
    return p - message;
}

#ifndef LOG_COLORS_ONLY
#define SHOW_LOG_LEVEL(level) do { \
    write(STDOUT, level, len(level)); \
} while (0)
#else
#define SHOW_LOG_LEVEL(level)
#endif

#ifndef LOG_NO_TIMESTAMPS
#define LOG_TIME() do { \
    update_time(); \
    write(STDOUT, time_f, len(time_f)); \
} while (0)
#else
#define LOG_TIME()
#endif


void clog_info(const char* message) {
    #ifdef LOG_LEVEL_INFO
    ++infos;
    write(STDOUT, CLOG_COLOR_INFO, sizeof(CLOG_COLOR_INFO));
    LOG_TIME();
    SHOW_LOG_LEVEL("INFO    ");
    write(STDOUT, message, len(message));
    write(STDOUT, CLOG_RESET_COLOR, sizeof(CLOG_RESET_COLOR));
    write(STDOUT, __CLOG_NEWLINE, 1);
    fflush(stdout);
    #endif
}

void clog_error(const char* message) {
    #if defined(LOG_LEVEL_ERR) || defined(LOG_LEVEL_WARN) || defined(LOG_LEVEL_INFO) 
    ++errors;
    write(STDOUT, CLOG_COLOR_ERR, sizeof(CLOG_COLOR_ERR));
    LOG_TIME();
    SHOW_LOG_LEVEL("ERROR   ");
    write(STDOUT, message, len(message));
    write(STDOUT, CLOG_RESET_COLOR, sizeof(CLOG_RESET_COLOR));
    write(STDOUT, __CLOG_NEWLINE, 1);
    fflush(stdout);
    #endif
}

void clog_warn(const char* message) {
    #if defined(LOG_LEVEL_WARN) || defined(LOG_LEVEL_INFO) 
    ++warnings;
    write(STDOUT, CLOG_COLOR_WARN, sizeof(CLOG_COLOR_WARN));
    LOG_TIME();
    SHOW_LOG_LEVEL("WARN    ");
    write(STDOUT, message, len(message));
    write(STDOUT, CLOG_RESET_COLOR, sizeof(CLOG_RESET_COLOR));
    write(STDOUT, __CLOG_NEWLINE, 1);
    fflush(stdout);
    #endif
}

void clog_extra(const char* message) {
    #ifdef LOG_LEVEL_INFO 
    ++extras;
    write(STDOUT, CLOG_COLOR_EXTRA, sizeof(CLOG_COLOR_EXTRA));
    LOG_TIME();
    SHOW_LOG_LEVEL("EXTRA   ");
    write(STDOUT, message, len(message));
    write(STDOUT, CLOG_RESET_COLOR, sizeof(CLOG_RESET_COLOR));
    write(STDOUT, __CLOG_NEWLINE, 1);
    fflush(stdout);
    #endif
}

void clogf_info(const char* format, ...) {
    #ifdef LOG_LEVEL_INFO
    va_list arg;
    ++infos;
    write(STDOUT, CLOG_COLOR_INFO, sizeof(CLOG_COLOR_INFO));
    LOG_TIME();
    SHOW_LOG_LEVEL("INFO    ");
    va_start(arg, format);
    vfprintf(stdout, format, arg);
    write(STDOUT, CLOG_RESET_COLOR, sizeof(CLOG_RESET_COLOR));
    fflush(stdout);
    va_end(arg);
    #endif
}

void clogf_error(const char* format, ...) {
    #if defined(LOG_LEVEL_ERR) || defined(LOG_LEVEL_WARN) || defined(LOG_LEVEL_INFO) 
    va_list arg;
    ++errors;
    write(STDOUT, CLOG_COLOR_ERR, sizeof(CLOG_COLOR_ERR));
    LOG_TIME();
    SHOW_LOG_LEVEL("ERROR   ");
    va_start(arg, format);
    vfprintf(stdout, format, arg);
    write(STDOUT, CLOG_RESET_COLOR, sizeof(CLOG_RESET_COLOR));
    fflush(stdout);
    va_end(arg);
    #endif
}

void clogf_warn(const char* format, ...) {
    #if defined(LOG_LEVEL_WARN) || defined(LOG_LEVEL_INFO) 
    va_list arg;
    ++warnings;
    write(STDOUT, CLOG_COLOR_WARN, sizeof(CLOG_COLOR_WARN));
    LOG_TIME();
    SHOW_LOG_LEVEL("WARN    ");
    va_start(arg, format);
    vfprintf(stdout, format, arg);
    write(STDOUT, CLOG_RESET_COLOR, sizeof(CLOG_RESET_COLOR));
    fflush(stdout);
    va_end(arg);
    #endif
}

void clogf_extra(const char* format, ...) {
    #ifdef LOG_LEVEL_INFO 
    va_list arg;
    ++extras;
    write(STDOUT, CLOG_COLOR_EXTRA, sizeof(CLOG_COLOR_EXTRA));
    LOG_TIME();
    SHOW_LOG_LEVEL("EXTRA   ");
    va_start(arg, format);
    vfprintf(stdout, format, arg);
    write(STDOUT, CLOG_RESET_COLOR, sizeof(CLOG_RESET_COLOR));
    fflush(stdout);
    va_end(arg);
    #endif
}

uint32_t clog_err_count() {
    return errors; 
}

uint32_t clog_warn_count() {
    return warnings;
}

uint32_t clog_info_count() {
    return infos;
}

uint32_t clog_extra_count() {
    return extras;
}

#endif

