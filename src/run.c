#include "shell.h"

/* Run one segment (the tokens between ; or &). */
static int run_segment(char** seg, int background) {
    expand_args(seg);                              /* $NAME -> value, before running anything */
    if (seg[0][0] == '\0') return 0;               /* e.g. "$UNSET" expanded to nothing */

    /* a lone built-in (no pipe/redirect) must run inside the shell process */
    if (!has_operator(seg) && handle_builtin(seg)) return last_status;
    return execute(seg, background);
}

/* Split the token list at ';' and '&' and run each part in order.
   ';' -> wait for the command to finish, '&' -> run it in the background. */
int run_tokens(char** arglist) {
    int status = 0;
    int i = 0;

    while (arglist[i] != NULL) {
        int start = i;
        while (arglist[i] != NULL && strcmp(arglist[i], ";") != 0 &&
               strcmp(arglist[i], "&") != 0)
            i++;

        char sep = 0;                              /* which separator ended this part */
        if (arglist[i] != NULL) {
            sep = arglist[i][0];
            free(arglist[i]);
            arglist[i] = NULL;                     /* terminates this segment */
            i++;
        }

        if (arglist[start] == NULL) {              /* e.g. ";;" or a leading "&" */
            fprintf(stderr, "shell: syntax error near unexpected token '%c'\n", sep);
            return 1;
        }
        status = run_segment(&arglist[start], sep == '&');
    }
    return status;
}

/* tokenize + run one line; returns the exit status */
int process_line(char* line) {
    char** arglist = tokenize(line);
    if (arglist == NULL) return 0;
    int status = run_tokens(arglist);
    free_tokens(arglist);
    return status;
}
