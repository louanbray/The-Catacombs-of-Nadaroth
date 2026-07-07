#ifdef _WIN32

#include "win_compat.h"

#include <stdlib.h>
#include <windows.h>

/// @brief Reentrant pseudo-random number generator (glibc-style LCG),
/// standing in for the POSIX rand_r() that mingw's C library lacks.
/// Not cryptographically secure - fine for gameplay RNG, matches how
/// this project already used rand_r() elsewhere.
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

/// @brief Minimal getline() replacement for mingw, which lacks it.
/// Reads a line (including the trailing '\n' if present) into a
/// dynamically-grown buffer, matching POSIX getline() semantics closely
/// enough for typical text-parsing use (return value, -1 on EOF/error,
/// buffer null-terminated, *lineptr/*n reused across calls).
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
        return -1;  // nothing read, hit EOF immediately
    }

    (*lineptr)[pos] = '\0';
    return (ssize_t)pos;
}

#endif  // _WIN32

#ifdef _WIN32

wchar_t* portable_fgetws(wchar_t* buffer, int max_chars, FILE* file) {
    // Read the raw line as bytes first (fgets does no locale conversion),
    // then decode it explicitly as UTF-8 -> UTF-16 via the Win32 API. This
    // sidesteps mingw's CRT, which cannot reliably decode UTF-8 through
    // fgetws()/setlocale() the way glibc can on Linux.
    char narrow_buf[4096];
    if (fgets(narrow_buf, sizeof(narrow_buf), file) == NULL) {
        return NULL;
    }

    int written = MultiByteToWideChar(CP_UTF8, 0, narrow_buf, -1, buffer, max_chars);
    if (written == 0) {
        // Malformed UTF-8 or buffer too small - return an empty line
        // rather than leaving `buffer` uninitialized/crashing.
        buffer[0] = L'\0';
    }

    return buffer;
}

#else
#include <stddef.h>
#include <stdio.h>
#include <wchar.h>
wchar_t* portable_fgetws(wchar_t* buffer, int max_chars, FILE* file) {
    return fgetws(buffer, max_chars, file);
}

#endif  // _WIN32