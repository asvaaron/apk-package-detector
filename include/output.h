/*
 * output.h
 *
 * Result reporting: probing installed tools and printing the results,
 * either as JSON (script-friendly) or as a human-readable table.
 */

#ifndef PKG_LANG_INFO_OUTPUT_H
#define PKG_LANG_INFO_OUTPUT_H

#include <cstddef>

#include "tables.h"

struct result {
    int found;
    char version[256];
};

/* Probe one tool: is it installed? what does it report as its version? */
struct result *probe(const char *binary, const char *version_cmd,
                     const char *filter);

/* Print all results as JSON (script-friendly). */
void print_json(const struct pkg_mgr *mgrs, std::size_t mgr_count,
                const struct lang *langs, std::size_t lang_count);

/* Print all results as a human-readable table (default output). */
void print_table(const struct pkg_mgr *mgrs, std::size_t mgr_count,
                 const struct lang *langs, std::size_t lang_count);

/* Print usage / help text. */
void print_usage(const char *prog);

#endif /* PKG_LANG_INFO_OUTPUT_H */
