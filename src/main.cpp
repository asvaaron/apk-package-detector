/*
 * main.cpp
 *
 * pkg_lang_info — Unix-based tool (Linux, macOS, *BSD) that shows:
 *   1. Available package managers (Homebrew, apt, dnf, pacman, ...)
 *   2. Installed programming languages / runtimes with their versions
 *
 * Options:
 *   --json, -j    Print results as JSON (script-friendly)
 *   --help, -h    Show usage
 *
 * Build:  cmake -B build && cmake --build build
 * Run:    ./build/bin/pkg_lang_info [--json]
 */

#include <cstdio>
#include <cstring>

#include "output.h"
#include "tables.h"

int main(int argc, char **argv)
{
    /* JSON Flag to activate --json -j */
    int want_json = 0;

    for (int i = 1; i < argc; i++) {
        /* Json --json -j */
        if (strcmp(argv[i], "--json") == 0 || strcmp(argv[i], "-j") == 0) {
            want_json = 1;
        /* Help --help -h  */
        } else if (strcmp(argv[i], "--help") == 0 ||
                   strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "error: unknown option '%s'\n\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    std::size_t mgr_count, lang_count;
    const struct pkg_mgr *mgrs = get_pkg_mgrs(&mgr_count);
    const struct lang *langs = get_langs(&lang_count);

    /* JSON mode */
    if (want_json) {
        print_json(mgrs, mgr_count, langs, lang_count);
        return 0;
    }

    /* Table mode (default) */
    print_table(mgrs, mgr_count, langs, lang_count);
    return 0;
}
