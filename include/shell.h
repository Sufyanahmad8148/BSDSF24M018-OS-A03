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

/* Input and history */
char* read_cmd(char* prompt, FILE* fp);
void add_to_history(const char* cmd);
const char* get_history(int n);
void print_history(void);

/* Readline completion */
char** shell_completion(const char* text, int start, int end);

/* Parsing */
char** tokenize(char* cmdline);
void free_tokens(char** arglist);
int has_operator(char** arglist);

/* Execution */
int execute(char** arglist);

/* Built-in commands */
int handle_builtin(char** arglist);

#endif
