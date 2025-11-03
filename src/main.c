#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "dynaserve.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf(COLOR_RED "No command provided. Use 'help' for a list of commands.\n" COLOR_RESET);
        return 1;
    }

    int verbose = 0;
    const char *config_file = NULL;

    // Parse global flags
    int opt;
    while ((opt = getopt(argc, argv, "vc:")) != -1) {
        switch(opt) {
            case 'v':
                verbose = 1;
                break;
            case 'c':
                config_file = optarg;
                break;
            default:
                printf(COLOR_RED "Unknown flag\n" COLOR_RESET);
                return 1;
        }
    }

    int cmd_index = optind;
    if (cmd_index >= argc) {
        printf(COLOR_RED "No command provided after flags.\n" COLOR_RESET);
        return 1;
    }

    const char *command = argv[cmd_index];

    if (strcmp(command, "help") == 0) {
        print_help();
    } 
    else if (strcmp(command, "greet") == 0) {
        const char *name = (cmd_index + 1 < argc) ? argv[cmd_index + 1] : "World";
        if (verbose) printf(COLOR_BLUE "[Verbose] Running greet command\n" COLOR_RESET);
        greet_user(name);
    } 
    else if (strcmp(command, "serve") == 0) {
        const char *port = (cmd_index + 1 < argc) ? argv[cmd_index + 1] : "8080";
        if (verbose) printf(COLOR_BLUE "[Verbose] Running serve command\n" COLOR_RESET);
        if (config_file) printf(COLOR_GREEN "Using config file: %s\n" COLOR_RESET, config_file);
        run_server(port);
    } 
    else {
        printf(COLOR_RED "Unknown command: %s\n" COLOR_RESET, command);
        printf("Use 'help' to see available commands.\n");
        return 1;
    }

    return 0;
}