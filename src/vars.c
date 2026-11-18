#include "shell.h"
#include <ctype.h>

/* Shell variables are stored in a singly linked list of name/value pairs. */
typedef struct var {
    char* name;
    char* value;
    struct var* next;
} var_t;

static var_t* vars = NULL;

/* 1 if token looks like NAME=value (NAME = letters, digits, '_', not starting with a digit) */
int is_assignment(const char* token) {
    if (!(isalpha((unsigned char)token[0]) || token[0] == '_')) return 0;
    int i = 1;
    while (isalnum((unsigned char)token[i]) || token[i] == '_') i++;
    return token[i] == '=';
}

/* create the variable, or overwrite its value if it already exists */
void set_var(const char* name, const char* value) {
    var_t* last = NULL;
    for (var_t* v = vars; v != NULL; v = v->next) {
        if (strcmp(v->name, name) == 0) {
            free(v->value);
            v->value = strdup(value);
            return;
        }
        last = v;
    }
    var_t* node = malloc(sizeof(var_t));
    node->name = strdup(name);
    node->value = strdup(value);
    node->next = NULL;
    if (last == NULL) vars = node;                 /* first variable */
    else              last->next = node;           /* append at the end */
}

const char* get_var(const char* name) {
    for (var_t* v = vars; v != NULL; v = v->next) {
        if (strcmp(v->name, name) == 0) return v->value;
    }
    return NULL;
}

void print_vars(void) {
    for (var_t* v = vars; v != NULL; v = v->next) {
        printf("%s=%s\n", v->name, v->value);
    }
}

/* append n bytes of src to *out, growing the buffer when needed */
static void append(char** out, size_t* len, size_t* cap, const char* src, size_t n) {
    while (*len + n + 1 > *cap) {
        *cap *= 2;
        *out = realloc(*out, *cap);
    }
    memcpy(*out + *len, src, n);
    *len += n;
}

/* Returns a NEW malloc'd string where every $NAME is replaced by its value
   (shell variable first, then environment variable, otherwise empty). */
char* expand_arg(const char* s) {
    size_t cap = strlen(s) + 16, len = 0;
    char* out = malloc(cap);

    for (const char* p = s; *p != '\0'; ) {
        if (*p == '$' && (isalpha((unsigned char)p[1]) || p[1] == '_')) {
            const char* q = p + 1;                 /* find the end of the name */
            while (isalnum((unsigned char)*q) || *q == '_') q++;

            char name[256];
            size_t nl = (size_t)(q - (p + 1));
            if (nl >= sizeof(name)) nl = sizeof(name) - 1;
            memcpy(name, p + 1, nl);
            name[nl] = '\0';

            const char* val = get_var(name);
            if (val == NULL) val = getenv(name);
            if (val == NULL) val = "";
            append(&out, &len, &cap, val, strlen(val));
            p = q;
        } else {
            append(&out, &len, &cap, p, 1);
            p++;
        }
    }
    out[len] = '\0';
    return out;
}

/* expand every token in place: old string freed, new string stored */
void expand_args(char** arglist) {
    for (int i = 0; arglist[i] != NULL; i++) {
        char* expanded = expand_arg(arglist[i]);
        free(arglist[i]);
        arglist[i] = expanded;
    }
}
