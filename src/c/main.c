#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static char **env;

typedef enum {
    CH_DIR,
    EXEC,
    TOKENIZE,
    ALLOC,
    LOOP
}err_t;

void eprint(err_t err) {
    switch (err) {
        case LOOP:
            fprintf(stderr, "\e[32;41;1merror:\e[m in loop error\n");
            exit(EXIT_FAILURE);
        case EXEC:
            fprintf(stderr, "\e[32;41;1merror:\e[m execute error\n");
            exit(EXIT_FAILURE);
        case TOKENIZE:
            fprintf(stderr, "\e[32;41;1merror:\e[m tokenize error\n");
            exit(EXIT_FAILURE);
        case ALLOC:
            fprintf(stderr, "\e[32;41;1merror:\e[m allocation error\n");
            exit(EXIT_FAILURE);
        case CH_DIR:
            fprintf(stderr, "\e[32;41;1merror:\e[m change directory error\n");
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "\e[32;41;1merror:\e[m something error\n");
            exit(EXIT_FAILURE);
    }
}

int sish_exit(char **args) {
    return 0;
}

int sish_cd(char **args) {
    if (chdir((args[1] != NULL? args[1]: getenv("HOME"))) != 0)
        eprint(CH_DIR);
    return 1;
}

char *cmd_str[] = {
    "exit",
    "cd"
};

int (*cmd_fn[]) (char**) =  {
    &sish_exit,
    &sish_cd
};

int cmd_num() {
    return sizeof(cmd_str) / sizeof(char *);
}

int launch(char **args) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1)
            eprint(EXEC);
    } else if (pid < 0) {
        perror("fork error");
    } else {
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int execute(char **args) {
    if (args[0] ==NULL)
        return 1;

    for (int i = 0; i < cmd_num(); i++) {
        if (strncmp(args[0], cmd_str[i], strlen(args[0])) == 0)
            return (*cmd_fn[i])(args);
    }
    return launch(args);
}

#define RL_BUFSIZE 1024

char *readln() {
    int bufsize = RL_BUFSIZE;
    int pos = 0;
    char *buf = malloc(sizeof(char) * RL_BUFSIZE);
    int c;

    if (!buf) {
        eprint(ALLOC);
    }

    while (1) {
        c = getchar();

        if (c == EOF) {
            exit(EXIT_SUCCESS);
        } else if (c == '\n') {
            buf[pos] = '\0';
            return buf;
        } else {
            buf[pos] = c;
        }
        pos++;

        if (pos >= RL_BUFSIZE) {
            bufsize += RL_BUFSIZE;
            buf = realloc(buf, bufsize);
            if (!buf) {
                eprint(ALLOC);
            }
        }
    }
}

#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"

char **cmd_tokenize(char *line) {
    int bufsize = TOK_BUFSIZE, pos = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token, **tokens_bk;

    if (!tokens) {
        eprint(TOKENIZE);
    }

    token = strtok(line, TOK_DELIM);
    while (token != NULL) {
        tokens[pos] = token;
        pos++;

        if (pos >= bufsize) {
                bufsize += TOK_BUFSIZE;
                tokens_bk = tokens;
                tokens = realloc(tokens, bufsize * sizeof(char ));
                if (!tokens) {
                    free(tokens_bk);
                    eprint(ALLOC);
                }
        }

        token = strtok(NULL, TOK_DELIM);
    }
    tokens[pos] = NULL;
    return tokens;
}

void loop() {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = readln();
        args = cmd_tokenize(line);
        status = execute(args);

        free(line);
        free(args);
    } while(status);
}

int main(int argc, const char **argv, char **envp) {
    env = envp;
    loop();
    return EXIT_SUCCESS;
}
