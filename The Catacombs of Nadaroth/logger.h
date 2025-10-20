#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

// Log file handle - defined in logger.c
extern FILE* g_log_file;

// Logging macros that write to the log file
#define LOG_INFO(fmt, ...)  do { if (g_log_file) fprintf(g_log_file, "[INFO] " fmt "\n", ##__VA_ARGS__); fflush(g_log_file); } while(0)
#define LOG_WARN(fmt, ...)  do { if (g_log_file) fprintf(g_log_file, "[WARN] " fmt "\n", ##__VA_ARGS__); fflush(g_log_file); } while(0)
#define LOG_ERROR(fmt, ...) do { if (g_log_file) fprintf(g_log_file, "[ERROR] " fmt "\n", ##__VA_ARGS__); fflush(g_log_file); } while(0)

void init_logger();
void close_logger();

#endif