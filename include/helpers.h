/*
 * helpers.h
 *
 * Small shared utilities for pkg_lang_info:
 *   - in_path()       PATH lookup for executables
 *   - first_line()    run a command, capture its first (filtered) output line
 *   - json_escape()   escape a string for safe use inside JSON literals
 *   - print_os_info() print the current system/kernel line
 */

#ifndef PKG_LANG_INFO_HELPERS_H
#define PKG_LANG_INFO_HELPERS_H

#include <cstddef>

/* Return 1 if cmd exists as an executable somewhere on $PATH. */
int in_path(const char *cmd);

/*
 * Run a shell command and return the first non-empty output line.
 * If filter is non-NULL, return the first line CONTAINING that substring
 * (useful when a tool prints extra lines before the version).
 * Returns 1 on success.
 */
int first_line(const char *command, const char *filter,
               char *out, std::size_t out_size);

/* Escape s for safe use inside a JSON string literal. */
void json_escape(const char *s, char *out, std::size_t out_size);

/* Print the current system/kernel info line. */
void print_os_info(void);

#endif /* PKG_LANG_INFO_HELPERS_H */
