#include "systemcalls.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{
    if (cmd == NULL) {
        return false; // Null command is an error
    }

    int ret = system(cmd); // Execute the command using system()
    if (ret == -1 || WEXITSTATUS(ret) != 0) {
        return false; // Error occurred or command returned non-zero exit code
    }

    return true; // Command executed successfully
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    if (count < 1) {
        return false; // At least one argument (command path) is required
    }

    va_list args;
    va_start(args, count);

    char *command[count + 1];
    for (int i = 0; i < count; i++) {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL; // Null-terminate the array for execv

    va_end(args);
    
    fflush(stdout); //repeating removing

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return false; // Fork failed
    }

    if (pid == 0) {
        // Child process
        execv(command[0], command);
        perror("execv failed"); // If execv returns, it must have failed
        exit(EXIT_FAILURE);
    }

    // Parent process
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid failed");
        return false;
    }

    // Check if the child process exited successfully
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return true;
    }

    return false; // Command failed
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    if (outputfile == NULL || count < 1) {
        return false; // Output file and at least one argument (command path) are required
    }

    va_list args;
    va_start(args, count);

    char *command[count + 1];
    for (int i = 0; i < count; i++) {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL; // Null-terminate the array for execv

    va_end(args);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return false; // Fork failed
    }

    if (pid == 0) {
        // Child process
        int fd = open(outputfile, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
        if (fd == -1) {
            perror("open failed");
            exit(EXIT_FAILURE);
        }

        // Redirect stdout to the file
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("dup2 failed");
            close(fd);
            exit(EXIT_FAILURE);
        }
        close(fd); // Close the file descriptor after duplication

        execv(command[0], command);
        perror("execv failed"); // If execv returns, it must have failed
        exit(EXIT_FAILURE);
    }

    // Parent process
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid failed");
        return false;
    }

    // Check if the child process exited successfully
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return true;
    }

    return false; // Command failed
}
