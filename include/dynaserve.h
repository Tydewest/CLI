#ifndef DYNASERVE_H
#define DYNASERVE_H

#define UPDATE_CACHE_FILE ".dynaserve_update_cache"
#define UPDATE_CACHE_TTL 86400  // 24 hours in seconds

// Color codes
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"

// Function declarations
void print_help();
void greet_user(const char *name);
void run_server(const char *port);
void show_version();
void update_cli(); 
void check_update();
const char* get_installed_version();

#endif