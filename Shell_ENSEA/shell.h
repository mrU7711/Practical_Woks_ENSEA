#ifndef SHELL_H
#define SHELL_H

#include <stddef.h>

// Macros for buffer and status sizes
#define BUFFER_SIZE 1024
#define STATUS_SIZE 50

// Messages
#define WELCOME_MESSAGE "Welcome to ENSEA Shell!\n"
#define EXIT_COMMAND_MESSAGE "If you want to exit the program type 'exit'\n"
#define BYE_MESSAGE "\nBye Bye\n"
#define PROMPT_FORMAT "ENSEASH %s %% "

// Error messages
#define ERROR_WRITE "Error while writing message"
#define ERROR_READ "Error while reading user input"
#define ERROR_FORK "Error during fork"
#define ERROR_EXECVP "Error during execvp"
#define ERROR_WAITPID "Error during waitpid"
#define ERROR_CLOCK_START "Error during clock_gettime (start)"
#define ERROR_CLOCK_END "Error during clock_gettime (end)"
#define ERROR_REDIRECT_INPUT "Error while opening input file"
#define ERROR_REDIRECT_OUTPUT "Error while opening output file"

// Macros for perror handling
#define HANDLE_ERROR(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

// Function prototypes
void display(const char *message);
int read_user_input(char *buffer, size_t size);
int is_exit_command(const char *buffer);
void get_command_status(int status, long elapsed_time_ms, char *status_string, size_t size);
int execute_command(char *command, long *elapsed_time_ms);

#endif // SHELL_H
