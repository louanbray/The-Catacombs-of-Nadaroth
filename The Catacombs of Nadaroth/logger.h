#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

// Log file handle - defined in logger.c
extern FILE* g_log_file;

// Logging macros that write to the log file
#define LOG_INFO(fmt, ...)  do { if (g_log_file) {time_t now = time(NULL); struct tm* tm_info = localtime(&now); char timestamp[20]; strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info); fprintf(g_log_file, "[%s][INFO][%s:%d] " fmt "\n", timestamp, __FILE__, __LINE__, ##__VA_ARGS__); fflush(g_log_file);}} while(0)
#define LOG_WARN(fmt, ...)  do { if (g_log_file) {time_t now = time(NULL); struct tm* tm_info = localtime(&now); char timestamp[20]; strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info); fprintf(g_log_file, "[%s][WARN][%s:%d] " fmt "\n", timestamp, __FILE__, __LINE__, ##__VA_ARGS__); fflush(g_log_file);}} while(0)
#define LOG_ERROR(fmt, ...) do { if (g_log_file) {time_t now = time(NULL); struct tm* tm_info = localtime(&now); char timestamp[20]; strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info); fprintf(g_log_file, "[%s][ERROR][%s:%d] " fmt "\n", timestamp, __FILE__, __LINE__, ##__VA_ARGS__); fflush(g_log_file);}} while(0)

void init_logger();
void close_logger();

#endif