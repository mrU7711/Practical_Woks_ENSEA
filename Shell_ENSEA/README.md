# EnseaSH

**EnseaSH** is a lightweight shell created as part of a second-year computer science project. Its goal was to replicate basic shell functionalities, allowing the execution of commands, handling redirections, and supporting pipelines. This document explains what we implemented and the approach we followed for each feature.

---

## Project Description

The aim of this project was to develop a micro-shell step by step, adding one feature at a time. We focused on making it functional and easy to use while adhering to good programming practices, such as:

- Splitting the code into clear and reusable functions.
- Using constants and macros for better readability.
- Writing clean and structured code with meaningful comments.

Our shell reads user commands, executes them, and displays helpful feedback like execution time and return codes.

---

## Features and Approach

### 1. Welcome Message and Prompt

The first task was to display a welcome message when the shell starts. We also created a simple prompt (`enseash %`) that indicates the shell is ready for commands. This was implemented using a `display` function that writes messages directly to the terminal.

### 2. Command Execution

We made the shell execute commands by:

- Reading user input with `fgets`.
- Splitting the input into a command and its arguments (if any).
- Using `execvp` to execute the command.

The shell returns to the prompt after executing a command.

### 3. Exiting the Shell

The shell can be exited either by typing `exit` or pressing `<Ctrl+D>`. When the user exits, the shell displays a goodbye message. This was a simple but essential step to ensure proper cleanup and user experience.

### 4. Return Codes in Prompt

To make the shell more informative, we added return codes to the prompt. After a command is executed, the shell checks its return status:

- If the command exited normally, the code is shown as `exit:x`.
- If the command was interrupted by a signal, it’s shown as `sign:x`.

We used `waitpid` to get the process status and macros like `WIFEXITED` to interpret it.

### 5. Execution Time

For every command, we measure the time it takes to run using `clock_gettime`. This information is displayed in the prompt along with the return code, helping users understand how long their commands take to execute.

### 6. Commands with Arguments

The shell supports commands with arguments, such as `ls -l`. User input is parsed into separate arguments using `strtok`, and the resulting array of arguments is passed to `execvp`.

### 7. Input and Output Redirection

We added support for redirections:

- `>` redirects command output to a file.
- `<` uses a file as input for a command.

The implementation detects these symbols in the input and uses `freopen` to redirect `stdin` or `stdout`. This feature lets users run commands like:

```bash
ls > output.txt
wc -l < output.txt
```

### 8. Pipe Support

The most advanced feature we implemented is support for pipelines (`|`). This allows users to chain commands, passing the output of one command as input to the next.

This was done by:

1. Splitting the command into segments based on the `|` symbol.
2. Creating a child process for each command in the pipeline.
3. Connecting the processes using pipes (`pipe()` and `dup2`).
---

## File Overview

- **`main.c`**: Contains the main loop of the shell. It handles user input, calls the execution logic, and updates the prompt based on the last command's status.
- **`shell.h`**: A header file that defines constants, macros, and function prototypes. It ensures modularity and consistency across the project.
- **`shell.c`**: Implements all the core functionalities, including command execution, handling redirections, managing pipes, and measuring execution time.

---

## How to Use

1. **Compile the shell:**

   ```bash
   make
   ```

2. **Run the shell:**

   ```bash
   ./enseash
   ```

3. **Exit the shell:**

   - Type `exit` or press `<Ctrl+D>`.

---

## What We Didn't Do

This version of the shell stops at **Task 8 (Pipe Redirection)**. The final task, which involves running processes in the background (`&`), was not implemented.

---

## Authors

This shell was developed collaborratively by **Khalid Zouhair** and **Fabien Marcellin** as part of our "TP de synthèse".
