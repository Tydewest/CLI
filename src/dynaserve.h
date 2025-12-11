#ifndef DYNASERVE_H
#define DYNASERVE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>

// Color codes
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"

// Update cache TTL (in seconds)
#define UPDATE_CACHE_TTL 86400  // 24 hours

// Cache file
#define UPDATE_CACHE_FILE ".dynaserve_update_cache"

// Function declarations
void print_help();
void greet_user(const char *name);
void run_server(const char *port);
void show_version();
void update_cli();
const char* get_installed_version();
const char* get_platform_string();
void check_update();

#endif