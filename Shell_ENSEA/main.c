#include "shell.h"

/**
 * Main entry point of the shell program.
 * Initializes the shell and runs the main loop to handle user commands.
 */
int main(int argc, char **argv) {
    shellDisplay();  // Display the welcome message

    char input[MAX_INPUT_SIZE];
    int bytesRead;

    while (1) {
        return_code();  // Update the shell prompt
        write(terminal, waitingPrompt, strlen(waitingPrompt));  // Display the prompt

        bytesRead = read(fd_input, input, sizeof(input));  // Read user input
        input[bytesRead - 1] = '\0';  // Remove the newline character from input

        command(input, bytesRead);  // Execute the entered command
    }

    return EXIT_SUCCESS;
}
