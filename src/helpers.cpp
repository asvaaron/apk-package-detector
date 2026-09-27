/*
 * helpers.cpp
 *
 * Implementation of the small shared utilities declared in helpers.h.
 */

/* Expose the POSIX APIs used below (popen, strtok_r, access, uname).
 * Harmless when the compiler already defines it (default gnu++ mode). */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "helpers.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/utsname.h>
#include <unistd.h>

/* Return 1 if cmd exists as an executable somewhere on $PATH. */
int in_path(const char *cmd)
{
    if (cmd == nullptr || *cmd == '\0')
        return 0;

    const char *path_env = getenv("PATH");
    if (path_env == nullptr || *path_env == '\0')
        return access(cmd, X_OK) == 0; /* no PATH set: try relative */

    char buf[4096];
    if (snprintf(buf, sizeof buf, "%s", path_env) >= (int)sizeof buf)
        return 0;

    char *save = nullptr;
    for (char *dir = strtok_r(buf, ":", &save); dir != nullptr;
         dir = strtok_r(nullptr, ":", &save)) {
        char full[4096 + 256];
        int n = snprintf(full, sizeof full, "%s/%s", dir, cmd);
        if (n < 0 || (std::size_t)n >= sizeof full)
            continue;
        if (access(full, X_OK) == 0)
            return 1;
    }
    return 0;
}

/*
 * Run a shell command and return the first non-empty output line.
 * If filter is non-NULL, return the first line CONTAINING that substring
 * (useful when a tool prints extra lines before the version).
 * Returns 1 on success.
 */
int first_line(const char *command, const char *filter,
               char *out, std::size_t out_size)
{
    out[0] = '\0';
    FILE *fp = popen(command, "r");
    if (fp == nullptr)
        return 0;

    char line[512];
    int found = 0;
    while (!found && fgets(line, sizeof line, fp) != nullptr) {
        /* strip trailing whitespace */
        std::size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r' ||
                           line[len - 1] == ' '  || line[len - 1] == '\t'))
            line[--len] = '\0';

        if (len == 0)
            continue;
        if (filter != nullptr && strstr(line, filter) == nullptr)
            continue;

        snprintf(out, out_size, "%s", line);
        found = 1;
    }
    pclose(fp);
    return found;
}

/* Escape s for safe use inside a JSON string literal. */
void json_escape(const char *s, char *out, std::size_t out_size)
{
    std::size_t o = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
        const char *rep = nullptr;
        char esc[8];

        switch (*p) {
        case '"':  rep = "\\\""; break;
        case '\\': rep = "\\\\"; break;
        case '\b': rep = "\\b";  break;
        case '\f': rep = "\\f";  break;
        case '\n': rep = "\\n";  break;
        case '\r': rep = "\\r";  break;
        case '\t': rep = "\\t";  break;
        default:
            if (*p < 0x20) {            /* other control chars -> \u00XX */
                snprintf(esc, sizeof esc, "\\u%04x", *p);
                rep = esc;
            }
            break;
        }

        if (rep == nullptr) {           /* plain byte */
            if (o + 1 > out_size)
                break;
            out[o++] = (char)*p;
            continue;
        }

        std::size_t len = strlen(rep);
        if (o + len > out_size)
            break;
        memcpy(out + o, rep, len);
        o += len;
    }
    out[o] = '\0';
}

/* Print the current system/kernel info line. */
void print_os_info(void)
{
    struct utsname u;
    if (uname(&u) == 0)
        printf("System: %s %s (%s)\n", u.sysname, u.release, u.machine);
    else
        printf("System: unknown (uname failed)\n");
}
