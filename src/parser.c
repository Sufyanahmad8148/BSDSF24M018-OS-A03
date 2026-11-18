#include "shell.h"

/* Split command into words and special operators */
char** tokenize(char* cmdline) {

    if (cmdline == NULL)
        return NULL;

    char** arglist = calloc(MAXARGS + 1, sizeof(char*));

    if (arglist == NULL)
        return NULL;

    char* buf = malloc(strlen(cmdline) + 1);

    if (buf == NULL) {
        free(arglist);
        return NULL;
    }

    int argc = 0;
    int error = 0;
    char* p = cmdline;

    while (*p != '\0') {

        while (*p == ' ' || *p == '\t')
            p++;

        if (*p == '\0')
            break;

        if (argc >= MAXARGS) {
            fprintf(stderr, "shell: too many arguments\n");
            error = 1;
            break;
        }

        int len = 0;

        /* Recognize <, > and | separately */
        if (*p == '<' || *p == '>' || *p == '|') {

            buf[len++] = *p;
            p++;

        } else {

            char quote = 0;

            while (*p != '\0') {

                if (quote) {

                    if (*p == quote)
                        quote = 0;
                    else
                        buf[len++] = *p;

                } else if (*p == '"' || *p == '\'') {

                    quote = *p;

                } else if (*p == ' ' || *p == '\t' ||
                           *p == '<' || *p == '>' || *p == '|') {

                    break;

                } else {

                    buf[len++] = *p;
                }

                p++;
            }

            if (quote) {
                fprintf(stderr, "shell: unterminated quote\n");
                error = 1;
                break;
            }
        }

        buf[len] = '\0';
        arglist[argc] = strdup(buf);

        if (arglist[argc] == NULL) {
            error = 1;
            break;
        }

        argc++;
    }

    free(buf);

    if (error || argc == 0) {
        free_tokens(arglist);
        return NULL;
    }

    return arglist;
}

/* Free all allocated tokens */
void free_tokens(char** arglist) {

    if (arglist == NULL)
        return;

    for (int i = 0; i < MAXARGS + 1; i++)
        free(arglist[i]);

    free(arglist);
}

/* Check whether redirection or pipe exists */
int has_operator(char** arglist) {

    for (int i = 0; arglist[i] != NULL; i++) {

        if (strcmp(arglist[i], "<") == 0 ||
            strcmp(arglist[i], ">") == 0 ||
            strcmp(arglist[i], "|") == 0) {

            return 1;
        }
    }

    return 0;
}
