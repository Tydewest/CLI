#ifndef DYNASERVE_H
#define DYNASERVE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"

#define UPDATE_CACHE_FILE ".dynaserve_cache"
#define UPDATE_CACHE_TTL 86400  // 24 hours in seconds

// Helpers
char* get_cache_path();
int is_cache_valid();
void write_cache(const char *version);
int read_cache(char *version, size_t size);
const char* get_installed_version();
const char* get_platform_string();
void check_update();

// Commands
void print_help();
void greet_user(const char *name);
void run_server(const char *port);
void show_version();
void update_cli();
void uninstall_cli();

#endif