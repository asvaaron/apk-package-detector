/*
 * pkg_lang_info.c
 *
 * Unix-based tool (Linux, macOS, *BSD) that shows:
 *   1. Available package managers (Homebrew, apt, dnf, pacman, ...)
 *   2. Installed programming languages / runtimes with their versions
 *
 * Options:
 *   --json, -j    Print results as JSON (script-friendly)
 *   --help, -h    Show usage
 *
 * Build:  cc -std=c99 -O2 -o pkg_lang_info pkg_lang_info.c
 *        or: cmake -B build && cmake --build build
 * Run:    ./pkg_lang_info [--json]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/utsname.h>

/* ------------------------------------------------------------------ */
/* Helpers                                                            */
/* ------------------------------------------------------------------ */

/* Return 1 if cmd exists as an executable somewhere on $PATH. */
static int in_path(const char *cmd)
{
    if (!cmd || *cmd == '\0')
        return 0;

    const char *path_env = getenv("PATH");
    if (!path_env || *path_env == '\0')
        return access(cmd, X_OK) == 0; /* no PATH set: try relative */

    char buf[4096];
    if (snprintf(buf, sizeof buf, "%s", path_env) >= (int)sizeof buf)
        return 0;

    char *save = NULL;
    for (char *dir = strtok_r(buf, ":", &save); dir;
         dir = strtok_r(NULL, ":", &save)) {
        char full[4096 + 256];
        int n = snprintf(full, sizeof full, "%s/%s", dir, cmd);
        if (n < 0 || (size_t)n >= sizeof full)
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
static int first_line(const char *command, const char *filter,
                      char *out, size_t out_size)
{
    out[0] = '\0';
    FILE *fp = popen(command, "r");
    if (!fp)
        return 0;

    char line[512];
    int found = 0;
    while (!found && fgets(line, sizeof line, fp)) {
        /* strip trailing whitespace */
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r' ||
                           line[len - 1] == ' '  || line[len - 1] == '\t'))
            line[--len] = '\0';

        if (len == 0)
            continue;
        if (filter && strstr(line, filter) == NULL)
            continue;

        snprintf(out, out_size, "%s", line);
        found = 1;
    }
    pclose(fp);
    return found;
}

/* Escape s for safe use inside a JSON string literal. */
static void json_escape(const char *s, char *out, size_t out_size)
{
    size_t o = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
        const char *rep = NULL;
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

        if (rep == NULL) {              /* plain byte */
            if (o + 1 > out_size)
                break;
            out[o++] = (char)*p;
            continue;
        }

        size_t len = strlen(rep);
        if (o + len > out_size)
            break;
        memcpy(out + o, rep, len);
        o += len;
    }
    out[o] = '\0';
}

static void print_os_info(void)
{
    struct utsname u;
    if (uname(&u) == 0)
        printf("System: %s %s (%s)\n", u.sysname, u.release, u.machine);
    else
        printf("System: unknown (uname failed)\n");
}

/* ------------------------------------------------------------------ */
/* Data tables                                                        */
/* ------------------------------------------------------------------ */

struct pkg_mgr {
    const char *name;
    const char *binary;
    const char *version_cmd;
};

static const struct pkg_mgr *get_pkg_mgrs(size_t *count)
{
#if defined(__APPLE__)
    static const struct pkg_mgr mgrs[] = {
        { "Homebrew",      "brew", "brew --version" },
        { "MacPorts",      "port", "port version" },
        { "Mac App Store", "mas",  "mas --version" },
    };
#else
    static const struct pkg_mgr mgrs[] = {
        { "APT (Debian/Ubuntu)", "apt-get", "apt-get --version" },
        { "DNF (Fedora)",        "dnf",     "dnf --version" },
        { "YUM (RHEL/CentOS)",   "yum",     "yum --version" },
        { "Zypper (openSUSE)",   "zypper",  "zypper --version" },
        { "Pacman (Arch)",       "pacman",  "pacman --version" },
        { "APK (Alpine)",        "apk",     "apk --version" },
        { "Nix",                 "nix",     "nix --version" },
    };
#endif
    *count = sizeof mgrs / sizeof mgrs[0];
    return mgrs;
}

struct lang {
    const char *name;
    const char *binary;
    const char *version_cmd;
    const char *filter; /* optional substring for the right version line */
};

static const struct lang *get_langs(size_t *count)
{
    static const struct lang langs[] = {
        { "C (GCC)",        "gcc",     "gcc --version",       NULL },
        { "C++ (G++)",      "g++",     "g++ --version",       NULL },
        { "C/C++ (Clang)",  "clang",   "clang --version",     NULL },
        { "Fortran (GFortran)", "gfortran", "gfortran --version", NULL },
        { "Java (JDK)",     "java",    "java -version 2>&1",  NULL },
        { "Kotlin",         "kotlinc", "kotlinc -version 2>&1", NULL },
        { "Scala",          "scala",   "scala -version 2>&1", NULL },
        { "Python 3",       "python3", "python3 --version",   NULL },
        { "Python 2",       "python",  "python --version",    NULL },
        { "Node.js",        "node",    "node --version",      NULL },
        { "Deno",           "deno",    "deno --version",      NULL },
        { "Bun",            "bun",     "bun --version",       NULL },
        { "Ruby",           "ruby",    "ruby --version",      NULL },
        { "Perl",           "perl",    "perl -e 'print $^V'", NULL },
        { "PHP",            "php",     "php --version",       NULL },
        { "Go",             "go",      "go version",          NULL },
        { "Rust",           "rustc",   "rustc --version",     NULL },
        { "Swift",          "swift",   "swift --version",     NULL },
        { "Lua",            "lua",     "lua -v 2>&1",         NULL },
        { "LuaJIT",         "luajit",  "luajit -v 2>&1",      NULL },
        { "Haskell (GHC)",  "ghc",     "ghc --version",       NULL },
        { "Elixir",         "elixir",  "elixir --version",    "Elixir" },
        { "OCaml",          "ocaml",   "ocaml --version",     NULL },
        { "Zig",            "zig",     "zig version",         NULL },
        { "Crystal",        "crystal", "crystal --version",   NULL },
        { "R",              "R",       "R --version",         "R version" },
        { "Julia",          "julia",   "julia --version",     NULL },
        { "Bash",           "bash",    "bash --version",      NULL },
        { "Zsh",            "zsh",     "zsh --version",       NULL },
    };
    *count = sizeof langs / sizeof langs[0];
    return langs;
}

/* ------------------------------------------------------------------ */
/* JSON output                                                        */
/* ------------------------------------------------------------------ */

struct result {
    int found;
    char version[256];
};

/* Probe one tool: is it installed? what does it report as its version? */
static struct result *probe(const char *binary,
                            const char *version_cmd,
                            const char *filter)
{
    struct result *r = calloc(1, sizeof *r);
    if (!r)
        return NULL;
    r->found = in_path(binary);
    if (r->found &&
        !first_line(version_cmd, filter, r->version, sizeof r->version))
        r->version[0] = '\0';
    return r;
}

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

static void print_json(const struct pkg_mgr *mgrs, size_t mgr_count,
                       const struct lang *langs, size_t lang_count)
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

    time_t now = time(NULL);
    struct tm tmv;
    gmtime_r(&now, &tmv);
    strftime(ts, sizeof ts, "%Y-%m-%dT%H:%M:%SZ", &tmv);

    /* Probe everything first so the summary counts are correct. */
    struct result **mgr_res = calloc(mgr_count ? mgr_count : 1,
                                     sizeof *mgr_res);
    struct result **lang_res = calloc(lang_count ? lang_count : 1,
                                      sizeof *lang_res);
    if (!mgr_res || !lang_res) {
        fprintf(stderr, "error: out of memory\n");
        free(mgr_res);
        free(lang_res);
        return;
    }

    size_t mgr_found = 0, lang_found = 0;
    for (size_t i = 0; i < mgr_count; i++) {
        mgr_res[i] = probe(mgrs[i].binary, mgrs[i].version_cmd, NULL);
        if (mgr_res[i] && mgr_res[i]->found)
            mgr_found++;
    }
    for (size_t i = 0; i < lang_count; i++) {
        lang_res[i] = probe(langs[i].binary, langs[i].version_cmd,
                            langs[i].filter);
        if (lang_res[i] && lang_res[i]->found)
            lang_found++;
    }

    printf("{\n");
    printf("  \"system\": { \"kernel\": \"%s\", \"release\": \"%s\", "
           "\"architecture\": \"%s\" },\n", sys_e, rel_e, mach_e);
    printf("  \"generated_at\": \"%s\",\n", ts);
    printf("  \"summary\": { \"package_managers_installed\": %zu, "
           "\"languages_installed\": %zu },\n", mgr_found, lang_found);

    printf("  \"package_managers\": [\n");
    for (size_t i = 0; i < mgr_count; i++)
        json_entry(mgrs[i].name, mgrs[i].binary,
                   mgr_res[i] ? mgr_res[i] : &(struct result){0, ""},
                   i + 1 == mgr_count);
    printf("  ],\n");

    printf("  \"languages\": [\n");
    for (size_t i = 0; i < lang_count; i++)
        json_entry(langs[i].name, langs[i].binary,
                   lang_res[i] ? lang_res[i] : &(struct result){0, ""},
                   i + 1 == lang_count);
    printf("  ]\n");
    printf("}\n");

    for (size_t i = 0; i < mgr_count; i++)
        free(mgr_res[i]);
    for (size_t i = 0; i < lang_count; i++)
        free(lang_res[i]);
    free(mgr_res);
    free(lang_res);
}

/* ------------------------------------------------------------------ */
/* Table output (default)                                             */
/* ------------------------------------------------------------------ */

static void print_usage(const char *prog)
{
    printf("Usage: %s [OPTION]\n\n", prog);
    printf("Shows package managers and installed programming languages\n");
    printf("on the current Unix system.\n\n");
    printf("Options:\n");
    printf("  -j, --json    Print results as JSON (for scripting)\n");
    printf("  -h, --help    Show this help and exit\n");
}

/* ------------------------------------------------------------------ */
/* Main                                                               */
/* ------------------------------------------------------------------ */

int main(int argc, char **argv)
{
    int want_json = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--json") == 0 || strcmp(argv[i], "-j") == 0) {
            want_json = 1;
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

    size_t mgr_count, lang_count;
    const struct pkg_mgr *mgrs = get_pkg_mgrs(&mgr_count);
    const struct lang *langs = get_langs(&lang_count);

    /* JSON mode */
    if (want_json) {
        print_json(mgrs, mgr_count, langs, lang_count);
        return 0;
    }

    /* Table mode */
    puts("======================================================");
    puts(" Unix package managers & installed languages");
    puts("======================================================");
    print_os_info();
    puts("");

    puts("--- Package managers ---");
    printf("%-26s %-12s %s\n", "NAME", "BINARY", "VERSION");
    printf("%-26s %-12s %s\n", "----------------------------",
           "------------", "-------");
    for (size_t i = 0; i < mgr_count; i++) {
        char version[256] = "";
        int found = in_path(mgrs[i].binary);
        if (found)
            first_line(mgrs[i].version_cmd, NULL, version, sizeof version);
        printf("%-26s %-12s %s\n", mgrs[i].name, mgrs[i].binary,
               found ? (version[0] ? version : "installed")
                     : "not installed");
    }

    puts("");
    puts("--- Programming languages / runtimes ---");
    printf("%-26s %-12s %s\n", "NAME", "BINARY", "VERSION");
    printf("%-26s %-12s %s\n", "----------------------------",
           "------------", "-------");
    for (size_t i = 0; i < lang_count; i++) {
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
    return 0;
}
