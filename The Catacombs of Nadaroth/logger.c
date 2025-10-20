#include "logger.h"

#include <string.h>
#include <time.h>

FILE* g_log_file = NULL;

#define LOG_FILE_PATH "game.log"

void init_logger() {
    g_log_file = fopen(LOG_FILE_PATH, "a");

    if (g_log_file == NULL) {
        g_log_file = stdout;
        fprintf(stderr, "[WARN] Could not open log file, falling back to stdout\n");
        return;
    }

    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%d-%m-%Y %H:%M:%S", t);

    fprintf(g_log_file, "\n");
    fprintf(g_log_file, "================================================================================\n");
    fprintf(g_log_file, "Game session started at %s\n", timestamp);
    fprintf(g_log_file, "================================================================================\n");
    fflush(g_log_file);
}

void close_logger() {
    if (g_log_file != NULL && g_log_file != stdout && g_log_file != stderr) {
        time_t now = time(NULL);
        struct tm* t = localtime(&now);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%d-%m-%Y %H:%M:%S", t);

        fprintf(g_log_file, "================================================================================\n");
        fprintf(g_log_file, "Game session ended at %s\n", timestamp);
        fprintf(g_log_file, "================================================================================\n\n");

        fclose(g_log_file);
        g_log_file = NULL;
    }
}
