#include <stdio.h>
#include <string.h>
#include "dynaserve.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf(COLOR_RED "No command provided. Use 'help' or '-h' for a list of commands.\n" COLOR_RESET);
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "help") == 0 || strcmp(command, "-h") == 0) print_help();
    else if (strcmp(command, "greet") == 0) {
        const char *name = (argc >= 3) ? argv[2] : "World";
        greet_user(name);
    }
    else if (strcmp(command, "serve") == 0) {
        const char *port = (argc >= 3) ? argv[2] : "8080";
        run_server(port);
    }
    else if (strcmp(command, "version") == 0 || strcmp(command, "-v") == 0) show_version();
    else if (strcmp(command, "update") == 0) update_cli();
    else {
        printf(COLOR_RED "Unknown command: %s\n" COLOR_RESET, command);
        printf("Use 'help' to see available commands.\n");
        return 1;
    }

    return 0;
}