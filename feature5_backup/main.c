#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

int main() {

    char* cmdline;
    char** arglist;

    /* Enable command completion */
    rl_attempted_completion_function = shell_completion;

    while ((cmdline = readline(PROMPT)) != NULL) {

        if (cmdline[0] == '\0') {
            free(cmdline);
            continue;
        }

        /* History re-execution: !n */
        if (cmdline[0] == '!') {

            const char* old = get_history(atoi(cmdline + 1));

            if (old == NULL) {
                fprintf(stderr,
                        "shell: %s: event not found\n", cmdline);

                free(cmdline);
                continue;
            }

            char* replacement = strdup(old);

            free(cmdline);
            cmdline = replacement;

            if (cmdline == NULL) {
                fprintf(stderr, "shell: memory allocation failed\n");
                continue;
            }

            printf("%s\n", cmdline);
        }

        /* Store command in both histories */
        add_to_history(cmdline);
        add_history(cmdline);

        arglist = tokenize(cmdline);

        if (arglist != NULL) {

            /*
             * If redirection or pipe exists,
             * send command to execute().
             *
             * Otherwise, check built-in commands.
             */

            if (has_operator(arglist)) {

                execute(arglist);

            } else if (handle_builtin(arglist) == 0) {

                execute(arglist);
            }

            free_tokens(arglist);
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");

    return 0;
}
