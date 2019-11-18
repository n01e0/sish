/* overall flow
 *  main()
 *   ↓
 *  loop()
 *   ↓
 *  cmd_tokenize(args)
 *   |
 *  (command)
 *   ↓
 *  execute(command)
 *    |-command is "exit"?→ sish_exit()
 *    ↓
 *  launch(command)
 *    ↓
 *  loop()
 */


#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* fn sish_exit
 * Description:
 *      This function for exit this shell
 *
 * Why this function is called:
 *      hook "exit" command and exit this shell
 */
int sish_exit() {
    return 0;
}

/* fn launch
 * Description:
 *      This function make child process and exec another program
 *
 * Variables:
 *      args is tokenized command strings
 *      pid and status for wait child process
 *     
 * Why this function is called:
 *      when called command other than "exit" this function launch other command
 */
int launch(char **args) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("command not found");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("child process generation failure");
    } else {
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}


/* fn execute
 * Description: 
 *      This function is hook command and call function
 *
 * Variables:
 *      args is tokenized command strings
 *
 * Why this function is called:
 *      hook "exec" command
 */

int execute(char **args) {
    if (args[0] ==NULL) {
        return 1;
    }

    if (strncmp(args[0], "exit", strlen(args[0])) == 0) {
        return sish_exit();
    }

    return launch(args);
}

/* fn readln
 * Description:
 *      This function is read and return command line
 * 
 * Variables:
 *      bufsize is buffer size
 *      pos is current position
 *      buf is buffer
 *      c is character in line
 *
 * Why this function is called:
 *      reading command line
 *      and allow file input
 */

#define RL_BUFSIZE 1024

char *readln() {
    int bufsize = RL_BUFSIZE;
    int pos = 0;
    char *buf = malloc(sizeof(char) * RL_BUFSIZE);
    int c;

    if (!buf) {
        perror("alloc");
        exit(1);
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
                perror("alloc");
                exit(1);
            }
        }
    }
}

/* fn readln
 * Description:
 *      this function tokenize and return command line
 * 
 * Variables:
 *      bufsize is buffer size
 *      token is tokenized command line strings
 *      tokens_bk is a backup for when reallocating tokens buffer
 *
 * Why this function is called:
 *      tokenization to allow strings to be passed to execvp
 */

#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"

char **cmd_tokenize(char *line) {
    int bufsize = TOK_BUFSIZE, pos = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token, **tokens_bk;

    if (!tokens) {
        perror("tokenize");
        exit(1);
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
                    perror("alloc");
                    exit(1);
                }
        }

        token = strtok(NULL, TOK_DELIM);
    }
    tokens[pos] = NULL;
    return tokens;
}

/* fn readln
 * Description:
 *      this function is the core
 *      call function recursively
 * 
 * Variables:
 *      line is command line strings
 *      args is tokenized strings for passed to execute
 *      status is execute status
 *
 * Why this function is called:
 *      this function is connecting other functions
 */

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


int main(int argc, char **argv) {
    loop();
    return EXIT_SUCCESS;
}
