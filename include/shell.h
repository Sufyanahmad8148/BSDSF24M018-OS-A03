#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_LEN 512
#define MAXARGS 64
#define ARGLEN 30
#define PROMPT "FCIT> "
#define HISTORY_SIZE 50
#define OPERATOR_CHARS "<>|;&"   /* characters that are tokens on their own */

extern int last_status;          /* exit status of the last command (builtins.c) */

/* shell.c */
char* read_cmd(char* prompt, FILE* fp);
void add_to_history(const char* cmd);
const char* get_history(int n);
void print_history(void);
char** shell_completion(const char* text, int start, int end);

/* parser.c */
char** tokenize(char* cmdline);
void free_tokens(char** arglist);
int has_operator(char** arglist);

/* execute.c */
int execute(char** arglist, int background);

/* run.c */
int run_tokens(char** arglist);
int process_line(char* line);

/* jobs.c */
int add_job(pid_t pid, const char* cmd);
void reap_jobs(void);
void print_jobs(void);

/* control.c (Feature 7) */
int is_if_line(const char* line);
int handle_if(const char* first_line);

/* vars.c (Feature 8) */
int is_assignment(const char* token);
void set_var(const char* name, const char* value);
const char* get_var(const char* name);
void print_vars(void);
char* expand_arg(const char* s);
void expand_args(char** arglist);

/* builtins.c */
int handle_builtin(char** arglist);

#endif // SHELL_H
