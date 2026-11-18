
#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "FCIT> "
#define HISTORY_SIZE 50

#define MAX_JOBS 100
#define MAX_COMMANDS 32

typedef struct {
    pid_t pid;
    char command[MAX_LEN];
} Job;

/* Existing shell functions */
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int handle_builtin(char** arglist);

void add_to_history(const char* cmd);
const char* get_history(int n);
void print_history(void);

char** shell_completion(const char* text, int start, int end);

/* Parser functions */
char** split_commands(char* line);
void free_commands(char** commands);
void free_tokens(char** arglist);
int has_operator(char** arglist);

/* Execution */
int execute(char** arglist, int background, const char* command);

/* Background job management */
void add_job(pid_t pid, const char* command);
void remove_job(pid_t pid);
void print_jobs(void);
void reap_jobs(void);

#endif
