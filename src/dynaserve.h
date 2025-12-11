#ifndef DYNASERVE_H
#define DYNASERVE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"

#define UPDATE_CACHE_FILE ".dynaserve_update_cache"
#define UPDATE_CACHE_TTL  3600 // 1 hour

// ---------------- Helpers ----------------
char* get_cache_path();
int is_cache_valid();
void write_cache(const char *version);
int read_cache(char *version, size_t size);
const char* get_installed_version();

// ---------------- Commands ----------------
void print_help();
void greet_user(const char *name);
void run_server(const char *port);
void show_version();
void update_cli();

#endif