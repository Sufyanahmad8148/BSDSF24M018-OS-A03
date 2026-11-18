#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

int main() {
    char* cmdline;

    rl_attempted_completion_function = shell_completion;

    while (1) {
        reap_jobs();                                 /* collect finished background jobs first */

        cmdline = readline(PROMPT);
        if (cmdline == NULL) break;                  /* Ctrl+D */

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

        process_line(cmdline);                       /* tokenize, split on ; &, execute */
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
