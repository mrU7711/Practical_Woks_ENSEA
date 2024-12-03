#include "shell.h"

int terminal = STDOUT_FILENO; // File descriptor for output to terminal
int fd_input = STDIN_FILENO;  // File descriptor for input from user
int status;
char waitingPrompt[MAX_INPUT_SIZE] = "";

/**
 * Displays a welcome message when the shell starts.
 */
void shellDisplay(void) {
    write(terminal, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE));
}

/**
 * Executes a command entered by the user.
 * If the user types "exit", the shell terminates.
 * @param input : the command entered by the user.
 * @param bytesRead : the number of bytes read from input.
 */
void command(char input[], int bytesRead) {
    if (strcmp(input, "exit") == 0 || bytesRead == 0) {  // If the user types 'exit' or nothing is entered
        write(terminal, EXIT_SUCCESS_MSG, strlen(EXIT_SUCCESS_MSG));
        close(fd_input);
        close(terminal);
        exit(EXIT_SUCCESS);  // Exit the shell
    }

    pid_t pid = fork();  // Create a new process
    if (pid <= -1) {  // If fork() fails
        close(fd_input);
        close(terminal);
        exit(EXIT_FAILURE);
    } else if (pid == 0) {  // Code executed by the child process
        execlp(input, input, NULL);  // Execute the command
        close(fd_input);
        close(terminal);
        exit(EXIT_SUCCESS);  // If execlp fails, terminate the child process
    } else {  // Code executed by the parent process
        wait(&status);  // Wait for the child process to finish
    }
}

/**
 * Updates the shell prompt based on the exit code or signal of the last process.
 */
void return_code(void) {
    if (WIFEXITED(status)) {  // If the process exited normally
        sprintf(waitingPrompt, PROMPT_TEMPLATE_EXIT, WEXITSTATUS(status));  // Format exit status
    } else if (WIFSIGNALED(status)) {  // If the process was terminated by a signal
        sprintf(waitingPrompt, PROMPT_TEMPLATE_SIGNAL, WTERMSIG(status));  // Format the signal number
    }
}
