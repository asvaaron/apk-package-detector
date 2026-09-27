/*
 * tables.cpp
 *
 * The static tables of package managers and programming languages /
 * runtimes that the tool probes for.
 */

#include "tables.h"

/* Return the platform-appropriate package manager table. */
const struct pkg_mgr *get_pkg_mgrs(std::size_t *count)
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

/* Return the programming language / runtime table. */
const struct lang *get_langs(std::size_t *count)
{
    static const struct lang langs[] = {
        { "C (GCC)",        "gcc",     "gcc --version",       nullptr },
        { "C++ (G++)",      "g++",     "g++ --version",       nullptr },
        { "C/C++ (Clang)",  "clang",   "clang --version",     nullptr },
        { "Fortran (GFortran)", "gfortran", "gfortran --version", nullptr },
        { "Java (JDK)",     "java",    "java -version 2>&1",  nullptr },
        { "Kotlin",         "kotlinc", "kotlinc -version 2>&1", nullptr },
        { "Scala",          "scala",   "scala -version 2>&1", nullptr },
        { "Python 3",       "python3", "python3 --version",   nullptr },
        { "Python 2",       "python",  "python --version",    nullptr },
        { "Node.js",        "node",    "node --version",      nullptr },
        { "Deno",           "deno",    "deno --version",      nullptr },
        { "Bun",            "bun",     "bun --version",       nullptr },
        { "Ruby",           "ruby",    "ruby --version",      nullptr },
        { "Perl",           "perl",    "perl -e 'print $^V'", nullptr },
        { "PHP",            "php",     "php --version",       nullptr },
        { "Go",             "go",      "go version",          nullptr },
        { "Rust",           "rustc",   "rustc --version",     nullptr },
        { "Swift",          "swift",   "swift --version",     nullptr },
        { "Lua",            "lua",     "lua -v 2>&1",         nullptr },
        { "LuaJIT",         "luajit",  "luajit -v 2>&1",      nullptr },
        { "Haskell (GHC)",  "ghc",     "ghc --version",       nullptr },
        { "Elixir",         "elixir",  "elixir --version",    "Elixir" },
        { "OCaml",          "ocaml",   "ocaml --version",     nullptr },
        { "Zig",            "zig",     "zig version",         nullptr },
        { "Crystal",        "crystal", "crystal --version",   nullptr },
        { "R",              "R",       "R --version",         "R version" },
        { "Julia",          "julia",   "julia --version",     nullptr },
        { "Bash",           "bash",    "bash --version",      nullptr },
        { "Zsh",            "zsh",     "zsh --version",       nullptr },
    };
    *count = sizeof langs / sizeof langs[0];
    return langs;
}
