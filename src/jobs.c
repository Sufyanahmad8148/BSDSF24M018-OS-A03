#include "shell.h"

static Job jobs[MAX_JOBS];
static int job_count = 0;

/* Add a background process */
void add_job(pid_t pid, const char* command)
{
    if (job_count >= MAX_JOBS) {
        fprintf(stderr, "shell: maximum background jobs reached\n");
        return;
    }

    jobs[job_count].pid = pid;

    snprintf(jobs[job_count].command,
             MAX_LEN, "%s", command);

    job_count++;

    printf("[%d] %d started: %s\n",
           job_count, (int)pid, command);
}

/* Remove a completed process */
void remove_job(pid_t pid)
{
    for (int i = 0; i < job_count; i++) {

        if (jobs[i].pid == pid) {

            for (int j = i; j < job_count - 1; j++)
                jobs[j] = jobs[j + 1];

            job_count--;
            return;
        }
    }
}

/* Display background jobs */
void print_jobs(void)
{
    if (job_count == 0) {
        printf("No background jobs.\n");
        return;
    }

    printf("PID\tCOMMAND\n");

    for (int i = 0; i < job_count; i++) {
        printf("%d\t%s\n",
               (int)jobs[i].pid,
               jobs[i].command);
    }
}

/* Collect completed child processes without blocking */
void reap_jobs(void)
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {

        printf("\n[Background process %d completed]\n",
               (int)pid);

        remove_job(pid);
    }
}
