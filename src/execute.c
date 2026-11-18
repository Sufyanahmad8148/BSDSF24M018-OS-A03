#include "shell.h"

#define MAX_STAGES 16      /* max commands connected by pipes */

/* Runs inside a child process: handle < and >, then become the program.
   no_tty_stdin = 1 for the first command of a background job. */
static void run_stage(char** stage, int no_tty_stdin) {
    char* argv[MAXARGS + 1];
    char* infile = NULL;
    char* outfile = NULL;
    int argc = 0;

    for (int i = 0; stage[i] != NULL; i++) {
        if (strcmp(stage[i], "<") == 0 || strcmp(stage[i], ">") == 0) {
            if (stage[i + 1] == NULL) {
                fprintf(stderr, "shell: syntax error: missing file name after '%s'\n", stage[i]);
                _exit(1);
            }
            if (stage[i][0] == '<') infile = stage[i + 1];
            else                    outfile = stage[i + 1];
            i++;
        } else {
            argv[argc++] = stage[i];
        }
    }
    argv[argc] = NULL;

    if (argc == 0) {
        fprintf(stderr, "shell: syntax error: missing command\n");
        _exit(1);
    }

    if (infile != NULL) {
        int fd = open(infile, O_RDONLY);
        if (fd < 0) { perror(infile); _exit(1); }
        dup2(fd, STDIN_FILENO);
        close(fd);
    } else if (no_tty_stdin) {                     /* background job must not read the keyboard */
        int fd = open("/dev/null", O_RDONLY);
        if (fd >= 0) { dup2(fd, STDIN_FILENO); close(fd); }
    }
    if (outfile != NULL) {
        int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) { perror(outfile); _exit(1); }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    if (handle_builtin(argv)) {
        fflush(stdout);
        _exit(0);
    }

    execvp(argv[0], argv);
    perror(argv[0]);
    _exit(127);
}

/* Runs one command line segment (may contain pipes and redirections).
   background = 1: do NOT wait; remember the job instead.
   Returns the exit status (0 for a background job that was just started). */
int execute(char** arglist, int background) {
    char** stages[MAX_STAGES];
    pid_t pids[MAX_STAGES];
    char cmdtext[MAX_LEN] = "";
    int n = 0;

    if (background) {                              /* text shown by 'jobs' */
        for (int i = 0; arglist[i] != NULL; i++) {
            strncat(cmdtext, arglist[i], sizeof(cmdtext) - strlen(cmdtext) - 1);
            strncat(cmdtext, " ", sizeof(cmdtext) - strlen(cmdtext) - 1);
        }
    }

    stages[n++] = arglist;
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "|") == 0) {
            if (n == MAX_STAGES) {
                fprintf(stderr, "shell: too many commands in pipeline\n");
                return 1;
            }
            free(arglist[i]);
            arglist[i] = NULL;
            stages[n++] = &arglist[i + 1];
        }
    }
    for (int k = 0; k < n; k++) {
        if (stages[k][0] == NULL) {
            fprintf(stderr, "shell: syntax error near '|'\n");
            return 1;
        }
    }

    int prev_read = -1;
    int started = 0;
    for (int k = 0; k < n; k++) {
        int fd[2] = {-1, -1};
        if (k < n - 1 && pipe(fd) < 0) {
            perror("pipe");
            break;
        }
        fflush(NULL);
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            if (fd[0] != -1) { close(fd[0]); close(fd[1]); }
            break;
        }
        if (pid == 0) {                            /* ----- child ----- */
            if (prev_read != -1) {
                dup2(prev_read, STDIN_FILENO);
                close(prev_read);
            }
            if (fd[1] != -1) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
            }
            run_stage(stages[k], background && k == 0);
        }
        pids[started++] = pid;                     /* ----- parent ----- */
        if (prev_read != -1) close(prev_read);
        if (fd[1] != -1) close(fd[1]);
        prev_read = fd[0];
    }
    if (prev_read != -1) close(prev_read);

    if (started == 0) return 1;

    if (background) {                              /* do NOT call waitpid for it */
        int id = add_job(pids[started - 1], cmdtext);
        printf("[%d] %d\n", id, (int)pids[started - 1]);
        return 0;
    }

    int status = 0;
    for (int k = 0; k < started; k++) {
        int st;
        waitpid(pids[k], &st, 0);
        if (k == started - 1) status = st;
    }
    return WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
}
