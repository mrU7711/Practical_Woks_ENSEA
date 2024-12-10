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
#define MAX_ARGS 100                // Maximum number of arguments for a command
#define MAX_PIPE_SEGMENTS 10        // Maximum number of segments in a pipeline
#define EXIT_CMD "exit"             // Command to exit the shell
#define REDIRECT_INPUT "<"          // Input redirection symbol
#define REDIRECT_OUTPUT ">"         // Output redirection symbol

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
    command[strcspn(command, "\n")] = '\0'; // Remove newline character from input

    struct timespec start, end;
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
        HANDLE_ERROR(ERROR_CLOCK_START);
    }

    // Split the command into segments by "|"
    char *pipe_segments[MAX_PIPE_SEGMENTS];
    int num_segments = 0;
    char *segment = strtok(command, "|");
    while (segment != NULL && num_segments < MAX_PIPE_SEGMENTS) {
        pipe_segments[num_segments++] = segment;
        segment = strtok(NULL, "|");
    }

    int pipe_fds[2];
    int prev_fd = -1;

    for (int i = 0; i < num_segments; i++) {
        if (i < num_segments - 1) {
            if (pipe(pipe_fds) == -1) {
                HANDLE_ERROR(PIPE_ERROR);
            }
        }

        pid_t pid = fork();
        if (pid == -1) {
            HANDLE_ERROR(ERROR_FORK);
        } else if (pid == 0) {
            if (prev_fd != -1) { // Connect to the previous pipe's read end
                if (dup2(prev_fd, STDIN_FILENO) == -1) {
                    HANDLE_ERROR(INPUT_REDIRECT_ERROR);
                }
                close(prev_fd);
            }
            if (i < num_segments - 1) { // Connect to the current pipe's write end
                if (dup2(pipe_fds[1], STDOUT_FILENO) == -1) {
                    HANDLE_ERROR(OUTPUT_REDIRECT_ERROR);
                }
                close(pipe_fds[1]);
                close(pipe_fds[0]);
            }

            // Parse arguments for the current command segment
            char *args[MAX_ARGS];
            char *input_file = NULL;
            char *output_file = NULL;
            int j = 0;

            char *token = strtok(pipe_segments[i], " ");
            while (token != NULL && j < MAX_ARGS) {
                if (strcmp(token, REDIRECT_INPUT) == 0) {
                    input_file = strtok(NULL, " ");
                } else if (strcmp(token, REDIRECT_OUTPUT) == 0) {
                    output_file = strtok(NULL, " ");
                } else {
                    args[j++] = token;
                }
                token = strtok(NULL, " ");
            }
            args[j] = NULL;

            // Handle input redirection
            if (input_file != NULL) {
                FILE *infile = freopen(input_file, "r", stdin);
                if (infile == NULL) {
                    HANDLE_ERROR(ERROR_REDIRECT_INPUT);
                }
            }

            // Handle output redirection
            if (output_file != NULL) {
                FILE *outfile = freopen(output_file, "w", stdout);
                if (outfile == NULL) {
                    HANDLE_ERROR(ERROR_REDIRECT_OUTPUT);
                }
            }

            execvp(args[0], args);
            HANDLE_ERROR(ERROR_EXECVP);
        } else {
            if (prev_fd != -1) {
                close(prev_fd);
            }
            if (i < num_segments - 1) {
                close(pipe_fds[1]);
                prev_fd = pipe_fds[0];
            }
        }
    }

    int status;
    for (int i = 0; i < num_segments; i++) {
        if (wait(&status) == -1) {
            HANDLE_ERROR(ERROR_WAITPID);
        }
    }

    if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) {
        HANDLE_ERROR(ERROR_CLOCK_END);
    }

    *elapsed_time_ms = (end.tv_sec - start.tv_sec) * 1000 +
                       (end.tv_nsec - start.tv_nsec) / 1000000;
    return status;
}
