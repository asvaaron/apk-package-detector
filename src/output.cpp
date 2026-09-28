/*
 * output.cpp
 *
 * Probing and display logic for pkg_lang_info: runs each version command,
 * remembers the results, and renders them as a table or JSON.
 */

#include "output.h"
#include "tables.h"
#include "helpers.h"

#include <cstdio>

namespace {

constexpr int kMaxResults = 128;

/* Results are stored in call order: managers first, then languages. */
struct result g_results[kMaxResults];
int g_n_results = 0;

/* Return the result recorded at index i, or nullptr if out of range. */
const struct result *result_at(std::size_t i)
{
    return (i < static_cast<std::size_t>(g_n_results)) ? &g_results[i] : nullptr;
}

} // namespace

/*
 * Probe one tool: check the binary is on $PATH, then run its version
 * command and capture the first matching line. The result is appended to
 * the internal table and a pointer to it is returned.
 */
struct result *probe(const char *binary, const char *version_cmd,
                     const char *filter)
{
    struct result r;
    r.found = 0;
    r.version[0] = '\0';

    if (in_path(binary) != 0) {
        char line[512];
        if (first_line(version_cmd, filter, line, sizeof line) != 0) {
            r.found = 1;
            std::snprintf(r.version, sizeof r.version, "%s", line);
        }
    }

    if (g_n_results >= kMaxResults)
        return nullptr; /* table overflow: drop this entry */

    g_results[g_n_results] = r;
    return &g_results[g_n_results++];
}

/* Render the collected results as a JSON document. */
void print_json(const struct pkg_mgr *mgrs, std::size_t n_mgrs,
                const struct lang *langs, std::size_t n_langs)
{
    std::printf("{\n");

    std::printf("  \"package_managers\": [\n");
    for (std::size_t i = 0; i < n_mgrs; i++) {
        const struct result *r = result_at(i);
        char name[512], bin[512], ver[512];
        json_escape(mgrs[i].name, name, sizeof name);
        json_escape(mgrs[i].binary, bin, sizeof bin);
        json_escape((r != nullptr && r->found) ? r->version : "", ver, sizeof ver);
        std::printf("    { \"name\": \"%s\", \"binary\": \"%s\", "
                    "\"installed\": %s, \"version\": \"%s\" }%s\n",
                    name, bin, (r != nullptr && r->found) ? "true" : "false",
                    ver, (i + 1 < n_mgrs) ? "," : "");
    }
    std::printf("  ],\n");

    std::printf("  \"languages\": [\n");
    for (std::size_t i = 0; i < n_langs; i++) {
        const struct result *r = result_at(n_mgrs + i);
        char name[512], bin[512], ver[512];
        json_escape(langs[i].name, name, sizeof name);
        json_escape(langs[i].binary, bin, sizeof bin);
        json_escape((r != nullptr && r->found) ? r->version : "", ver, sizeof ver);
        std::printf("    { \"name\": \"%s\", \"binary\": \"%s\", "
                    "\"installed\": %s, \"version\": \"%s\" }%s\n",
                    name, bin, (r != nullptr && r->found) ? "true" : "false",
                    ver, (i + 1 < n_langs) ? "," : "");
    }
    std::printf("  ]\n");

    std::printf("}\n");
}

/* Render the collected results as an aligned text table. */
void print_table(const struct pkg_mgr *mgrs, std::size_t n_mgrs,
                 const struct lang *langs, std::size_t n_langs)
{
    std::printf("%-24s %-12s %s\n", "NAME", "BINARY", "VERSION");
    std::printf("------------------------------- -------------- "
                "------------------------------\n");

    for (std::size_t i = 0; i < n_mgrs; i++) {
        const struct result *r = result_at(i);
        if (r != nullptr && r->found)
            std::printf("%-24s %-12s %s\n", mgrs[i].name, mgrs[i].binary,
                        r->version);
        else
            std::printf("%-24s %-12s (not installed)\n", mgrs[i].name,
                        mgrs[i].binary);
    }

    std::printf("\n");

    for (std::size_t i = 0; i < n_langs; i++) {
        const struct result *r = result_at(n_mgrs + i);
        if (r != nullptr && r->found)
            std::printf("%-24s %-12s %s\n", langs[i].name, langs[i].binary,
                        r->version);
        else
            std::printf("%-24s %-12s (not installed)\n", langs[i].name,
                        langs[i].binary);
    }
}

/* Print the usage/help text. */
void print_usage(const char *prog)
{
    std::printf("Usage: %s [options]\n", prog);
    std::printf("Report package managers and language runtimes installed on "
                "this system.\n\n");
    std::printf("Options:\n");
    std::printf("  -j, --json    Output as JSON\n");
    std::printf("  -h, --help    Show this help and exit\n");
}
