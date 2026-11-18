#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

int main() {
    char* cmdline;
    char** arglist;

    rl_attempted_completion_function = shell_completion;

    while ((cmdline = readline(PROMPT)) != NULL) {
        if (cmdline[0] == '\0') {
            free(cmdline);
            continue;
        }

        if (cmdline[0] == '!') {                     /* !n history expansion */
            const char* old = get_history(atoi(cmdline + 1));
            if (old == NULL) {
                fprintf(stderr, "shell: %s: event not found\n", cmdline);
                free(cmdline);
                continue;
            }
            free(cmdline);
            cmdline = strdup(old);
            printf("%s\n", cmdline);
        }

        add_to_history(cmdline);
        add_history(cmdline);

        if ((arglist = tokenize(cmdline)) != NULL) {
            /* a plain built-in must run in the shell process; anything with a
               pipe or redirection (or any external command) goes to execute() */
            if (has_operator(arglist) || !handle_builtin(arglist)) {
                execute(arglist);
            }
            free_tokens(arglist);
        }
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
