#include <stdio.h>
#include <time.h>
#include "logger.h"
#include <string.h>

#define LOG_FILE "mpcc.log"

FILE *log_file;

void init_logger()
{
    log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL)
    {
        fprintf(stderr, "Error opening log file\n");
    }
}

void log_message(const char *level, const char *message)
{
    time_t now;
    time(&now);
    char *date = ctime(&now);
    date[strcspn(date, "\n")] = 0;

    fprintf(log_file, "[%s] %s: %s\n", date, level, message);
    fflush(log_file);

    printf("[%s] %s: %s\n", date, level, message);
}

void log_fatal(const char *message)
{
    log_message("FATAL", message);
}

void log_error(const char *message)
{
    log_message("ERROR", message);
}

void log_warning(const char *message)
{
    log_message("WARNING", message);
}

void log_info(const char *message)
{
    log_message("INFO", message);
}

void log_debug(const char *message)
{
    log_message("DEBUG", message);
}