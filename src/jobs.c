#include "shell.h"

#define MAX_JOBS 64

typedef struct {
    int used;
    int id;
    pid_t pid;
    char cmd[MAX_LEN];
} job_t;

static job_t jobs[MAX_JOBS];
static int next_job_id = 1;

/* remember a background process; returns its job number (0 if table is full) */
int add_job(pid_t pid, const char* cmd) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].used) {
            jobs[i].used = 1;
            jobs[i].id = next_job_id++;
            jobs[i].pid = pid;
            snprintf(jobs[i].cmd, sizeof(jobs[i].cmd), "%s", cmd);
            return jobs[i].id;
        }
    }
    return 0;
}

/* Collect every finished child WITHOUT blocking (WNOHANG).
   This is what prevents zombie processes. */
void reap_jobs(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < MAX_JOBS; i++) {
            if (jobs[i].used && jobs[i].pid == pid) {
                printf("[%d] Done      %s\n", jobs[i].id, jobs[i].cmd);
                jobs[i].used = 0;                  /* remove from the list */
            }
        }
    }
}

void print_jobs(void) {
    reap_jobs();                                   /* drop finished jobs first */
    int shown = 0;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].used) {
            printf("[%d] %d Running   %s\n", jobs[i].id, (int)jobs[i].pid, jobs[i].cmd);
            shown++;
        }
    }
    if (shown == 0) printf("No background jobs.\n");
}
