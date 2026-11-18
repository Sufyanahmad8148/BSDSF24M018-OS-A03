#include "shell.h"

int last_status = 0;

int handle_builtin(char** arglist) {
    /* NAME=value  ->  store a shell variable (never forked) */
    if (is_assignment(arglist[0])) {
        if (arglist[1] != NULL) {
            fprintf(stderr, "shell: unexpected text after assignment (use quotes: NAME=\"a b\")\n");
            last_status = 1;
            return 1;
        }
        char* eq = strchr(arglist[0], '=');
        *eq = '\0';                                /* split into NAME and value */
        set_var(arglist[0], eq + 1);
        last_status = 0;
        return 1;
    }
    if (strcmp(arglist[0], "exit") == 0) {
        printf("Goodbye!\n");
        exit(0);
    }
    if (strcmp(arglist[0], "cd") == 0) {
        const char* dir = arglist[1] ? arglist[1] : getenv("HOME");
        if (dir == NULL || chdir(dir) != 0) {
            perror("cd");
            last_status = 1;
        } else {
            last_status = 0;
        }
        return 1;
    }
    if (strcmp(arglist[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("  cd <dir>  - change directory\n");
        printf("  exit      - exit the shell\n");
        printf("  help      - show this message\n");
        printf("  jobs      - list background jobs\n");
        printf("  history   - show previous commands (use !n to re-run command n)\n");
        last_status = 0;
        return 1;
    }
    if (strcmp(arglist[0], "jobs") == 0) {
        print_jobs();
        last_status = 0;
        return 1;
    }
    if (strcmp(arglist[0], "history") == 0) {
        print_history();
        last_status = 0;
        return 1;
    }
    if (strcmp(arglist[0], "set") == 0) {
        print_vars();
        last_status = 0;
        return 1;
    }
    return 0;
}
