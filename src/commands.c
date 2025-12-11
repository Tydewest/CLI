#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "dynaserve.h"

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

// ---------------- Helpers ----------------
static char *get_cache_path() {
    static char path[PATH_MAX];
    const char *home = getenv("HOME");
    if (!home) home = ".";
    snprintf(path, sizeof(path), "%s/%s", home, UPDATE_CACHE_FILE);
    return path;
}

static int is_cache_valid() {
    const char *cache_file = get_cache_path();
    struct stat st;
    if (stat(cache_file, &st) != 0) return 0;
    time_t now = time(NULL);
    return (now - st.st_mtime) < UPDATE_CACHE_TTL;
}

static void write_cache(const char *version) {
    const char *cache_file = get_cache_path();
    FILE *fp = fopen(cache_file, "w");
    if (!fp) return;
    fprintf(fp, "%s\n", version);
    fclose(fp);
}

static int read_cache(char *version, size_t size) {
    const char *cache_file = get_cache_path();
    FILE *fp = fopen(cache_file, "r");
    if (!fp) return 0;
    if (!fgets(version, size, fp)) {
        fclose(fp);
        return 0;
    }
    version[strcspn(version, "\n")] = 0;
    fclose(fp);
    return 1;
}

// ---------------- Installed version ----------------
const char* get_installed_version() {
    static char version[64] = {0};
    const char *home = getenv("HOME");
    if (!home) return "unknown";

    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/.dynaserve_version", home);

    FILE *fp = fopen(path, "r");
    if (!fp) return "unknown";

    fgets(version, sizeof(version), fp);
    fclose(fp);

    version[strcspn(version, "\n")] = 0;
    return version;
}

// ---------------- Update check ----------------
void check_update() {
    char latest_version[64] = {0};

    if (!is_cache_valid() || !read_cache(latest_version, sizeof(latest_version))) {
        FILE *fp = popen(
            "curl -s https://api.github.com/repos/Tydewest/CLI/releases/latest | "
            "grep tag_name | head -n1 | cut -d'\"' -f4", "r");
        if (fp) {
            if (fgets(latest_version, sizeof(latest_version), fp) != NULL) {
                latest_version[strcspn(latest_version, "\n")] = 0;
                write_cache(latest_version);
            }
            pclose(fp);
        }
    }

    if (strlen(latest_version) > 0 && strcmp(latest_version, get_installed_version()) != 0) {
        if (isatty(fileno(stdout))) {
            printf("\n");
            printf(COLOR_YELLOW "  ⚠ Dynaserve CLI v%s available! Run %ssudo dynaserve update%s to upgrade\n" COLOR_RESET,
                   latest_version, COLOR_GREEN, COLOR_YELLOW);
            printf("\n");
        }
    }
}

// ---------------- Commands ----------------
void print_help() {
    printf(COLOR_BLUE "Dynaserve CLI Commands:\n" COLOR_RESET);
    printf(COLOR_GREEN "  help" COLOR_RESET "          Show this help message\n");
    printf(COLOR_GREEN "  greet [name]" COLOR_RESET "  Greet the user\n");
    printf(COLOR_GREEN "  serve [port]" COLOR_RESET "  Start server on specified port (default 8080)\n");
    printf(COLOR_GREEN "  version" COLOR_RESET "       Show Dynaserve CLI version\n");
    printf(COLOR_GREEN "  update" COLOR_RESET "        Update CLI to latest version\n");
    check_update();
}

void greet_user(const char *name) {
    printf(COLOR_YELLOW "Hello, %s!\n" COLOR_RESET, name);
    check_update();
}

void run_server(const char *port) {
    printf(COLOR_YELLOW "Starting Dynaserve server on port %s...\n" COLOR_RESET, port);
    check_update();
}

void show_version() {
    printf(COLOR_GREEN "Dynaserve CLI %s\n" COLOR_RESET, get_installed_version());
    check_update();
}

// ---------------- Platform string ----------------
const char* get_platform_string() {
#if defined(__APPLE__)
    FILE *arch_fp = popen("uname -m", "r");
    static char platform[64] = {0};
    if (arch_fp) {
        char arch[32] = {0};
        if (fgets(arch, sizeof(arch), arch_fp) != NULL) {
            arch[strcspn(arch, "\n")] = 0;
            if (strcmp(arch, "arm64") == 0) snprintf(platform, sizeof(platform), "darwin-arm64");
            else snprintf(platform, sizeof(platform), "darwin-x86_64");
        }
        pclose(arch_fp);
    }
    return platform[0] ? platform : "darwin-arm64";
#elif defined(__linux__)
    return "linux-x86_64";
#else
    return "unknown";
#endif
}

// ---------------- Update CLI ----------------
void update_cli() {
    const char *cache_file = get_cache_path();
    remove(cache_file);

    printf(COLOR_YELLOW "Checking for updates...\n" COLOR_RESET);

    char latest_version[64] = {0};
    FILE *fp = popen(
        "curl -s https://api.github.com/repos/Tydewest/CLI/releases/latest | "
        "grep tag_name | head -n1 | cut -d'\"' -f4", "r");
    if (!fp) {
        printf(COLOR_RED "Failed to check latest version.\n" COLOR_RESET);
        return;
    }
    if (fgets(latest_version, sizeof(latest_version), fp) != NULL) {
        latest_version[strcspn(latest_version, "\n")] = 0;
    }
    pclose(fp);

    const char *installed = get_installed_version();
    if (strcmp(latest_version, installed) == 0) {
        printf(COLOR_GREEN "Dynaserve CLI is already up-to-date (%s).\n" COLOR_RESET, installed);
        return;
    }

    printf(COLOR_YELLOW "New version available: %s\n" COLOR_RESET, latest_version);
}