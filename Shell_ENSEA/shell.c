#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <linux/time.h>

// Macros for command execution
#define MAX_ARGS 100
#define EXIT_CMD "exit"
#define REDIRECT_INPUT "<"
#define REDIRECT_OUTPUT ">"

// Displays a message to the standard output
void display(const char *message) {
    if (write(STDOUT_FILENO, message, strlen(message)) == -1) {
        HANDLE_ERROR(ERROR_WRITE);
    }
}

// Reads user input and stores it in a buffer
int read_user_input(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin) == NULL) {
        if (feof(stdin)) {
            return 0; // End of input (Ctrl+D)
        } else {
            HANDLE_ERROR(ERROR_READ);
        }
    }
    return 1;
}

// Checks if the user input matches the exit command
int is_exit_command(const char *buffer) {
    return strncmp(buffer, EXIT_CMD, strlen(EXIT_CMD)) == 0 &&
           (buffer[strlen(EXIT_CMD)] == '\n' || buffer[strlen(EXIT_CMD)] == '\0');
}

// Formats the status of the executed command
void get_command_status(int status, long elapsed_time_ms, char *status_string, size_t size) {
    if (WIFEXITED(status)) {
        snprintf(status_string, size, "[exit:%d|%ldms]", WEXITSTATUS(status), elapsed_time_ms);
    } else if (WIFSIGNALED(status)) {
        snprintf(status_string, size, "[sign:%d|%ldms]", WTERMSIG(status), elapsed_time_ms);
    } else {
        status_string[0] = '\0';
    }
}

// Executes a command, measures its execution time, and returns its status
int execute_command(char *command, long *elapsed_time_ms) {
    command[strcspn(command, "\n")] = '\0';

    struct timespec start, end;
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
        HANDLE_ERROR(ERROR_CLOCK_START);
    }

    char *args[MAX_ARGS];
    char *input_file = NULL, *output_file = NULL;
    char *token = strtok(command, " ");
    int i = 0;

    while (token != NULL) {
        if (strcmp(token, REDIRECT_INPUT) == 0) {
            input_file = strtok(NULL, " ");
        } else if (strcmp(token, REDIRECT_OUTPUT) == 0) {
            output_file = strtok(NULL, " ");
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    pid_t pid = fork();
    if (pid == -1) {
        HANDLE_ERROR(ERROR_FORK);
    } else if (pid == 0) {
        if (input_file != NULL && freopen(input_file, "r", stdin) == NULL) {
            HANDLE_ERROR(ERROR_REDIRECT_INPUT);
        }
        if (output_file != NULL && freopen(output_file, "w", stdout) == NULL) {
            HANDLE_ERROR(ERROR_REDIRECT_OUTPUT);
        }
        execvp(args[0], args);
        HANDLE_ERROR(ERROR_EXECVP);
    } else {
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            HANDLE_ERROR(ERROR_WAITPID);
        }

        if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) {
            HANDLE_ERROR(ERROR_CLOCK_END);
        }

        *elapsed_time_ms = (end.tv_sec - start.tv_sec) * 1000 +
                           (end.tv_nsec - start.tv_nsec) / 1000000;
        return status;
    }
}
