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
#elif defined(_WIN32)
#include <windows.h>
#endif

// ---------------- Helpers ----------------
static char *get_cache_path() {
    static char path[PATH_MAX];
    const char *home = getenv("HOME");
#if defined(_WIN32)
    if (!home) home = getenv("USERPROFILE");
#endif
    if (!home) home = ".";
    snprintf(path, sizeof(path), "%s/%s", home, UPDATE_CACHE_FILE);
    return path;
}

static int is_cache_valid() {
    const char *cache_file = get_cache_path();
    struct stat st;
    if (stat(cache_file, &st) != 0) return 0; // no cache
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

// ---------------- Dynamic installed version ----------------
const char* get_installed_version() {
    static char version[64] = {0};
    const char *home = getenv("HOME");
#if defined(_WIN32)
    if (!home) home = getenv("USERPROFILE");
#endif
    static char path[PATH_MAX];

    if (!home) return "unknown";

    snprintf(path, sizeof(path), "%s/.dynaserve_version", home);

    FILE *fp = fopen(path, "r");
    if (!fp) return "unknown";

    fgets(version, sizeof(version), fp);
    fclose(fp);

    version[strcspn(version, "\n")] = 0;
    return version;
}

// ---------------- Update check (synchronous) ----------------
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
        if (isatty(fileno(stdout))) { // only print in interactive terminals
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
    printf(COLOR_GREEN "  version, -v" COLOR_RESET "  Show Dynaserve CLI version\n");
    printf(COLOR_GREEN "  update" COLOR_RESET "        Update CLI to latest version\n");
    printf(COLOR_BLUE "Instructions can be found at: https://cli.dynserve.io/manual\n" COLOR_RESET);
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
    const char *installed = get_installed_version();
    printf(COLOR_GREEN "Dynaserve CLI %s\n" COLOR_RESET, installed);
    check_update();
}

// ---------------- Get platform string ----------------
static const char* get_platform_string() {
#if defined(__APPLE__)
    FILE *arch_fp = popen("uname -m", "r");
    static char platform[64] = {0};
    if (arch_fp) {
        char arch[32] = {0};
        if (fgets(arch, sizeof(arch), arch_fp) != NULL) {
            arch[strcspn(arch, "\n")] = 0;
            if (strcmp(arch, "arm64") == 0) {
                snprintf(platform, sizeof(platform), "darwin-arm64");
            } else {
                snprintf(platform, sizeof(platform), "darwin-x86_64");
            }
        }
        pclose(arch_fp);
    }
    return platform[0] ? platform : "darwin-x86_64";
#elif defined(__linux__)
    return "linux-x86_64";
#elif defined(_WIN32)
    return "windows-x64.exe";
#else
    return "unknown";
#endif
}

// ---------------- Update CLI ----------------
void update_cli() {
    const char *cache_file = get_cache_path();
    remove(cache_file); // clear cached version

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

    // Get executable path
    char exe_path[PATH_MAX] = {0};
#if defined(__APPLE__)
    uint32_t size = sizeof(exe_path);
    if (_NSGetExecutablePath(exe_path, &size) != 0) {
        printf(COLOR_RED "Failed to get executable path.\n" COLOR_RESET);
        return;
    }
#elif defined(__linux__)
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path)-1);
    if (len != -1) exe_path[len] = '\0';
    else {
        printf(COLOR_RED "Failed to get executable path.\n" COLOR_RESET);
        return;
    }
#elif defined(_WIN32)
    if (!GetModuleFileNameA(NULL, exe_path, sizeof(exe_path))) {
        printf(COLOR_RED "Failed to get executable path.\n" COLOR_RESET);
        return;
    }
#else
    printf(COLOR_RED "Unsupported platform.\n" COLOR_RESET);
    return;
#endif

    const char *platform = get_platform_string();
    printf(COLOR_YELLOW "Detected platform: %s\n" COLOR_RESET, platform);

    // Construct download command
    char download_cmd[1024];
#if defined(_WIN32)
    snprintf(download_cmd, sizeof(download_cmd),
        "powershell -Command \"Invoke-WebRequest -Uri https://github.com/Tydewest/CLI/releases/download/%s/dynaserve-%s -OutFile '%s_new'\"",
        latest_version, platform, exe_path);
#else
    snprintf(download_cmd, sizeof(download_cmd),
        "curl -L -o \"%s_new\" https://github.com/Tydewest/CLI/releases/download/%s/dynaserve-%s",
        exe_path, latest_version, platform);
#endif

    printf(COLOR_YELLOW "Downloading new version...\n" COLOR_RESET);
    if (system(download_cmd) != 0) {
        printf(COLOR_RED "Failed to download latest binary.\n" COLOR_RESET);
        return;
    }

#if !defined(_WIN32)
    char chmod_cmd[512];
    snprintf(chmod_cmd, sizeof(chmod_cmd), "chmod +x \"%s_new\"", exe_path);
    system(chmod_cmd);

    char backup_cmd[512];
    snprintf(backup_cmd, sizeof(backup_cmd), "mv \"%s\" \"%s_backup\"", exe_path, exe_path);
    system(backup_cmd);

    char replace_cmd[512];
    snprintf(replace_cmd, sizeof(replace_cmd), "mv \"%s_new\" \"%s\"", exe_path, exe_path);
    if (system(replace_cmd) != 0) {
        printf(COLOR_RED "Failed to replace binary. Restoring backup...\n" COLOR_RESET);
        snprintf(replace_cmd, sizeof(replace_cmd), "mv \"%s_backup\" \"%s\"", exe_path, exe_path);
        system(replace_cmd);
        return;
    }
#else
    // Windows replacement
    char old_backup[PATH_MAX];
    snprintf(old_backup, sizeof(old_backup), "%s_backup.exe", exe_path);
    rename(exe_path, old_backup);
    rename(strcat(exe_path, "_new"), exe_path);
#endif

    // Write installed version file
    const char *home = getenv("HOME");
#if defined(_WIN32)
    if (!home) home = getenv("USERPROFILE");
#endif
    char version_path[PATH_MAX];
    snprintf(version_path, sizeof(version_path), "%s/.dynaserve_version", home);

    FILE *vfp = fopen(version_path, "w");
    if (vfp) {
        fprintf(vfp, "%s\n", latest_version);
        fclose(vfp);
    }

    printf(COLOR_GREEN "Dynaserve CLI updated to version %s!\n" COLOR_RESET, latest_version);
}