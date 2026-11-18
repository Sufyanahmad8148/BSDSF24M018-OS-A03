#include "shell.h"

#define MAX_STAGES 16      /* max commands connected by pipes */

/* Runs inside a child process: handle < and >, then become the program. */
static void run_stage(char** stage) {
    char* argv[MAXARGS + 1];
    char* infile = NULL;
    char* outfile = NULL;
    int argc = 0;

    /* separate the real arguments from the redirection tokens */
    for (int i = 0; stage[i] != NULL; i++) {
        if (strcmp(stage[i], "<") == 0 || strcmp(stage[i], ">") == 0) {
            if (stage[i + 1] == NULL) {
                fprintf(stderr, "shell: syntax error: missing file name after '%s'\n", stage[i]);
                _exit(1);
            }
            if (stage[i][0] == '<') infile = stage[i + 1];
            else                    outfile = stage[i + 1];
            i++;                                   /* skip the file name */
        } else {
            argv[argc++] = stage[i];
        }
    }
    argv[argc] = NULL;

    if (argc == 0) {
        fprintf(stderr, "shell: syntax error: missing command\n");
        _exit(1);
    }

    if (infile != NULL) {                          /* cmd < file */
        int fd = open(infile, O_RDONLY);
        if (fd < 0) { perror(infile); _exit(1); }
        dup2(fd, STDIN_FILENO);                    /* stdin now reads from the file */
        close(fd);                                 /* original fd no longer needed */
    }
    if (outfile != NULL) {                         /* cmd > file */
        int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) { perror(outfile); _exit(1); }
        dup2(fd, STDOUT_FILENO);                   /* stdout now writes to the file */
        close(fd);
    }

    /* built-ins used with a pipe/redirect (e.g. "history > h.txt") run here */
    if (handle_builtin(argv)) {
        fflush(stdout);
        _exit(0);
    }

    execvp(argv[0], argv);
    perror(argv[0]);                               /* only reached if exec failed */
    _exit(127);
}

/* Runs a command line that may contain pipes and redirections.
   Returns the exit status of the last command. */
int execute(char** arglist) {
    char** stages[MAX_STAGES];     /* stages[k] = tokens of the k-th command */
    pid_t pids[MAX_STAGES];
    int n = 0;

    /* cut the token list at every "|" so each stage is its own NULL-terminated list */
    stages[n++] = arglist;
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "|") == 0) {
            if (n == MAX_STAGES) {
                fprintf(stderr, "shell: too many commands in pipeline\n");
                return 1;
            }
            free(arglist[i]);
            arglist[i] = NULL;                     /* terminates the previous stage */
            stages[n++] = &arglist[i + 1];
        }
    }
    for (int k = 0; k < n; k++) {
        if (stages[k][0] == NULL) {
            fprintf(stderr, "shell: syntax error near '|'\n");
            return 1;
        }
    }

    int prev_read = -1;            /* read end of the previous pipe */
    int started = 0;
    for (int k = 0; k < n; k++) {
        int fd[2] = {-1, -1};
        if (k < n - 1 && pipe(fd) < 0) {           /* pipe only between two commands */
            perror("pipe");
            break;
        }
        fflush(NULL);                              /* no duplicated buffered output in the child */
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            if (fd[0] != -1) { close(fd[0]); close(fd[1]); }
            break;
        }
        if (pid == 0) {                            /* ----- child ----- */
            if (prev_read != -1) {                 /* read from the previous command */
                dup2(prev_read, STDIN_FILENO);
                close(prev_read);
            }
            if (fd[1] != -1) {                     /* write into the next command */
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);                      /* close BOTH unused pipe ends */
                close(fd[1]);
            }
            run_stage(stages[k]);                  /* never returns */
        }
        /* ----- parent ----- */
        pids[started++] = pid;
        if (prev_read != -1) close(prev_read);     /* parent must close its copies too, */
        if (fd[1] != -1) close(fd[1]);             /* otherwise readers never see EOF */
        prev_read = fd[0];
    }
    if (prev_read != -1) close(prev_read);

    int status = 0;
    for (int k = 0; k < started; k++) {            /* wait for every child */
        int st;
        waitpid(pids[k], &st, 0);
        if (k == started - 1) status = st;
    }
    if (started == 0) return 1;
    return WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
}
