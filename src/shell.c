#include "shell.h"
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>

/* ---------- Command history ---------- */

static char* history[HISTORY_SIZE];
static int hist_total = 0;

static int first_valid(void) {
    return hist_total > HISTORY_SIZE
        ? hist_total - HISTORY_SIZE + 1 : 1;
}

void add_to_history(const char* cmd) {
    int idx = hist_total % HISTORY_SIZE;

    free(history[idx]);
    history[idx] = strdup(cmd);

    hist_total++;
}

const char* get_history(int n) {
    if (n < first_valid() || n > hist_total)
        return NULL;

    return history[(n - 1) % HISTORY_SIZE];
}

void print_history(void) {
    for (int i = first_valid(); i <= hist_total; i++) {
        printf("%d  %s\n", i,
               history[(i - 1) % HISTORY_SIZE]);
    }
}

/* ---------- Tab completion ---------- */

static const char* builtin_names[] = {
    "exit", "cd", "help", "jobs", "history", NULL
};

static char* command_generator(const char* text, int state) {

    static int bi;
    static char* path_copy;
    static char* path_iter;
    static DIR* dir;
    static char cur_dir[1024];
    static size_t len;

    struct dirent* de;

    if (state == 0) {
        bi = 0;
        len = strlen(text);

        if (dir) {
            closedir(dir);
            dir = NULL;
        }

        free(path_copy);

        const char* p = getenv("PATH");

        path_copy = p ? strdup(p) : NULL;
        path_iter = path_copy;
    }

    while (builtin_names[bi]) {
        const char* name = builtin_names[bi++];

        if (strncmp(name, text, len) == 0)
            return strdup(name);
    }

    while (1) {

        if (dir == NULL) {

            if (path_iter == NULL) {
                free(path_copy);
                path_copy = NULL;
                return NULL;
            }

            char* d = strsep(&path_iter, ":");

            if (d == NULL || *d == '\0')
                continue;

            dir = opendir(d);

            if (dir == NULL)
                continue;

            strncpy(cur_dir, d, sizeof(cur_dir) - 1);
            cur_dir[sizeof(cur_dir) - 1] = '\0';
        }

        while ((de = readdir(dir)) != NULL) {

            if (strncmp(de->d_name, text, len) != 0)
                continue;

            char full[2048];

            snprintf(full, sizeof(full), "%s/%s",
                     cur_dir, de->d_name);

            if (access(full, X_OK) == 0)
                return strdup(de->d_name);
        }

        closedir(dir);
        dir = NULL;
    }
}

char** shell_completion(const char* text, int start, int end) {

    (void)end;

    if (start == 0)
        return rl_completion_matches(text, command_generator);

    return NULL;
}
