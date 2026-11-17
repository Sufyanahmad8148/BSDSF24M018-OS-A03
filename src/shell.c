#include "shell.h"

char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);
    int c, pos = 0;

    while ((c = getc(fp)) != EOF) {
        if (c == '\n') break;
        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL; // Handle Ctrl+D
    }
    
    cmdline[pos] = '\0';
    return cmdline;
}

char** tokenize(char* cmdline) {
    // Edge case: empty command line
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n') {
        return NULL;
    }

    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char* cp = cmdline;
    char* start;
    int len;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++; // Skip leading whitespace
        
        if (*cp == '\0') break; // Line was only whitespace

        start = cp;
        len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) {
            len++;
        }
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) { // No arguments were parsed
        for(int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}
/* ---------- Command history (ring buffer) ---------- */
static char* history[HISTORY_SIZE];
static int hist_total = 0;   /* total commands ever stored */

static int first_valid(void) {
    return hist_total > HISTORY_SIZE ? hist_total - HISTORY_SIZE + 1 : 1;
}

void add_to_history(const char* cmd) {
    int idx = hist_total % HISTORY_SIZE;
    free(history[idx]);              /* frees the oldest entry when full */
    history[idx] = strdup(cmd);
    hist_total++;
}

const char* get_history(int n) {
    if (n < first_valid() || n > hist_total) return NULL;
    return history[(n - 1) % HISTORY_SIZE];
}

void print_history(void) {
    for (int i = first_valid(); i <= hist_total; i++) {
        printf("%d  %s\n", i, history[(i - 1) % HISTORY_SIZE]);
    }
}
