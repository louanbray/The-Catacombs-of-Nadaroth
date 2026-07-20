#include "sys_platform.h"

// -----------------------------------------------------------------------------
// Header
// -----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#endif
#ifdef _WIN32
#include <direct.h>

#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
typedef LONG_PTR ssize_t;
#endif
#else
#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <time.h>
#include <wchar.h>
#endif

// -----------------------------------------------------------------------------
// Windows specifics
// -----------------------------------------------------------------------------
#ifdef _WIN32

/// @brief Reentrant pseudo-random number generator (glibc-style LCG),
/// standing in for the POSIX rand_r() that MinGW's C library lacks.
int rand_r(unsigned int* seed) {
    unsigned int next = *seed;
    int result;

    next = next * 1103515245 + 12345;
    result = (unsigned int)(next / 65536) % 2048;

    next = next * 1103515245 + 12345;
    result <<= 10;
    result ^= (unsigned int)(next / 65536) % 1024;

    next = next * 1103515245 + 12345;
    result <<= 10;
    result ^= (unsigned int)(next / 65536) % 1024;

    *seed = next;
    return result;
}

/// @brief Minimal getline() replacement for MinGW, which lacks it.
ssize_t getline(char** lineptr, size_t* n, FILE* stream) {
    if (lineptr == NULL || n == NULL || stream == NULL) {
        return -1;
    }

    if (*lineptr == NULL || *n == 0) {
        *n = 128;
        *lineptr = malloc(*n);
        if (*lineptr == NULL) {
            return -1;
        }
    }

    size_t pos = 0;
    int c;

    while ((c = fgetc(stream)) != EOF) {
        if (pos + 1 >= *n) {
            size_t new_size = *n * 2;
            char* new_ptr = realloc(*lineptr, new_size);
            if (new_ptr == NULL) {
                return -1;
            }
            *n = new_size;
            *lineptr = new_ptr;
        }

        (*lineptr)[pos++] = (char)c;

        if (c == '\n') {
            break;
        }
    }

    if (pos == 0 && c == EOF) {
        return -1;  // Nothing read, hit EOF immediately
    }

    (*lineptr)[pos] = '\0';
    return (ssize_t)pos;
}

wchar_t* portable_fgetws(wchar_t* buffer, int max_chars, FILE* file) {
    // Read the raw line as bytes first (fgets does no locale conversion),
    // then decode it explicitly as UTF-8 -> UTF-16 via the Win32 API.
    char narrow_buf[4096];
    if (fgets(narrow_buf, sizeof(narrow_buf), file) == NULL) {
        return NULL;
    }

    int written = MultiByteToWideChar(CP_UTF8, 0, narrow_buf, -1, buffer, max_chars);
    if (written == 0) {
        buffer[0] = L'\0';
    }

    return buffer;
}

#else

wchar_t* portable_fgetws(wchar_t* buffer, int max_chars, FILE* file) {
    return fgetws(buffer, max_chars, file);
}

#endif  // _WIN32

// -----------------------------------------------------------------------------
// Cross-Platform Abstract Functions
// -----------------------------------------------------------------------------

void sys_sleep_ms(int ms) {
    if (ms <= 0) return;

#ifdef _WIN32
    Sleep((DWORD)ms);
#else
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
#endif
}

int sys_mkdir(const char* path) {
#ifdef _WIN32
    return _mkdir(path);
#else
    return mkdir(path, 0755);
#endif
}

bool directory_exists(const char* path) {
    struct stat st;

    if (stat(path, &st) != 0)
        return false;

#ifdef _WIN32
    return (st.st_mode & _S_IFDIR) != 0;
#else
    return S_ISDIR(st.st_mode);
#endif
}