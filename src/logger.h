#ifndef LOGGER_H
#define LOGGER_H

void init_logger();
void log_fatal(const char *message);
void log_error(const char *message);
void log_warning(const char *message);
void log_info(const char *message);
void log_debug(const char *message);

#endif