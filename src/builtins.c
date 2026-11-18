#include "shell.h"

int handle_builtin(char** arglist)
{
    if (arglist == NULL || arglist[0] == NULL)
        return 0;

    if (strcmp(arglist[0], "exit") == 0) {
        printf("Goodbye!\n");
        exit(0);
    }

    if (strcmp(arglist[0], "cd") == 0) {

        const char* dir =
            arglist[1] ? arglist[1] : getenv("HOME");

        if (dir == NULL || chdir(dir) != 0)
            perror("cd");

        return 1;
    }

    if (strcmp(arglist[0], "help") == 0) {

        printf("Built-in commands:\n");
        printf("  cd <dir>  - change directory\n");
        printf("  exit      - exit the shell\n");
        printf("  help      - show this message\n");
        printf("  jobs      - list background jobs\n");
        printf("  history   - show previous commands\n");

        return 1;
    }

    if (strcmp(arglist[0], "jobs") == 0) {
        print_jobs();
        return 1;
    }

    if (strcmp(arglist[0], "history") == 0) {
        print_history();
        return 1;
    }

    return 0;
}
