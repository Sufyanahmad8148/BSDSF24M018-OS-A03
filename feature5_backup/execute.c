
#include "shell.h"

#define MAX_STAGES 16

/* Execute one command with input/output redirection */
static void run_stage(char** stage)
{
    char* argv[MAXARGS + 1];
    char* infile = NULL;
    char* outfile = NULL;
    int argc = 0;

    /* Remove redirection operators from command arguments */
    for (int i = 0; stage[i] != NULL; i++) {

        if (strcmp(stage[i], "<") == 0 ||
            strcmp(stage[i], ">") == 0) {

            char* op = stage[i];

            if (stage[i + 1] == NULL ||
                strcmp(stage[i + 1], "<") == 0 ||
                strcmp(stage[i + 1], ">") == 0 ||
                strcmp(stage[i + 1], "|") == 0) {

                fprintf(stderr, "shell: missing filename after %s\n", op);
                _exit(1);
            }

            if (strcmp(op, "<") == 0)
                infile = stage[i + 1];
            else
                outfile = stage[i + 1];

            i++;
        }
        else {
            argv[argc++] = stage[i];
        }
    }

    argv[argc] = NULL;

    if (argc == 0) {
        fprintf(stderr, "shell: missing command\n");
        _exit(1);
    }

    /* Input redirection */
    if (infile != NULL) {

        int fd = open(infile, O_RDONLY);

        if (fd < 0) {
            perror(infile);
            _exit(1);
        }

        if (dup2(fd, STDIN_FILENO) < 0) {
            perror("dup2");
            close(fd);
            _exit(1);
        }

        close(fd);
    }

    /* Output redirection */
    if (outfile != NULL) {

        int fd = open(outfile,
                      O_WRONLY | O_CREAT | O_TRUNC,
                      0644);

        if (fd < 0) {
            perror(outfile);
            _exit(1);
        }

        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("dup2");
            close(fd);
            _exit(1);
        }

        close(fd);
    }

    /* Built-in commands */
    if (handle_builtin(argv)) {
        fflush(NULL);
        _exit(0);
    }

    /* External commands */
    execvp(argv[0], argv);

    perror(argv[0]);
    _exit(127);
}


/* Execute pipeline and redirection */
int execute(char** arglist)
{
    char** stages[MAX_STAGES];
    pid_t pids[MAX_STAGES];

    int n = 0;
    int started = 0;

    stages[n++] = arglist;

    /* Divide commands using pipe operator */
    for (int i = 0; arglist[i] != NULL; i++) {

        if (strcmp(arglist[i], "|") == 0) {

            if (n >= MAX_STAGES) {
                fprintf(stderr, "shell: too many pipeline commands\n");
                return 1;
            }

            arglist[i] = NULL;
            stages[n++] = &arglist[i + 1];
        }
    }

    /* Check for empty commands */
    for (int k = 0; k < n; k++) {

        if (stages[k][0] == NULL) {
            fprintf(stderr, "shell: syntax error near '|'\n");
            return 1;
        }
    }

    int prev_read = -1;

    /* Start each pipeline command */
    for (int k = 0; k < n; k++) {

        int fd[2] = {-1, -1};

        if (k < n - 1) {

            if (pipe(fd) < 0) {
                perror("pipe");
                break;
            }
        }

        fflush(NULL);

        pid_t pid = fork();

        if (pid < 0) {

            perror("fork");

            if (fd[0] != -1) {
                close(fd[0]);
                close(fd[1]);
            }

            break;
        }

        if (pid == 0) {

            /* Child: connect previous pipe to input */
            if (prev_read != -1) {

                if (dup2(prev_read, STDIN_FILENO) < 0) {
                    perror("dup2");
                    _exit(1);
                }
            }

            /* Child: connect next pipe to output */
            if (fd[1] != -1) {

                if (dup2(fd[1], STDOUT_FILENO) < 0) {
                    perror("dup2");
                    _exit(1);
                }
            }

            /* Close unused descriptors */
            if (prev_read != -1)
                close(prev_read);

            if (fd[0] != -1)
                close(fd[0]);

            if (fd[1] != -1)
                close(fd[1]);

            /* Execute this pipeline stage */
            run_stage(stages[k]);
        }

        /* Parent process */
        pids[started++] = pid;

        if (prev_read != -1)
            close(prev_read);

        if (fd[1] != -1)
            close(fd[1]);

        prev_read = fd[0];
    }

    /* Close remaining read descriptor */
    if (prev_read != -1)
        close(prev_read);

    /* Wait for all child processes */
    int status = 0;

    for (int k = 0; k < started; k++) {

        int st;

        if (waitpid(pids[k], &st, 0) < 0) {
            perror("waitpid");
            continue;
        }

        if (k == started - 1)
            status = st;
    }

    if (started == 0)
        return 1;

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    if (WIFSIGNALED(status))
        return 128 + WTERMSIG(status);

    return 1;
}
