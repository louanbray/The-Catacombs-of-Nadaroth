#ifndef WIN_COMPAT_H
#define WIN_COMPAT_H

#include <stdio.h>
#include <wchar.h>

// mingw's C library (msvcrt) doesn't provide a handful of POSIX functions
// that this project uses. Declarations + implementations live here, built
// only for the Windows target (see win_compat.c).
#ifdef _WIN32

#include <stdio.h>
#include <sys/types.h>  // ssize_t (mingw provides this)

int rand_r(unsigned int* seed);
ssize_t getline(char** lineptr, size_t* n, FILE* stream);

#endif  // _WIN32

// Reads one line from `file` as UTF-8 and decodes it into `buffer` as wide
// characters, regardless of the C runtime's locale support. mingw's CRT
// cannot reliably decode UTF-8 via fgetws()/locale, so on Windows this
// reads raw bytes and decodes them explicitly via the Win32 API instead.
// On non-Windows platforms this is a thin passthrough to fgetws(), assuming
// a UTF-8 locale is already active (as set by setlocale(LC_CTYPE, "")).
wchar_t* portable_fgetws(wchar_t* buffer, int max_chars, FILE* file);

#endif  // WIN_COMPAT_H