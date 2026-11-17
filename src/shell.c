#include "shell.h"
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>

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

/* ---------- Tab completion (GNU Readline) ---------- */
static const char* builtin_names[] = {"exit", "cd", "help", "jobs", "history", NULL};

/* Called repeatedly by readline: returns one matching command per call,
   then NULL when there are no more. state == 0 on the first call. */
static char* command_generator(const char* text, int state) {
    static int bi;
    static char* path_copy;
    static char* path_iter;
    static DIR* dir;
    static char cur_dir[1024];
    static size_t len;
    struct dirent* de;

    if (state == 0) {                       /* first call: reset everything */
        bi = 0;
        len = strlen(text);
        if (dir) { closedir(dir); dir = NULL; }
        free(path_copy);
        const char* p = getenv("PATH");
        path_copy = p ? strdup(p) : NULL;
        path_iter = path_copy;
    }

    while (builtin_names[bi]) {             /* 1) built-in commands */
        const char* name = builtin_names[bi++];
        if (strncmp(name, text, len) == 0) return strdup(name);
    }

    while (1) {                             /* 2) executables found in $PATH */
        if (dir == NULL) {
            if (path_iter == NULL) {
                free(path_copy);
                path_copy = NULL;
                return NULL;
            }
            char* d = strsep(&path_iter, ":");
            if (d == NULL || *d == '\0') continue;
            dir = opendir(d);
            if (dir == NULL) continue;
            strncpy(cur_dir, d, sizeof(cur_dir) - 1);
            cur_dir[sizeof(cur_dir) - 1] = '\0';
        }
        while ((de = readdir(dir)) != NULL) {
            if (strncmp(de->d_name, text, len) != 0) continue;
            char full[2048];
            snprintf(full, sizeof(full), "%s/%s", cur_dir, de->d_name);
            if (access(full, X_OK) == 0) return strdup(de->d_name);
        }
        closedir(dir);
        dir = NULL;
    }
}

/* First word on the line -> complete commands.
   Any other word -> return NULL so readline completes file names. */
char** shell_completion(const char* text, int start, int end) {
    (void)end;
    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    }
    return NULL;
}
