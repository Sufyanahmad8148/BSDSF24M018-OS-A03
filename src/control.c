#include "shell.h"
#include <readline/readline.h>

#define MAX_BLOCK 64     /* max lines in a then/else block */

/* remove leading and trailing spaces/tabs (edits the string in place) */
static char* trim(char* s) {
    while (*s == ' ' || *s == '\t') s++;
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t')) s[--n] = '\0';
    return s;
}

/* 1 if the line starts with the word "if" */
int is_if_line(const char* line) {
    while (*line == ' ' || *line == '\t') line++;
    return strncmp(line, "if", 2) == 0 &&
           (line[2] == '\0' || line[2] == ' ' || line[2] == '\t');
}

static void free_block(char** block, int n) {
    for (int i = 0; i < n; i++) free(block[i]);
}

/* Reads the rest of an if ... then ... [else ...] fi block (using the "> "
   prompt), then runs the condition and the chosen branch. */
int handle_if(const char* first_line) {
    char* line_copy = strdup(first_line);
    char* cond = trim(trim(line_copy) + 2);        /* text after the word "if" */

    char* then_lines[MAX_BLOCK];
    char* else_lines[MAX_BLOCK];
    int nthen = 0, nelse = 0;
    int state = 0;      /* 0 = waiting for 'then', 1 = in then-block, 2 = in else-block */
    int bad = 0, done = 0;

    /* allow the one-line form:  if cmd; then */
    size_t cl = strlen(cond);
    if (cl >= 6 && strcmp(cond + cl - 6, "; then") == 0) {
        cond[cl - 6] = '\0';
        cond = trim(cond);
        state = 1;
    }
    if (*cond == '\0') {
        fprintf(stderr, "shell: syntax error: 'if' needs a command\n");
        bad = 1;
    }

    while (!done) {                                /* read lines until 'fi' */
        char* line = readline("> ");
        if (line == NULL) {                        /* Ctrl+D / end of input */
            fprintf(stderr, "shell: syntax error: unexpected end of input (missing 'fi')\n");
            free_block(then_lines, nthen);
            free_block(else_lines, nelse);
            free(line_copy);
            return 1;
        }
        char* t = trim(line);

        if (strcmp(t, "fi") == 0) {
            if (state == 0) {
                fprintf(stderr, "shell: syntax error: 'fi' before 'then'\n");
                bad = 1;
            }
            done = 1;
        } else if (state == 0) {
            if (strcmp(t, "then") == 0) {
                state = 1;
            } else if (*t != '\0') {
                fprintf(stderr, "shell: syntax error: expected 'then'\n");
                bad = 1;
            }
        } else if (strcmp(t, "else") == 0) {
            if (state == 2) {
                fprintf(stderr, "shell: syntax error: duplicate 'else'\n");
                bad = 1;
            }
            state = 2;
        } else if (*t != '\0') {
            if (nthen >= MAX_BLOCK || nelse >= MAX_BLOCK) {
                fprintf(stderr, "shell: block too long\n");
                bad = 1;
            } else if (state == 1) {
                then_lines[nthen++] = strdup(t);
            } else {
                else_lines[nelse++] = strdup(t);
            }
        }
        free(line);
    }

    int status = 1;
    if (!bad) {
        status = process_line(cond);               /* run the condition: exit status 0 = true */
        if (status == 0) {
            for (int i = 0; i < nthen; i++) status = process_line(then_lines[i]);
        } else {
            for (int i = 0; i < nelse; i++) status = process_line(else_lines[i]);
        }
    }

    free_block(then_lines, nthen);
    free_block(else_lines, nelse);
    free(line_copy);
    return status;
}
