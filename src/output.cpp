/*
 * output.cpp
 *
 * Probing installed tools and printing the results, either as JSON
 * (script-friendly) or as a human-readable table.
 */

/* Needed for gmtime_r() when not already in gnu++ mode. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "output.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/utsname.h>

#include "helpers.h"

/* ------------------------------------------------------------------ */
/* Probing                                                            */
/* ------------------------------------------------------------------ */

/* Probe one tool: is it installed? what does it report as its version? */
struct result *probe(const char *binary, const char *version_cmd,
                     const char *filter)
{
    struct result *r =
        static_cast<struct result *>(calloc(1, sizeof *r));
    if (r == nullptr)
        return nullptr;
    r->found = in_path(binary);
    if (r->found &&
        !first_line(version_cmd, filter, r->version, sizeof r->version))
        r->version[0] = '\0';
    return r;
}

/* ------------------------------------------------------------------ */
/* JSON output                                                        */
/* ------------------------------------------------------------------ */

static void json_entry(const char *name, const char *binary,
                       const struct result *r, int last)
{
    char name_e[512], bin_e[512], ver_e[2048];
    json_escape(name, name_e, sizeof name_e);
    json_escape(binary, bin_e, sizeof bin_e);
    json_escape(r->found ? r->version : "", ver_e, sizeof ver_e);

    printf("    { \"name\": \"%s\", \"binary\": \"%s\", "
           "\"installed\": %s, \"version\": \"%s\" }%s\n",
           name_e, bin_e, r->found ? "true" : "false", ver_e,
           last ? "" : ",");
}

void print_json(const struct pkg_mgr *mgrs, std::size_t mgr_count,
                const struct lang *langs, std::size_t lang_count)
{
    struct utsname u;
    char sys_e[256], rel_e[256], mach_e[256], ts[64];
    int have_u = (uname(&u) == 0);

    if (have_u) {
        json_escape(u.sysname, sys_e, sizeof sys_e);
        json_escape(u.release, rel_e, sizeof rel_e);
        json_escape(u.machine, mach_e, sizeof mach_e);
    } else {
        snprintf(sys_e, sizeof sys_e, "unknown");
        rel_e[0] = '\0';
        mach_e[0] = '\0';
    }

    time_t now = time(nullptr);
    struct tm tmv;
    gmtime_r(&now, &tmv);
    strftime(ts, sizeof ts, "%Y-%m-%dT%H:%M:%SZ", &tmv);

    /* Probe everything first so the summary counts are correct. */
    struct result **mgr_res =
        static_cast<struct result **>(
            calloc(mgr_count ? mgr_count : 1, sizeof *mgr_res));
    struct result **lang_res =
        static_cast<struct result **>(
            calloc(lang_count ? lang_count : 1, sizeof *lang_res));
    if (mgr_res == nullptr || lang_res == nullptr) {
        fprintf(stderr, "error: out of memory\n");
        free(mgr_res);
        free(lang_res);
        return;
    }

    std::size_t mgr_found = 0, lang_found = 0;
    for (std::size_t i = 0; i < mgr_count; i++) {
        mgr_res[i] = probe(mgrs[i].binary, mgrs[i].version_cmd, nullptr);
        if (mgr_res[i] != nullptr && mgr_res[i]->found)
            mgr_found++;
    }
    for (std::size_t i = 0; i < lang_count; i++) {
        lang_res[i] = probe(langs[i].binary, langs[i].version_cmd,
                            langs[i].filter);
        if (lang_res[i] != nullptr && lang_res[i]->found)
            lang_found++;
    }

    /* Fallback entry used if a per-tool probe failed (out of memory). */
    static const struct result empty_result{};

    printf("{\n");
    printf("  \"system\": { \"kernel\": \"%s\", \"release\": \"%s\", "
           "\"architecture\": \"%s\" },\n", sys_e, rel_e, mach_e);
    printf("  \"generated_at\": \"%s\",\n", ts);
    printf("  \"summary\": { \"package_managers_installed\": %zu, "
           "\"languages_installed\": %zu },\n", mgr_found, lang_found);

    printf("  \"package_managers\": [\n");
    for (std::size_t i = 0; i < mgr_count; i++)
        json_entry(mgrs[i].name, mgrs[i].binary,
                   mgr_res[i] != nullptr ? mgr_res[i] : &empty_result,
                   i + 1 == mgr_count);
    printf("  ],\n");

    printf("  \"languages\": [\n");
    for (std::size_t i = 0; i < lang_count; i++)
        json_entry(langs[i].name, langs[i].binary,
                   lang_res[i] != nullptr ? lang_res[i] : &empty_result,
                   i + 1 == lang_count);
    printf("  ]\n");
    printf("}\n");

    for (std::size_t i = 0; i < mgr_count; i++)
        free(mgr_res[i]);
    for (std::size_t i = 0; i < lang_count; i++)
        free(lang_res[i]);
    free(mgr_res);
    free(lang_res);
}

/* ------------------------------------------------------------------ */
/* Table output (default)                                             */
/* ------------------------------------------------------------------ */

void print_usage(const char *prog)
{
    printf("Usage: %s [OPTION]\n\n", prog);
    printf("Shows package managers and installed programming languages\n");
    printf("on the current Unix system.\n\n");
    printf("Options:\n");
    printf("  -j, --json    Print results as JSON (for scripting)\n");
    printf("  -h, --help    Show this help and exit\n");
}

void print_table(const struct pkg_mgr *mgrs, std::size_t mgr_count,
                 const struct lang *langs, std::size_t lang_count)
{
    puts("======================================================");
    puts(" Unix package managers & installed languages");
    puts("======================================================");
    print_os_info();
    puts("");

    puts("--- Package managers ---");
    printf("%-26s %-12s %s\n", "NAME", "BINARY", "VERSION");
    printf("%-26s %-12s %s\n", "----------------------------",
           "------------", "-------");
    for (std::size_t i = 0; i < mgr_count; i++) {
        char version[256] = "";
        int found = in_path(mgrs[i].binary);
        if (found)
            first_line(mgrs[i].version_cmd, nullptr, version,
                       sizeof version);
        printf("%-26s %-12s %s\n", mgrs[i].name, mgrs[i].binary,
               found ? (version[0] ? version : "installed")
                     : "not installed");
    }

    puts("");
    puts("--- Programming languages / runtimes ---");
    printf("%-26s %-12s %s\n", "NAME", "BINARY", "VERSION");
    printf("%-26s %-12s %s\n", "----------------------------",
           "------------", "-------");
    for (std::size_t i = 0; i < lang_count; i++) {
        char version[256] = "";
        int found = in_path(langs[i].binary);
        if (found)
            first_line(langs[i].version_cmd, langs[i].filter,
                       version, sizeof version);
        printf("%-26s %-12s %s\n", langs[i].name, langs[i].binary,
               found ? (version[0] ? version : "installed")
                     : "not installed");
    }

    puts("");
}
