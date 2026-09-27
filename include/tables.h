/*
 * tables.h
 *
 * The static tables describing which package managers and which
 * programming languages / runtimes the tool probes for.
 */

#ifndef PKG_LANG_INFO_TABLES_H
#define PKG_LANG_INFO_TABLES_H

#include <cstddef>

struct pkg_mgr {
    const char *name;
    const char *binary;
    const char *version_cmd;
};

struct lang {
    const char *name;
    const char *binary;
    const char *version_cmd;
    const char *filter; /* optional substring for the right version line */
};

/* Return the platform-appropriate package manager table. */
const struct pkg_mgr *get_pkg_mgrs(std::size_t *count);

/* Return the programming language / runtime table. */
const struct lang *get_langs(std::size_t *count);

#endif /* PKG_LANG_INFO_TABLES_H */
