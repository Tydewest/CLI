#ifndef DYNASERVE_H
#define DYNASERVE_H

// Terminal colors
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_RESET   "\x1b[0m"

// Command functions
void print_help();
void greet_user(const char *name);
void run_server(const char *port);

#endif