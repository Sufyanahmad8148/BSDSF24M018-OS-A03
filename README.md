# BSDSF24M018 - OS Assignment 03: A Unix Shell in C

A command-line shell (like a small bash) built step by step in C, with one
Git branch, one tag and one GitHub release per feature.

## Build and run

    sudo apt install build-essential libreadline-dev
    make
    ./bin/myshell

## Features

| Tag | Feature |
|-----|---------|
| v1.0-base | Base shell: fork, exec, wait |
| v2 | Built-ins: `exit`, `cd`, `help`, `jobs` |
| v3 | Command history: `history`, `!n` |
| v4 | GNU Readline: arrow keys and Tab completion |
| v5 | I/O redirection `<` `>` and pipes `\|` |
| v6 | Command chaining `;` and background jobs `&` |
| v7 | `if / then / else / fi` |
| v8 | Shell variables: `NAME=value`, `$NAME`, `set` |

## Examples

    FCIT> ls -l > files.txt
    FCIT> sort < files.txt
    FCIT> cat /etc/passwd | grep root | wc -l
    FCIT> echo "one" ; sleep 2 ; echo "two"
    FCIT> sleep 10 &
    FCIT> jobs
    FCIT> NAME="World"
    FCIT> echo "Hello $NAME"
    FCIT> if grep root /etc/passwd > /dev/null
    > then
    > echo found
    > else
    > echo missing
    > fi

## Source layout

| File | Purpose |
|------|---------|
| `src/main.c` | main loop: reap jobs, read line, history, dispatch |
| `src/parser.c` | tokenizer (quotes, operators) |
| `src/run.c` | splits a line at `;` and `&`, runs each part |
| `src/execute.c` | fork, pipe, dup2, exec, wait |
| `src/builtins.c` | built-in commands and variable assignment |
| `src/jobs.c` | background job list and zombie reaping |
| `src/control.c` | if/then/else/fi |
| `src/vars.c` | variable storage and `$NAME` expansion |
| `src/shell.c` | history and Tab completion |

## Known limitations

No `>>`, `&&`, `||` or nested `if`. Variables are not exported to child programs.

## Git workflow

Every feature was developed on its own branch (`feature-*`), merged into
`main`, tagged, and published as a GitHub release with the compiled binary.
The last change (a `help` fix) went through an issue and a pull request from a fork.
