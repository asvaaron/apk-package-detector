/*
 * main.cpp
 *
 * Entry point for pkg_lang_info: parses the command line, probes every
 * known package manager and language runtime, and prints the results.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "helpers.h"
#include "output.h"
#include "tables.h"

int main(int argc, char **argv)
{
    int json = 0;

    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--json") == 0 ||
            std::strcmp(argv[i], "-j") == 0) {
            json = 1;
        } else if (std::strcmp(argv[i], "--help") == 0 ||
                   std::strcmp(argv[i], "-h") == 0) {
            print_usage("pkg_lang_info");
            return 0;
        } else {
            std::fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage("pkg_lang_info");
            return 1;
        }
    }

    print_os_info();

    std::size_t mgr_count = 0, lang_count = 0;
    const struct pkg_mgr *mgrs = get_pkg_mgrs(&mgr_count);
    const struct lang *langs = get_langs(&lang_count);

    /* Probe managers first, then languages (order matters for results). */
    for (std::size_t i = 0; i < mgr_count; i++)
        probe(mgrs[i].binary, mgrs[i].version_cmd, nullptr);
    for (std::size_t i = 0; i < lang_count; i++)
        probe(langs[i].binary, langs[i].version_cmd, langs[i].filter);

    if (json)
        print_json(mgrs, mgr_count, langs, lang_count);
    else
        print_table(mgrs, mgr_count, langs, lang_count);

    return 0;
}
