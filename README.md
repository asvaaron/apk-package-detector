# pkg_lang_info

A small Unix utility (C++, no dependencies) that shows which **package managers** and
**programming languages / runtimes** are installed on your system, along with their
versions.

Works on **Linux, macOS, and \*BSD** — only POSIX APIs are used, so no libraries
to install.

## What it shows

- **Package managers** — detects the ones for your platform and their versions:
  - macOS: Homebrew, MacPorts, mas
  - Linux: APT, DNF, YUM, Zypper, Pacman, APK, Nix
- **Languages & runtimes** — C (GCC), C++ (G++), Clang, Fortran, Java, Kotlin,
  Scala, Python 2/3, Node.js, Deno, Bun, Ruby, Perl, PHP, Go, Rust, Swift, Lua,
  LuaJIT, Haskell, Elixir, OCaml, Zig, Crystal, R, Julia, Bash, Zsh

Each entry is reported as **installed** (with version) or **not installed**.

## How to compile

Option A — CMake (recommended):

```bash
cmake -B build
cmake --build build -j
```

Option B — plain compiler:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Iinclude -o pkg_lang_info src/main.cpp src/helpers.cpp src/tables.cpp src/output.cpp
```

Requirements: any POSIX C++ compiler (g++, clang++, ...). No other dependencies.

## How to run

```bash
# Table output (default)
./build/bin/pkg_lang_info        # or ./pkg_lang_info with option B

# JSON output (script-friendly)
./build/bin/pkg_lang_info --json
./build/bin/pkg_lang_info -j | jq '.languages'

# Help
./build/bin/pkg_lang_info --help
```

### Example (table mode)

```
======================================================
 Unix package managers & installed languages
======================================================
System: Linux 6.8.0 (x86_64)

--- Package managers ---
NAME                       BINARY     VERSION
APT (Debian/Ubuntu)        apt-get    apt 2.7.14
DNF (Fedora)               dnf        not installed
...

--- Programming languages / runtimes ---
NAME                       BINARY     VERSION
C (GCC)                    gcc        gcc (Debian 12.2.0-14) 12.2.0
Python 3                   python3    Python 3.11.2
Node.js                    node       v20.11.1
...
```

### Example (JSON mode)

```bash
./build/bin/pkg_lang_info --json
```

```json
{
  "system": { "kernel": "Linux", "release": "6.8.0", "architecture": "x86_64" },
  "generated_at": "2026-01-15T09:32:10Z",
  "summary": { "package_managers_installed": 1, "languages_installed": 6 },
  "package_managers": [
    { "name": "APT (Debian/Ubuntu)", "binary": "apt-get",
      "installed": true, "version": "apt 2.7.14" }
  ],
  "languages": [
    { "name": "Python 3", "binary": "python3",
      "installed": true, "version": "Python 3.11.2" }
  ]
}
```

## Files

| File                | Purpose                                              |
| ------------------- | ---------------------------------------------------- |
| `include/helpers.h` | declarations: PATH lookup, command output, JSON escaping |
| `src/helpers.cpp`   | implementation of the shared helpers                 |
| `include/tables.h`  | declarations of the package-manager/language tables  |
| `src/tables.cpp`    | the static tables of tools to probe                  |
| `include/output.h`  | declarations: probing, JSON/table output             |
| `src/output.cpp`    | probing + JSON and table printing                    |
| `src/main.cpp`      | argument parsing and `main()`                        |
| `CMakeLists.txt`    | CMake build configuration                            |

## Roadmap

- The **next version of this utility will be rewritten in Rust**, with the same
  feature set (package managers, installed languages, `--json` output) and a
  more comfortable CLI.
