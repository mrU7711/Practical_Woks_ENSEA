#include "shell.h"
#include <stdio.h>

int main(void) {
    char buffer[BUFFER_SIZE];
    char status_string[STATUS_SIZE] = "";

    display(WELCOME_MESSAGE);
    display(EXIT_COMMAND_MESSAGE);

    while (1) {
        char prompt[BUFFER_SIZE];
        snprintf(prompt, BUFFER_SIZE, PROMPT_FORMAT, status_string);
        display(prompt);

        if (!read_user_input(buffer, BUFFER_SIZE) || is_exit_command(buffer)) {
            break;
        }

        long elapsed_time_ms = 0;
        int raw_status = execute_command(buffer, &elapsed_time_ms);
        get_command_status(raw_status, elapsed_time_ms, status_string, STATUS_SIZE);
    }

    display(BYE_MESSAGE);
    return 0;
}
