#ifndef SYS_PLATFORM_H
#define SYS_PLATFORM_H

#include <stdbool.h>
#include <stdio.h>
#include <wchar.h>

#ifdef _WIN32
#include <sys/types.h>

#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
typedef long long ssize_t;
#endif

int rand_r(unsigned int* seed);

ssize_t getline(char** lineptr, size_t* n, FILE* stream);

#endif  // _WIN32

// -----------------------------------------------------------------------------
// Cross-Platform Abstraction API
// -----------------------------------------------------------------------------

/// @brief Uses the appropriate method to read a line from a file into a wide-character buffer, using Windows API on Windows and fgetws() on other platforms.
wchar_t* portable_fgetws(wchar_t* buffer, int max_chars, FILE* file);

/// @brief Sleeps for the specified number of milliseconds, using Windows API on Windows and nanosleep() on other platforms.
void sys_sleep_ms(int ms);

/// @brief Creates a directory at the specified path, using Windows API on Windows and mkdir() on other platforms. Returns 0 on success, or -1 on failure.
int sys_mkdir(const char* path);

/// @brief Checks if a directory exists at the specified path, using Windows API on Windows and stat() on other platforms.
bool directory_exists(const char* path);

#endif  // SYS_PLATFORM_H