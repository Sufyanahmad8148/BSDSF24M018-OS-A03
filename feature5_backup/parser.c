
#include "shell.h"

/* Split commands using semicolon */
char** split_commands(char* line)
{
    char** commands = calloc(MAX_COMMANDS + 1, sizeof(char*));

    if (commands == NULL)
        return NULL;

    int count = 0;
    char* start = line;
    char quote = 0;

    for (char* p = line; ; p++) {

        if (*p == '"' || *p == '\'') {

            if (quote == 0)
                quote = *p;
            else if (quote == *p)
                quote = 0;
        }

        if ((*p == ';' && quote == 0) || *p == '\0') {

            char saved = *p;
            *p = '\0';

            while (*start == ' ' || *start == '\t')
                start++;

            char* end = start + strlen(start);

            while (end > start &&
                   (end[-1] == ' ' || end[-1] == '\t')) {
                *--end = '\0';
            }

            if (*start != '\0') {

                if (count >= MAX_COMMANDS) {
                    fprintf(stderr,
                            "shell: too many chained commands\n");
                    free_commands(commands);
                    return NULL;
                }

                commands[count] = strdup(start);

                if (commands[count] == NULL) {
                    free_commands(commands);
                    return NULL;
                }

                count++;
            }

            if (saved == '\0')
                break;

            start = p + 1;
        }
    }

    return commands;
}


/* Free command strings */
void free_commands(char** commands)
{
    if (commands == NULL)
        return;

    for (int i = 0; i < MAX_COMMANDS + 1; i++)
        free(commands[i]);

    free(commands);
}


/* Tokenize a command */
char** tokenize(char* cmdline)
{
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

        /* Recognize special operators */
        if (*p == '<' || *p == '>' ||
            *p == '|' || *p == '&') {

            buf[len++] = *p;
            p++;
        }
        else {

            char quote = 0;

            while (*p != '\0') {

                if (quote != 0) {

                    if (*p == quote)
                        quote = 0;
                    else
                        buf[len++] = *p;
                }
                else if (*p == '"' || *p == '\'') {
                    quote = *p;
                }
                else if (*p == ' ' || *p == '\t' ||
                         *p == '<' || *p == '>' ||
                         *p == '|' || *p == '&') {
                    break;
                }
                else {
                    buf[len++] = *p;
                }

                p++;
            }

            if (quote != 0) {
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


/* Free token memory */
void free_tokens(char** arglist)
{
    if (arglist == NULL)
        return;

    for (int i = 0; i < MAXARGS + 1; i++)
        free(arglist[i]);

    free(arglist);
}


/* Check for pipes or redirection */
int has_operator(char** arglist)
{
    for (int i = 0; arglist[i] != NULL; i++) {

        if (strcmp(arglist[i], "<") == 0 ||
            strcmp(arglist[i], ">") == 0 ||
            strcmp(arglist[i], "|") == 0) {
            return 1;
        }
    }

    return 0;
}
