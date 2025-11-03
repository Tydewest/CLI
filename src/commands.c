#include <stdio.h>
#include "dynaserve.h"

// Print help message
void print_help() {
    printf(COLOR_BLUE "Dynaserve CLI Commands:\n" COLOR_RESET);
    printf(COLOR_GREEN "  help" COLOR_RESET "            Show this help message\n");
    printf(COLOR_GREEN "  greet [name]" COLOR_RESET "  Greet the user\n");
    printf(COLOR_GREEN "  serve [port]" COLOR_RESET "  Start server on specified port (default 8080)\n");
}

// Greet user
void greet_user(const char *name) {
    printf(COLOR_YELLOW "Hello, %s!\n" COLOR_RESET, name);
}

// Run server (placeholder)
void run_server(const char *port) {
    printf(COLOR_YELLOW "Starting Dynaserve server on port %s...\n" COLOR_RESET, port);
    // Here you could start an actual server
}