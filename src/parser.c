#include "shell.h"

/* Split a command line into tokens.
   - words are separated by spaces/tabs
   - '...' and "..." group text (spaces inside quotes are kept, quotes removed)
   - the characters in OPERATOR_CHARS (< > |) are always tokens on their own
   Returns a NULL-terminated array of malloc'd strings, or NULL if empty/error. */
char** tokenize(char* cmdline) {
    if (cmdline == NULL) return NULL;

    char** arglist = calloc(MAXARGS + 1, sizeof(char*));   /* all slots start NULL */
    char* buf = malloc(strlen(cmdline) + 1);               /* scratch for one token */
    int argc = 0, error = 0;
    char* p = cmdline;

    while (*p != '\0') {
        while (*p == ' ' || *p == '\t') p++;               /* skip whitespace */
        if (*p == '\0') break;

        if (argc >= MAXARGS) {
            fprintf(stderr, "shell: too many arguments (limit is %d)\n", MAXARGS);
            error = 1;
            break;
        }

        int len = 0;
        if (strchr(OPERATOR_CHARS, *p) != NULL) {          /* operator: one char token */
            buf[len++] = *p++;
        } else {                                           /* ordinary word */
            char quote = 0;
            while (*p != '\0') {
                if (quote) {
                    if (*p == quote) quote = 0;            /* closing quote */
                    else buf[len++] = *p;
                } else if (*p == '"' || *p == '\'') {
                    quote = *p;                            /* opening quote */
                } else if (*p == ' ' || *p == '\t' || strchr(OPERATOR_CHARS, *p) != NULL) {
                    break;                                 /* end of word */
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
        arglist[argc++] = strdup(buf);
    }

    free(buf);
    if (error || argc == 0) {
        free_tokens(arglist);
        return NULL;
    }
    return arglist;
}

/* Frees every slot (free(NULL) is harmless), so it is safe even after we
   replaced some tokens with NULL while splitting the line. */
void free_tokens(char** arglist) {
    if (arglist == NULL) return;
    for (int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
    free(arglist);
}

/* 1 if the tokens contain a redirection or a pipe */
int has_operator(char** arglist) {
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "<") == 0 || strcmp(arglist[i], ">") == 0 ||
            strcmp(arglist[i], "|") == 0)
            return 1;
    }
    return 0;
}
