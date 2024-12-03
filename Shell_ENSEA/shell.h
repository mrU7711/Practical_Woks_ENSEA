#ifndef SHELL_H
#define SHELL_H

#define MAX_INPUT_SIZE 256

// Messages and strings used in the shell
#define WELCOME_MESSAGE "Welcome to ShellENSEA! \nType 'exit' to quit\n"
#define EXIT_SUCCESS_MSG "End of ShellENSEA\nBye bye...\n"
#define PROMPT_TEMPLATE_EXIT "enseash [exit:%d] %% "
#define PROMPT_TEMPLATE_SIGNAL "enseash [sign:%d] %% "

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

// Function prototypes
void shellDisplay(void);
void command(char input[], int bytesRead);
void return_code(void);

// Extern variables
extern int terminal;
extern int fd_input;
extern int status;
extern char waitingPrompt[MAX_INPUT_SIZE];

#endif // SHELL_H
