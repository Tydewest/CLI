#include "dynaserve.h"

// ---------------- Helpers ----------------
char* get_cache_path() {
    static char path[PATH_MAX];
    const char *home = getenv("HOME");
    if (!home) home = ".";
    snprintf(path, sizeof(path), "%s/%s", home, UPDATE_CACHE_FILE);
    return path;
}

int is_cache_valid() {
    const char *cache_file = get_cache_path();
    struct stat st;
    if (stat(cache_file, &st) != 0) return 0;
    time_t now = time(NULL);
    return (now - st.st_mtime) < UPDATE_CACHE_TTL;
}

void write_cache(const char *version) {
    const char *cache_file = get_cache_path();
    FILE *fp = fopen(cache_file, "w");
    if (!fp) return;
    fprintf(fp, "%s\n", version);
    fclose(fp);
}

int read_cache(char *version, size_t size) {
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
    if (!home) home = ".";
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
            if (fgets(latest_version, sizeof(latest_version), fp)) {
                latest_version[strcspn(latest_version, "\n")] = 0;
                write_cache(latest_version);
            }
            pclose(fp);
        }
    }

    if (strlen(latest_version) > 0 && strcmp(latest_version, get_installed_version()) != 0) {
        if (isatty(fileno(stdout))) {
            printf("\n%s  ⚠ Dynaserve CLI v%s available! Run %supdate%s to upgrade\n\n",
                   COLOR_YELLOW, latest_version, COLOR_GREEN, COLOR_YELLOW);
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
    printf("\nInstructions can be found at: https://cli.dynserve.io/manual\n");

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
    if (fgets(latest_version, sizeof(latest_version), fp)) {
        latest_version[strcspn(latest_version, "\n")] = 0;
    }
    pclose(fp);

    const char *installed = get_installed_version();
    if (strcmp(latest_version, installed) == 0) {
        printf(COLOR_GREEN "Dynaserve CLI is already up-to-date (%s).\n" COLOR_RESET, installed);
        return;
    }

    printf(COLOR_YELLOW "New version available: %s\n" COLOR_RESET, latest_version);

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
#else
    printf(COLOR_RED "Unsupported platform.\n" COLOR_RESET);
    return;
#endif

    const char *platform =
#if defined(__APPLE__)
        "darwin-x86_64";
#else
        "linux-x86_64";
#endif

    char tmp_path[PATH_MAX];
    snprintf(tmp_path, sizeof(tmp_path), "%s_new", exe_path);

    char download_cmd[1024];
    snprintf(download_cmd, sizeof(download_cmd),
        "curl -L -o \"%s\" https://github.com/Tydewest/CLI/releases/download/%s/dynaserve-%s",
        tmp_path, latest_version, platform);

    printf(COLOR_YELLOW "Downloading new version...\n" COLOR_RESET);
    if (system(download_cmd) != 0) {
        printf(COLOR_RED "Failed to download latest binary.\n" COLOR_RESET);
        return;
    }

    char chmod_cmd[512];
    snprintf(chmod_cmd, sizeof(chmod_cmd), "chmod +x \"%s\"", tmp_path);
    system(chmod_cmd);

    char move_cmd[1024];
    if (access(exe_path, W_OK) != 0) {
        snprintf(move_cmd, sizeof(move_cmd), "sudo mv \"%s\" \"%s\"", tmp_path, exe_path);
    } else {
        snprintf(move_cmd, sizeof(move_cmd), "mv \"%s\" \"%s\"", tmp_path, exe_path);
    }

    if (system(move_cmd) != 0) {
        printf(COLOR_RED "Failed to replace binary. You may need to run with sudo manually.\n" COLOR_RESET);
        return;
    }

    const char *home = getenv("HOME");
    if (!home) home = ".";
    char version_path[PATH_MAX];
    snprintf(version_path, sizeof(version_path), "%s/.dynaserve_version", home);

    FILE *vfp = fopen(version_path, "w");
    if (vfp) {
        fprintf(vfp, "%s\n", latest_version);
        fclose(vfp);
    }

    printf(COLOR_GREEN "Dynaserve CLI updated to version %s!\n" COLOR_RESET, latest_version);
}