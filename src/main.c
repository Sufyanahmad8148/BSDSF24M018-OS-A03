#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

int main() {
    char* cmdline;
    char** arglist;

    // Tell readline to use our completer for the first word (commands)
    rl_attempted_completion_function = shell_completion;

    while ((cmdline = readline(PROMPT)) != NULL) {   // readline prints the prompt
        if (cmdline[0] == '\0') {                    // empty line
            free(cmdline);
            continue;
        }

        // !n  -> replace the line with the n-th command from our history
        if (cmdline[0] == '!') {
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

        add_to_history(cmdline);   // our own list (used by 'history' and '!n')
        add_history(cmdline);      // readline's list (used by Up/Down arrows)

        if ((arglist = tokenize(cmdline)) != NULL) {
            if (handle_builtin(arglist) == 0) {
                execute(arglist);
            }
            for (int i = 0; arglist[i] != NULL; i++) {
                free(arglist[i]);
            }
            free(arglist);
        }
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
